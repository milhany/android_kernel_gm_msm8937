# GM8 Android 10 kernel baseline

Initial lineage-17.1 starts from the stock-derived 3.18 kernel:
1f1981295c7507163310c812e4a430be6f8d8777 (lineage-18.1-rework).

Retain the existing panel/touch/modem/charger drivers and boot layout. No
4.19 port, PSI backport, userspace-LMK experiment, CPU overclock or thermal
policy removal is part of this baseline. This source checkpoint does not
establish bootability, Android 10 compatibility or performance.

Carry the already-reviewed input-clock quirk fix from 44e0901: private key/
headset resume quirks use input_dev.no_dummy_release instead of reusing public
input-property bit 0x06 (INPUT_PROP_ACCELEROMETER). No other 19.1 kernel
changes are imported. This source carry has not been recompiled here.

Use only with the paired lineage-17.1 device/vendor sources. A full linked
kernel and Android target-files build remain required. Keep the existing
working recovery and backup; do not install an isolated boot image over
the Android 12 build.
