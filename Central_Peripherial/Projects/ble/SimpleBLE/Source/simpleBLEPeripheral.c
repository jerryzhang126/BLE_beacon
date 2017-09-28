/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "hal_mcu.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "simpleGATTprofile.h"

#if defined(CC2540_MINIDK)
#include "simplekeys.h"
#endif

#if defined(PLUS_BROADCASTER)
#include "peripheralBroadcaster.h"
#else
#include "peripheral.h"
#endif

#include "gapbondmgr.h"

#include "simpleBLEPeripheral.h"

#if defined FEATURE_OAD
#include "oad.h"
#include "oad_target.h"
#endif

#include "simpleble.h"
#include "npi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "amomcu_buffer.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL 6 //80   连接间隔与数据发送量有关， 连接间隔越短， 单位时间内就能发送越多的数据

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL 6 //800   连接间隔与数据发送量有关， 连接间隔越短， 单位时间内就能发送越多的数据

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY 0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT 100 //1000  -各种原因断开连接后，超时并重新广播的时间:  100 = 1s

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL 3

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID 0x000D

#define INVALID_CONNHANDLE 0xFFFF

#if defined(PLUS_BROADCASTER)
#define ADV_IN_CONN_WAIT 500 // delay 500 ms
#endif

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
//uint8 simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

gaprole_States_t gapProfileState = GAPROLE_INIT;

bool simpleBLEChar6DoWrite2 = TRUE;
bool timerIsOn = FALSE; //

// Connection handle
uint16 gapConnHandle = NULL;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
    {
        // complete name
        0x0A, // length of this data
        GAP_ADTYPE_LOCAL_NAME_COMPLETE,
        0x44,
        0x61,
        0x72,
        0x72,
        0x65,
        0x6E,
        0x42,
        0x4C,
        0x45,
        // connection interval range
        0x05, // length of this data
        GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
        LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), // 100ms
        HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
        LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL), // 1s
        HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
        // Tx power level
        0x02, // length of this data
        GAP_ADTYPE_POWER_LEVEL,
        0 // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData_iBeacon[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02, // length of this data, 0
  GAP_ADTYPE_FLAGS, // 1
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED, // 2

  // in this peripheral
  0x1A, // length of this data 26byte, 3
  GAP_ADTYPE_MANUFACTURER_SPECIFIC, // 4
  /*Apple Pre-Amble*/
  0x4C, // 5
  0x00, // 6
  0x02, // 7
  0x15, // 8
  /*Device UUID (16 Bytes)*/
  0xE2, 0xC5, 0x6D, 0xB5, 0xDF, 0xFB, 0x48, 0xD2, 0xB0, 0x60, 0xD0, 0xF5, 0xA7, 0x10, 0x96, 0xE0, //9 ~ 24
  /*Major Value (2 Bytes)*/
  0x00, 0x00, // 25-26
  /*Minor Value (2 Bytes)*/
  0x00, 0x00, // 27-28
  /*Measured Power*/
  0xCD //29
};

// LED related.
static uint8 key_led_count = BUTTON_LED_TOGGLE_COUNT; //Blink for 3 times.
static uint8 led_toggle_count = 0;
static uint8 led_toggle_cnt_target = PERIPHERAL_START_LED_TOGGLE_CNT;
static uint8 led_toggling = FALSE;
static uint16 led_toggle_period = PERIPHERAL_START_LED_TOGGLE_PERIOD;
// Default WAKEUP period
static uint16 wake_up_hours_remain = DEFAULT_WAKE_TIME_HOURS;
static uint8 battery_voltage = 0;
// Key related
static uint8 key_pressed_count = 0;
static uint8 key_processing = FALSE;
//first boot up
static bool first_boot = TRUE;
// Low power status
static bool low_power_state = FALSE;
static bool key_pressed = FALSE;

#ifdef DEBUG_BOARD
static bool debug_low_power = FALSE;
#endif

//static int8 gMP = 0xCD;
// GAP GATT Attributes

// GAP GATT Attributes
//static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "Simple BLE Peripheral";

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg(osal_event_hdr_t *pMsg);
static void simpleBLEPeripheral_ProcessGATTMsg(gattMsgEvent_t *pMsg);
static void peripheralStateNotificationCB(gaprole_States_t newState);
static void peripheralRssiReadCB(int8 rssi);
static void simpleProfileChangeCB(uint8 paramID);
static uint8 led_toggle_set_param(uint16 toggle_period, uint32 toggle_target_cnt, uint16 delay);
static uint8 led_toggle_clean_param(void);
static bool check_low_battery(void);
static void enter_low_battery_mode(void);

//#if defined( BLE_BOND_PAIR )
typedef enum {
  BOND_PAIR_STATUS_PAIRING, //未配对
  BOND_PAIR_STATUS_PAIRED,  //已配对
} BOND_PAIR_STATUS;
// 用来管理当前的状态，如果密码不正确，立即取消连接，并重启
static BOND_PAIR_STATUS gPairStatus = BOND_PAIR_STATUS_PAIRING;

void ProcessPasscodeCB(uint8 *deviceAddr, uint16 connectionHandle, uint8 uiInputs, uint8 uiOutputs);
static void ProcessPairStateCB(uint16 connHandle, uint8 state, uint8 status);
//#endif

//#if defined( CC2540_MINIDK )
static void simpleBLEPeripheral_HandleKeys(uint8 shift, uint8 keys);
//#endif

static uint16 advInt;

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
    {
        peripheralStateNotificationCB, // Profile State Change Callbacks
        peripheralRssiReadCB,          // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
    {
        //#if defined( BLE_BOND_PAIR )
        ProcessPasscodeCB, // 密码回调
        ProcessPairStateCB // 绑定状态回调
                           //#else
                           //  NULL,                     // Passcode callback (not used by application)
                           //  NULL                      // Pairing / Bonding state Callback (not used by application)
                           //#endif
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t simpleBLEPeripheral_SimpleProfileCBs =
    {
        simpleProfileChangeCB // Charactersitic value change callback
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEPeripheral_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEPeripheral_Init(uint8 task_id)
{
  simpleBLETaskId = task_id;

  // Setup the GAP
  VOID GAP_SetParamValue(TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL);

  // Setup the GAP Peripheral Role Profile
  {
    // For other hardware platforms, device starts advertising upon initialization
    uint8 initial_advertising_enable = FALSE;
    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    // Cannot be connected.
    uint8 advType = GAP_ADTYPE_ADV_NONCONN_IND; // use non-connectable advertisements
    GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(uint8), &advType);

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);
    GAPRole_SetParameter(GAPROLE_ADVERT_OFF_TIME, sizeof(uint16), &gapRole_AdvertOffTime);

    GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);
    GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData_iBeacon), advertData_iBeacon);
    GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE, sizeof(uint8), &enable_update_request);
    GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16), &desired_min_interval);
    GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16), &desired_max_interval);
    GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY, sizeof(uint16), &desired_slave_latency);
    GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER, sizeof(uint16), &desired_conn_timeout);
  }

  // Set the GAP Characteristics
  //GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );
  GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, simpleBle_GetAttDeviceName());

  {
    // 设置rssi 参数更新速率
    uint16 rssi_read_rate_1ms = 500; //一秒更新2次
    GAPRole_SetParameter(GAPROLE_RSSI_READ_RATE, sizeof(uint16), &rssi_read_rate_1ms);
  }

  // Set advertising interval
  {
    advInt = SLOW_ADVERTISING_INTERVAL;
    /*
    if(simpleBLE_CheckIfUse_iBeacon())
    {
        // advInt = simpleBLE_GetiBeaconAdvertisingInterral();
        // We need to change this interval dynamically.
        // advInt = (advInt*1000 / 625);
    }
    */
    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, advInt);
    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, advInt);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, advInt);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, advInt);
  }

  //下面是与配对相关的设置
  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY; //显示密码， 以便主机输入配对的密码

    /*
    bonding就是把配对信息记录下来, 下次就不用配对了. 不bonding下次就还得配对.    
    所以我们从机这里把 bonding = FALSE 的后果就是， 主设备每次连接都必须输入密码
    而把  bonding = TRUE 后， 主设备只需第一次连接时输入密码， 后面断开后都不需要再次输入密码即可连接
    ---------------amomcu.com-------------------------    
    */
    uint8 bonding = FALSE;
    pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
     GAPBondMgr_SetParameter(GAPBOND_DEFAULT_PASSCODE, sizeof(uint32), &passkey);
    GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8), &pairMode);
    GAPBondMgr_SetParameter(GAPBOND_MITM_PROTECTION, sizeof(uint8), &mitm);
    GAPBondMgr_SetParameter(GAPBOND_IO_CAPABILITIES, sizeof(uint8), &ioCap);
    GAPBondMgr_SetParameter(GAPBOND_BONDING_ENABLED, sizeof(uint8), &bonding);
  }
  
  // Initialize GATT attributes
  GGS_AddService(GATT_ALL_SERVICES);           // GAP
  GATTServApp_AddService(GATT_ALL_SERVICES);   // GATT attributes
  DevInfo_AddService();                        // Device Information Service
  SimpleProfile_AddService(GATT_ALL_SERVICES); // Simple GATT Profile
#if defined FEATURE_OAD
  VOID OADTarget_AddService(); // OAD Profile
#endif

  // Setup the SimpleProfile Characteristic Values
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    uint8 charValue5[SIMPLEPROFILE_CHAR5_LEN] = {1, 2, 3, 4, 5};
    //    uint8 charValue6[SIMPLEPROFILE_CHAR6_LEN] = { 1, 2, 3, 4, 5 };
    //    uint8 charValue7[SIMPLEPROFILE_CHAR7_LEN] = { 1, 2, 3, 4, 5 };
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR1, sizeof(uint8), &charValue1);
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR2, sizeof(uint8), &charValue2);
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, sizeof(uint8), &charValue3);
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR4, sizeof(uint8), &charValue4);
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5);
    //    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR6, SIMPLEPROFILE_CHAR6_LEN, charValue6 );
    //    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR7, SIMPLEPROFILE_CHAR7_LEN, charValue7 );
  }
  // Register for all key events - This app will handle all key events
  RegisterForKeys(simpleBLETaskId);

#if (defined HAL_LCD) && (HAL_LCD == TRUE)

#if defined FEATURE_OAD
#if defined(HAL_IMAGE_A)
  HalLcdWriteStringValue("BLE Peri-A", OAD_VER_NUM(_imgHdr.ver), 16, HAL_LCD_LINE_1);
#else
  HalLcdWriteStringValue("BLE Peri-B", OAD_VER_NUM(_imgHdr.ver), 16, HAL_LCD_LINE_1);
#endif // HAL_IMAGE_A
#else
  HalLcdWriteString("BLE Peripheral", HAL_LCD_LINE_1);
#endif // FEATURE_OAD

#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

  // Register callback with SimpleGATTprofile
  VOID SimpleProfile_RegisterAppCBs(&simpleBLEPeripheral_SimpleProfileCBs);

  // 需要关闭的CLK自动分频，在初始化中加入HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_DISABLE_CLK_DIVIDE_ON_HALT )?  // 如果开启，会导致频率自动切换，DMA工作受到影响，小范围丢数。
  // 这里把他关闭， 如果想降低功耗， 这个应该要开启的， 这里矛盾了
  HCI_EXT_ClkDivOnHaltCmd(HCI_EXT_DISABLE_CLK_DIVIDE_ON_HALT);
  //HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

  // 信号发射强度
  HCI_EXT_SetTxPowerCmd(3 - sys_config.txPower);

  // Setup a delayed profile startup
  osal_set_event(simpleBLETaskId, START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SimpleBLEPeripheral_ProcessEvent(uint8 task_id, uint16 events)
{

  VOID task_id; // OSAL required parameter that isn't used in this function

  if (events & SYS_EVENT_MSG)
  {
    uint8 *pMsg;

    if ((pMsg = osal_msg_receive(simpleBLETaskId)) != NULL)
    {
      simpleBLEPeripheral_ProcessOSALMsg((osal_event_hdr_t *)pMsg);

      // Release the OSAL message
      VOID osal_msg_deallocate(pMsg);
    }
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if (events & START_DEVICE_EVT)
  {
    // Start the Device
    VOID GAPRole_StartDevice(&simpleBLEPeripheral_PeripheralCBs);

    // Start Bond Manager
    VOID GAPBondMgr_Register(&simpleBLEPeripheral_BondMgrCBs);

    osal_start_timerEx(simpleBLETaskId, SBP_WAKE_EVT, 500);

    return (events ^ START_DEVICE_EVT);
  }

  if (events & SBP_WAKE_EVT)
  {
    osal_pwrmgr_device(PWRMGR_ALWAYS_ON); //  不睡眠，功耗很高的
    g_sleepFlag = FALSE;
    if (check_low_battery() == TRUE)
    {
      enter_low_battery_mode();
      return (events ^ SBP_WAKE_EVT);
    }

    if (first_boot == TRUE)
    {
      first_boot = FALSE;
      led_toggle_set_param(PERIPHERAL_START_LED_TOGGLE_PERIOD, PERIPHERAL_START_LED_TOGGLE_CNT, 0);
      osal_start_timerEx(simpleBLETaskId, SBP_SLEEP_EVT, PERIPHERAL_START_LED_TOGGLE_PERIOD * (PERIPHERAL_START_LED_TOGGLE_CNT));
    }
    else
    {
      // Start advertising
      uint8 initial_advertising_enable = TRUE;
      GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);

      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_INDEX_EVT, SBP_PERIODIC_INDEX_EVT_PERIOD);
      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_PER_HOUR_EVT, SBP_PERIODIC_PER_HOUR_PERIOD);
      // LED
      led_toggle_set_param(PERIPHERAL_START_LED_TOGGLE_PERIOD, PERIPHERAL_WAKEUP_LED_TOGGLE_CNT, 0);
    }
    return (events ^ SBP_WAKE_EVT);
  }

  if (events & SBP_PERIODIC_INDEX_EVT)
  {
    // Restart timer
    if (SBP_PERIODIC_INDEX_EVT_PERIOD)
    {
      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_INDEX_EVT, SBP_PERIODIC_INDEX_EVT_PERIOD);
    }

    // Perform periodic application task
    PeripherialPerformPeriodicTask(SBP_PERIODIC_INDEX_EVT);

    return (events ^ SBP_PERIODIC_INDEX_EVT);
  }

  if (events & SBP_PERIODIC_PER_HOUR_EVT)
  {
    DEBUG_PRINT("SBP_PERIODIC_PER_HOUR_EVT\r\n");
    // Restart timer
    if (SBP_PERIODIC_PER_HOUR_PERIOD)
    {
      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_PER_HOUR_EVT, SBP_PERIODIC_PER_HOUR_PERIOD);
    }

    // Perform periodic application task
    PeripherialPerformPeriodicTask(SBP_PERIODIC_PER_HOUR_EVT);

    return (events ^ SBP_PERIODIC_PER_HOUR_EVT);
  }

  if (events & SBP_PERIODIC_BUTTON_LED_EVT)
  {
    // Restart timer
    if (key_led_count > 0)
    {
      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_BUTTON_LED_EVT, SBP_PERIODIC_BUTTON_LED_PERIOD);
      // Perform periodic application task
      PeripherialPerformPeriodicTask(SBP_PERIODIC_BUTTON_LED_EVT);
    }
    else if (key_led_count == 0)
    {
      key_led_count = BUTTON_LED_TOGGLE_COUNT;
    }

    return (events ^ SBP_PERIODIC_BUTTON_LED_EVT);
  }

  if (events & SBP_PERIODIC_CHN_ADVERT_EVT_PRESS)
  {
    change_advertise_data(TRUE);
    return (events ^ SBP_PERIODIC_CHN_ADVERT_EVT_PRESS);
  }

  if (events & SBP_PERIODIC_CHN_ADVERT_EVT_RELEASE)
  {
    change_advertise_data(FALSE);
    return (events ^ SBP_PERIODIC_CHN_ADVERT_EVT_RELEASE);
  }

  if (events & SBP_DATA_EVT)
  {
    return (events ^ SBP_DATA_EVT);
  }

  if (events & SBP_UART_EVT)
  {
    if (FALSE == timerIsOn)
    {
      //         osal_set_event( simpleBLETaskId, SBP_DATA_EVT );
      osal_start_timerEx(simpleBLETaskId, SBP_DATA_EVT, 6);
    }
    return (events ^ SBP_UART_EVT);
  }

  if (events & SBP_SLEEP_EVT)
  {
    DEBUG_PRINT("SBP_SLEEP_EVT\r\n");
    extern uint8 uart_sleep_count;
    if (uart_sleep_count > 1)
    {
      low_power_state = FALSE; // set false to enable key event.
      g_sleepFlag = TRUE;
      osal_pwrmgr_device(PWRMGR_BATTERY); //  自动睡眠
      osal_stop_timerEx(simpleBLETaskId, SBP_PERIODIC_EVT_ALL);
      uint8 initial_advertising_enable = FALSE;
      GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);
      DEBUG_PRINT("Enter Sleep Mode\r\n");
      // 为了让串口数据发送完毕， 需要先延时一下，否则进入了睡眠就发送乱码了， 1ms即可
      simpleBLE_Delay_1ms(1);
    }
    else
    {
      uart_sleep_count++;
      osal_start_timerEx(simpleBLETaskId, SBP_SLEEP_EVT, SLEEP_MS);
    }
    return (events ^ SBP_SLEEP_EVT);
  }

  if (events & SBP_PERIODIC_EVT)
  {
    // Restart timer
    if (SBP_PERIODIC_EVT_PERIOD)
    {
      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD);
    }
    return (events ^ SBP_PERIODIC_EVT);
  }

  if (events & SBP_PERIODIC_LED_EVT)
  {
    if (++led_toggle_count <= led_toggle_cnt_target)
    {
      HalLedSet(HAL_LED_1, HAL_LED_MODE_TOGGLE);
      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_LED_EVT, led_toggle_period);
    }
    else
    {
      HalLedSet(HAL_LED_1, HAL_LED_MODE_OFF);
      osal_stop_timerEx(simpleBLETaskId, SBP_PERIODIC_LED_EVT);
      led_toggle_clean_param();
      if (low_power_state == TRUE)
      {
        osal_set_event(simpleBLETaskId, SBP_SLEEP_EVT);
        low_power_state = FALSE;
      }
    }
    return (events ^ SBP_PERIODIC_LED_EVT);
  }

  if (events & SBP_KEY_CNT_EVT)
  {
    if (key_pressed_count >= 2 && g_sleepFlag == TRUE)
    {
      osal_set_event(simpleBLETaskId, SBP_WAKE_EVT);
    }
    else if (key_pressed_count == 1 && g_sleepFlag == TRUE)
    {
      first_boot = TRUE;
      osal_set_event(simpleBLETaskId, SBP_WAKE_EVT);
    }
    else if (key_pressed_count >= 2 && g_sleepFlag == FALSE)
    {
      DEBUG_PRINT("Timer is reset\r\n");
      // reset timer count.
      wake_up_hours_remain = DEFAULT_WAKE_TIME_HOURS;
      // LED. blink twice.
      led_toggle_set_param(PERIPHERAL_START_LED_TOGGLE_PERIOD, PERIPHERAL_WAKEUP_LED_TOGGLE_CNT, 0);
    }
    else if(key_pressed_count == 1 && g_sleepFlag == FALSE)
    {
      led_toggle_set_param(PERIPHERAL_START_LED_TOGGLE_PERIOD, BUTTON_LED_TOGGLE_COUNT, 0);
      change_advertise_data(TRUE);
    }
    key_pressed_count = 0;
    key_processing = FALSE;    
    return (events ^ SBP_KEY_CNT_EVT);
  }
  // Discard unknown events
  return 0;
}


static uint8 led_toggle_set_param(uint16 toggle_period, uint32 toggle_target_cnt, uint16 delay)
{
  if (led_toggling == TRUE)
    return FALSE;
  led_toggling = TRUE;
  led_toggle_period = toggle_period;
  led_toggle_count = 0;
  led_toggle_cnt_target = toggle_target_cnt;
  osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_LED_EVT, delay);
  return TRUE;
}
static uint8 led_toggle_clean_param()
{
  if (led_toggling == FALSE)
    return FALSE;
  led_toggle_period = PERIPHERAL_START_LED_TOGGLE_PERIOD;
  led_toggle_count = 0;
  led_toggle_cnt_target = PERIPHERAL_START_LED_TOGGLE_CNT;
  led_toggling = FALSE;
  return TRUE;
}

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessOSALMsg(osal_event_hdr_t *pMsg)
{
  switch (pMsg->event)
  {
  case KEY_CHANGE:
    simpleBLEPeripheral_HandleKeys(((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys);
    break;

  case GATT_MSG_EVENT:
    // Process GATT message
    simpleBLEPeripheral_ProcessGATTMsg((gattMsgEvent_t *)pMsg);
    break;

  default:
    // do nothing
    break;
  }
}

//#if defined( CC2540_MINIDK )
/*********************************************************************
 * @fn      simpleBLEPeripheral_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void simpleBLEPeripheral_HandleKeys(uint8 shift, uint8 keys)
{
  /*
    按键处理公共函数， 主机与从机都是运行这个函数，
    注意每次启动不是主机就是从机，不是同时是主机与从机的， 所以他们不冲突的
    */
  if (keys & HAL_KEY_SW_6)
  {
    if (low_power_state == TRUE)
    {
      DEBUG_PRINT("Handle Keys, Low Power Mode\r\n");
      return;
    }

    if (key_processing == FALSE)
    {
      if (key_pressed_count == 0)
      {
        osal_start_timerEx(simpleBLETaskId, SBP_KEY_CNT_EVT, PERIPHERAL_KEY_CALCULATE_PERIOD);
      }
      key_pressed_count++;
    }
    key_pressed = !key_pressed;
  }
  #ifdef DEBUG_BOARD
  if (keys & HAL_KEY_SW_1)
  {
    debug_low_power = TRUE;
  }
  if (keys & HAL_KEY_SW_3)
  {
    debug_low_power = FALSE;
  }
  #endif
}
//#endif // #if defined( CC2540_MINIDK )

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessGATTMsg
 *
 * @brief   Process GATT messages
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessGATTMsg(gattMsgEvent_t *pMsg)
{
  GATT_bm_free(&pMsg->msg, pMsg->method);
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB(gaprole_States_t newState)
{
  switch (newState)
  {
  case GAPROLE_STARTED:
  {
    uint8 ownAddress[B_ADDR_LEN];
    uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

    GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

    // use 6 bytes of device address for 8 bytes of system ID value
    systemId[0] = ownAddress[0];
    systemId[1] = ownAddress[1];
    systemId[2] = ownAddress[2];

    // set middle bytes to zero
    systemId[4] = 0x00;
    systemId[3] = 0x00;

    // shift three bytes up
    systemId[7] = ownAddress[5];
    systemId[6] = ownAddress[4];
    systemId[5] = ownAddress[3];

    DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
    // Display device address
    HalLcdWriteString(bdAddr2Str(ownAddress), HAL_LCD_LINE_2);
    HalLcdWriteString("Initialized", HAL_LCD_LINE_3);
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

    //mac 地址   bdAddr2Str( pEvent->initDone.devAddr ) 的输出格式是， 例如     0xD03972A5F3
    osal_memcpy(sys_config.mac_addr, bdAddr2Str(ownAddress) + 2, 12);
    sys_config.mac_addr[12] = 0;
    //PrintAllPara();
  }
  break;

  case GAPROLE_ADVERTISING:
  {
#if (defined HAL_LCD) && (HAL_LCD == TRUE)
    HalLcdWriteString("Advertising", HAL_LCD_LINE_3);
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
  }
  break;

  case GAPROLE_CONNECTED:
  {
#if (defined HAL_LCD) && (HAL_LCD == TRUE)
    HalLcdWriteString("Connected", HAL_LCD_LINE_3);
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
    simpleBle_LedSetState(HAL_LED_MODE_OFF);
    simpleBLE_Delay_1ms(1); //为了等发送完整所以延时一小下

    // Get connection handle
    GAPRole_GetParameter(GAPROLE_CONNHANDLE, &gapConnHandle);
    // 连接上了
    g_sleepFlag = FALSE;
    osal_pwrmgr_device(PWRMGR_ALWAYS_ON); //  不睡眠，功耗很高的
  }
  break;

  case GAPROLE_WAITING:
  {
#if (defined HAL_LCD) && (HAL_LCD == TRUE)
    HalLcdWriteString("In State Waiting", HAL_LCD_LINE_3);
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
  }
  break;

  case GAPROLE_WAITING_AFTER_TIMEOUT:
  {
#if (defined HAL_LCD) && (HAL_LCD == TRUE)
    HalLcdWriteString("Timed Out", HAL_LCD_LINE_3);
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
  }
  break;

  case GAPROLE_ERROR:
  {
#if (defined HAL_LCD) && (HAL_LCD == TRUE)
    HalLcdWriteString("Error", HAL_LCD_LINE_3);
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
  }
  break;

  default:
  {
#if (defined HAL_LCD) && (HAL_LCD == TRUE)
    HalLcdWriteString("", HAL_LCD_LINE_3);
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
  }
  break;
  }

  gapProfileState = newState;

#if !defined(CC2540_MINIDK)
  VOID gapProfileState; // added to prevent compiler warning with
                        // "CC2540 Slave" configurations
#endif

  //LCD_WRITE_STRING( "?", HAL_LCD_LINE_1 );
}

static void peripheralRssiReadCB(int8 rssi)
{
  return;
}

/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void simpleProfileChangeCB(uint8 paramID)
{
  uint8 newValue;
  uint8 newChar6Value[SIMPLEPROFILE_CHAR6_LEN];
  uint8 returnBytes;

  switch (paramID)
  {
  case SIMPLEPROFILE_CHAR1:
    SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR1, &newValue, &returnBytes);

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
    HalLcdWriteStringValue("Char 1:", (uint16)(newValue), 10, HAL_LCD_LINE_3);
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

    break;

  case SIMPLEPROFILE_CHAR3:
    SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR3, &newValue, &returnBytes);

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
    HalLcdWriteStringValue("Char 3:", (uint16)(newValue), 10, HAL_LCD_LINE_3);
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

    break;

  case SIMPLEPROFILE_CHAR6:
    SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR6, newChar6Value, &returnBytes);
    break;
  default:
    // should not reach here!
    break;
  }
}

//#if defined( BLE_BOND_PAIR )
//绑定过程中的密码管理回调函数
static void ProcessPasscodeCB(uint8 *deviceAddr, uint16 connectionHandle, uint8 uiInputs, uint8 uiOutputs)
{
  uint32 passcode;
  uint8 str[7];

//在这里可以设置存储，保存之前设定的密码，这样就可以动态修改配对密码了。
// Create random passcode
  LL_Rand( ((uint8 *) &passcode), sizeof( uint32 ));
  passcode %= 1000000;
  //在lcd上显示当前的密码，这样手机端，根据此密码连接。
  // Display passcode to user
  if (uiOutputs != 0)
  {
    HalLcdWriteString("Passcode:", HAL_LCD_LINE_1);
    HalLcdWriteString((char *)_ltoa(passcode, str, 10), HAL_LCD_LINE_2);
  }
  HalLcdWriteString(">>>no Passcode", HAL_LCD_LINE_1);
}

//绑定过程中的状态管理，在这里可以设置标志位，当密码不正确时不允许连接。
static void ProcessPairStateCB(uint16 connHandle, uint8 state, uint8 status)
{
  // 主机发起连接，会进入开始绑定状态
  if (state == GAPBOND_PAIRING_STATE_STARTED)
  {
    HalLcdWriteString("Pairing started", HAL_LCD_LINE_1);
    gPairStatus = BOND_PAIR_STATUS_PAIRING;
  }
  // 当主机提交密码后，会进入完成
  else if (state == GAPBOND_PAIRING_STATE_COMPLETE)
  {
    if (status == SUCCESS)
    {
      HalLcdWriteString("Pairing success", HAL_LCD_LINE_1); /*密码正确*/
      gPairStatus = BOND_PAIR_STATUS_PAIRED;
    }
    else
    {
      HalLcdWriteStringValue("Pairing fail", status, 10, HAL_LCD_LINE_1); /*密码不正确，或者先前已经绑定*/

      if (status == 8)
      { //已绑定
        gPairStatus = BOND_PAIR_STATUS_PAIRED;
      }
      else
      {
        gPairStatus = BOND_PAIR_STATUS_PAIRING;

        GAPRole_TerminateConnection(); // 终止连接
        simpleBLE_Delay_1ms(100);

        // 终止连接后， 需要复位从机
        HAL_SYSTEM_RESET();
      }

      gPairStatus = BOND_PAIR_STATUS_PAIRING;
    }

    //判断配对结果，如果不正确立刻停止连接。
    if ((gapProfileState == GAPROLE_CONNECTED) && (gPairStatus == BOND_PAIR_STATUS_PAIRING))
    {
      GAPRole_TerminateConnection(); // 终止连接
      // 终止连接后， 需要复位从机
      HAL_SYSTEM_RESET();
    }
  }
  // 当主机提交密码从机验证后进入配对成功状态
  else if (state == GAPBOND_PAIRING_STATE_BONDED)
  {
    if (status == SUCCESS)
    {
      HalLcdWriteString("Bonding success", HAL_LCD_LINE_1);
    }
  }
}

static void PeripherialPerformPeriodicTask(uint16 event_id)
{
  if (low_power_state == TRUE)
  {
    return;
  }

  switch (event_id)
  {
  case SBP_PERIODIC_INDEX_EVT:
    {
      uint16 index;
      index = (advertData_iBeacon[25] << 8) + advertData_iBeacon[26];
      index++;
      advertData_iBeacon[25] = index >> 8;
      advertData_iBeacon[26] = index & 0x00FF;
      GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData_iBeacon), advertData_iBeacon);
    }
    break;
  case SBP_PERIODIC_PER_HOUR_EVT:
    if (g_sleepFlag == TRUE)
    {
      DEBUG_PRINT("sleep already, stop the per hour timer\r\n");
      osal_stop_timerEx(simpleBLETaskId, SBP_PERIODIC_PER_HOUR_EVT);
    }
    else
    {
      DEBUG_PRINT("This is a per hour event\r\n");
      if (check_low_battery() == TRUE)
      {
        enter_low_battery_mode();
      }
      else
      {
        wake_up_hours_remain--;
        if (wake_up_hours_remain == 0)
        {
          DEBUG_PRINT("Enter Sleep mode\r\n");
          osal_start_timerEx(simpleBLETaskId, SBP_SLEEP_EVT, SLEEP_MS);
        }
      }
    }
    break;
  case SBP_PERIODIC_BUTTON_LED_EVT:
    HalLedSet(HAL_LED_1, HAL_LED_MODE_TOGGLE);
    key_led_count--;
    break;
  default:
    break;
  }
}

#define ENABLE_DISABLE_PERIOD 500
static bool rapid_processing = FALSE;
static void change_advertise_data(uint8 key_pressed)
{
  uint8 initial_advertising_enable;
  GAPRole_GetParameter(GAPROLE_ADVERT_ENABLED, &initial_advertising_enable);
  if (key_pressed == TRUE)
  {
    if (initial_advertising_enable == TRUE && rapid_processing == FALSE)
    {
      DEBUG_PRINT("Enter Rapid Mode\r\n");
      rapid_processing = TRUE;
      initial_advertising_enable = FALSE;
      GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);
      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_CHN_ADVERT_EVT_PRESS, ENABLE_DISABLE_PERIOD); // 1s for test.
    }
    else if (initial_advertising_enable == FALSE && rapid_processing == TRUE)
    {
      advertData_iBeacon[27] |= 0x80;
      // Set advertising interval
      {
        advInt = RAPID_ADVERTISING_INTERVAL;
        GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, advInt);
        GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, advInt);
        initial_advertising_enable = TRUE;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);
      }
      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_CHN_ADVERT_EVT_RELEASE, SBP_PERIODIC_ADVERT_CHG_PERIOD); // 1s for test.
    }
  }
  else
  {
    if (initial_advertising_enable == TRUE && rapid_processing == TRUE)
    {
      initial_advertising_enable = FALSE;
      GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);
      osal_start_timerEx(simpleBLETaskId, SBP_PERIODIC_CHN_ADVERT_EVT_RELEASE, ENABLE_DISABLE_PERIOD); // 1s for test.
    }
    else if (initial_advertising_enable == FALSE && rapid_processing == TRUE)
    {
      DEBUG_PRINT("Exit Rapid Mode\r\n");
      advertData_iBeacon[27] &= 0x7F;
      // Set advertising interval
      {
        advInt = SLOW_ADVERTISING_INTERVAL;
        GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, advInt);
        GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, advInt);
        initial_advertising_enable = TRUE;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);
      }
      rapid_processing = FALSE;
    }
  }
}

static bool check_low_battery()
{
  #ifdef DEBUG_BOARD
  if (debug_low_power == TRUE)
    return TRUE;
  #endif

  uint32 adc_read = 0;
  for (uint8 i = 0; i < 16; i++)
  {
    HalAdcSetReference(HAL_ADC_REF_125V);
    adc_read += HalAdcRead(HAL_ADC_CHANNEL_VDD, HAL_ADC_RESOLUTION_10);
  }
  adc_read = adc_read >> 4;
  battery_voltage = adc_read * 3 * 125 / 511 / 10;
  DEBUG_VALUE("adc value : ", adc_read, 10);
  DEBUG_VALUE("Battery Value : ", battery_voltage * 10, 10);
  // Update the advertise data.
  advertData_iBeacon[28] = battery_voltage & 0xFF;
  if (battery_voltage < BATTERY_LOW_THRESHOLD)
  {
    return TRUE;
  }
  return FALSE;
}

static void enter_low_battery_mode()
{
  DEBUG_PRINT("Enter Low Battery Mode\r\n");
  low_power_state = TRUE;
  // Stop advertising.
  uint8 initial_advertising_enable = FALSE;
  GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);
  // LED Blinking.
  led_toggle_set_param(PERIPHERAL_LOW_BAT_LED_TOGGLE_PERIOD, PERIPHERAL_LOW_BAT_LED_TOGGLE_CNT, 0);
  //osal_start_timerEx(simpleBLETaskId, SBP_SLEEP_EVT, (PERIPHERAL_LOW_BAT_LED_TOGGLE_PERIOD * PERIPHERAL_LOW_BAT_LED_TOGGLE_CNT));
}
//#endif
/*********************************************************************
*********************************************************************/