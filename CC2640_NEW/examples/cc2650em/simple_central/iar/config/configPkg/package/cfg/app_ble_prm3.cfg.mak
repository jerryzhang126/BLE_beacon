# invoke SourceDir generated makefile for app_ble.prm3
app_ble.prm3: .libraries,app_ble.prm3
.libraries,app_ble.prm3: package/cfg/app_ble_prm3.xdl
	$(MAKE) -f F:\PersonalProject\BLE-CC2541\BLE_Meeting\CC2640_NEW\examples\cc2650em\simple_central\iar\config/src/makefile.libs

clean::
	$(MAKE) -f F:\PersonalProject\BLE-CC2541\BLE_Meeting\CC2640_NEW\examples\cc2650em\simple_central\iar\config/src/makefile.libs clean
