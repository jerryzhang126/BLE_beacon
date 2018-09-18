@REM This batch file has been generated by the IAR Embedded Workbench
@REM C-SPY Debugger, as an aid to preparing a command line for running
@REM the cspybat command line utility using the appropriate settings.
@REM
@REM Note that this file is generated every time a new debug session
@REM is initialized, so you may want to move or rename the file before
@REM making changes.
@REM
@REM You can launch cspybat by typing the name of this batch file followed
@REM by the name of the debug file (usually an ELF/DWARF or UBROF file).
@REM
@REM Read about available command line parameters in the C-SPY Debugging
@REM Guide. Hints about additional command line parameters that may be
@REM useful in specific cases:
@REM   --download_only   Downloads a code image without starting a debug
@REM                     session afterwards.
@REM   --silent          Omits the sign-on message.
@REM   --timeout         Limits the maximum allowed execution time.
@REM 


@echo off 

if not "%~1" == "" goto debugFile 

@echo on 

"D:\IAR_Systems\Embedded_Workbench_7_5\common\bin\cspybat" -f "G:\PersonalProject\BLE-CC2541\BLE_Meeting\CC2640_NEW\examples\cc2650lp\simple_peripheral\iar\stack\settings\cc2650lp_stack.FlashROM.general.xcl" --backend -f "G:\PersonalProject\BLE-CC2541\BLE_Meeting\CC2640_NEW\examples\cc2650lp\simple_peripheral\iar\stack\settings\cc2650lp_stack.FlashROM.driver.xcl" 

@echo off 
goto end 

:debugFile 

@echo on 

"D:\IAR_Systems\Embedded_Workbench_7_5\common\bin\cspybat" -f "G:\PersonalProject\BLE-CC2541\BLE_Meeting\CC2640_NEW\examples\cc2650lp\simple_peripheral\iar\stack\settings\cc2650lp_stack.FlashROM.general.xcl" "--debug_file=%~1" --backend -f "G:\PersonalProject\BLE-CC2541\BLE_Meeting\CC2640_NEW\examples\cc2650lp\simple_peripheral\iar\stack\settings\cc2650lp_stack.FlashROM.driver.xcl" 

@echo off 
:end