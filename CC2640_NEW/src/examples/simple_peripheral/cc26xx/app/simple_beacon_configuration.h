#ifndef __SIMPLE_BEACON_CONFIGURATION_H
#define __SIMPLE_BEACON_CONFIGURATION_H

#define DEFAULT_BOOT_UP_BLINK           true
#define DEFAULT_WAKE_TIME_MINS          60 //8bits
#define DEFAULT_SCAN_ADV_TRANS_MINS     20 //8bits
#define DEFAULT_OFF_SCAN_PERIOD_MIN     60 //Only 16 bits, max 109 Mins.
#define DEFAULT_OFF_SCAN_PERIOD_x00MS   (DEFAULT_OFF_SCAN_PERIOD_MIN * 60 * 10)

#define DEFAULT_ADVERTISING_INTERVAL    (160 * 20) //100ms * 20 = 2s
#define RAPID_ADVERTISING_INTERVAL      (160 * 2) //100ms * 2 = 200ms

#define RAPID_STATE_PERIOD              (10000) //10s
#define COMMS_STATE_PERIOD              (10000) //10s

#define LED_BLINK_ON_PERIOD         (100) //ms
#define LED_BLINK_OFF_PERIOD        (200) //ms
#define LED_BLINK_RAPID_OFF_PERIOD  (100) //ms

#define VIBRA_SENSOR_CHECK_PERIOD   (60000) //ms

#define DEF_TX_POWER        HCI_EXT_TX_POWER_MINUS_6_DBM
#define MAX_TX_POWER        HCI_EXT_TX_POWER_2_DBM

#endif

