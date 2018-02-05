#
#  Do not edit this file.  This file is generated from 
#  package.bld.  Any modifications to this file will be 
#  overwritten whenever makefiles are re-generated.
#
#  target compatibility key = iar.targets.arm.M3{1,0,8.20,2
#
ifeq (,$(MK_NOGENDEPS))
-include package/cfg/app_ble_prm3.orm3.dep
package/cfg/app_ble_prm3.orm3.dep: ;
endif

package/cfg/app_ble_prm3.orm3: | .interfaces
package/cfg/app_ble_prm3.orm3: package/cfg/app_ble_prm3.c package/cfg/app_ble_prm3.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm3 $< ...
	LC_ALL=C $(iar.targets.arm.M3.rootDir)/bin/iccarm    -D BOARD_DISPLAY_EXCLUDE_UART   -D CC2650DK_4XS   -D CC26XX   -D HEAPMGR_SIZE=0   -D ICALL_MAX_NUM_ENTITIES=6   -D ICALL_MAX_NUM_TASKS=3   -D POWER_SAVING   -D USE_ICALL   -D xBOARD_DISPLAY_EXCLUDE_LCD   -D xDisplay_DISABLE_ALL   -D xdc_runtime_Assert_DISABLE_ALL   -D xdc_runtime_Log_DISABLE_ALL   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/controller/cc26xx/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/common/cc26xx/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/icall/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/dev_info/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/roles/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/roles/cc26xx/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/simple_profile/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/simple_profile/cc26xx/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/target/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/hal/src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/hal/src/target/_common/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/hal/src/target/_common/cc26xx/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/hal/src/target/cc2650/rom/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/heapmgr/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/icall/src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/osal/src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/services/src/saddr/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/services/src/sdata/   -I C:/ti/tirtos_cc13xx_cc26xx_2_20_01_08/products/cc26xxware_2_24_02_17393/   -I C:/ti/tirtos_cc13xx_cc26xx_2_20_01_08/products/tidrivers_cc13xx_cc26xx_2_20_01_10/packages/   --silent --aeabi --cpu=Cortex-M3 --diag_suppress=Pa050,Go005 --endian=little -e --thumb   -Dxdc_cfg__xheader__='"configPkg/package/cfg/app_ble_prm3.h"'  -Dxdc_target_name__=M3 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_release -Dxdc_bld__vers_1_0_8_20_2 -Ohs --dlib_config $(iar.targets.arm.M3.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

package/cfg/app_ble_prm3.srm3: | .interfaces
package/cfg/app_ble_prm3.srm3: package/cfg/app_ble_prm3.c package/cfg/app_ble_prm3.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm3 $< ...
	LC_ALL=C $(iar.targets.arm.M3.rootDir)/bin/iccarm    -D BOARD_DISPLAY_EXCLUDE_UART   -D CC2650DK_4XS   -D CC26XX   -D HEAPMGR_SIZE=0   -D ICALL_MAX_NUM_ENTITIES=6   -D ICALL_MAX_NUM_TASKS=3   -D POWER_SAVING   -D USE_ICALL   -D xBOARD_DISPLAY_EXCLUDE_LCD   -D xDisplay_DISABLE_ALL   -D xdc_runtime_Assert_DISABLE_ALL   -D xdc_runtime_Log_DISABLE_ALL   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/controller/cc26xx/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/common/cc26xx/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/icall/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/dev_info/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/roles/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/roles/cc26xx/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/simple_profile/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/profiles/simple_profile/cc26xx/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/target/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/hal/src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/hal/src/target/_common/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/hal/src/target/_common/cc26xx/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/hal/src/target/cc2650/rom/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/heapmgr/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/icall/src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/osal/src/inc/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/services/src/saddr/   -I F:/PersonalProject/BLE-CC2541/BLE_Meeting/CC2640/examples/cc2650em/simple_peripheral/iar/app/../../../../../src/components/services/src/sdata/   -I C:/ti/tirtos_cc13xx_cc26xx_2_20_01_08/products/cc26xxware_2_24_02_17393/   -I C:/ti/tirtos_cc13xx_cc26xx_2_20_01_08/products/tidrivers_cc13xx_cc26xx_2_20_01_10/packages/   --silent --aeabi --cpu=Cortex-M3 --diag_suppress=Pa050,Go005 --endian=little -e --thumb   -Dxdc_cfg__xheader__='"configPkg/package/cfg/app_ble_prm3.h"'  -Dxdc_target_name__=M3 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_release -Dxdc_bld__vers_1_0_8_20_2 -Ohs --dlib_config $(iar.targets.arm.M3.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

clean,rm3 ::
	-$(RM) package/cfg/app_ble_prm3.orm3
	-$(RM) package/cfg/app_ble_prm3.srm3

app_ble.prm3: package/cfg/app_ble_prm3.orm3 package/cfg/app_ble_prm3.mak

clean::
	-$(RM) package/cfg/app_ble_prm3.mak
