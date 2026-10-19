# GM8 Android 12 early-boot logging

The uploaded boot image contains kernel 3.18.124-perf-g748338524ae4 with
CONFIG_PSTORE disabled. The supplied recovery log identifies the active board
as Qualcomm MSM8940-PMI8937 MTP. This temporary bring-up change targets that
DTB only; other DTBs receive no ramoops address or memory reservation.

The verified board reserves 0x9ff00000..0xa0000000 (1 MiB) before the normal
allocator starts. It does not overlap the uploaded board's fixed firmware and
splash reservations or the CMA locations shown by recovery. The mapping is
excluded from ordinary RAM with no-map. No storage partition is repurposed.

This tree's 3.18 ramoops driver does not probe DT nodes. The same DTS include
therefore supplies its existing module parameters through chosen/bootargs:
512 KiB console, 256 KiB pmsg, one 256 KiB panic/oops record, no ftrace buffer,
and the driver's 16-byte ECC mode. Console capture does not depend on a panic.

Android 12 init fatal errors request recovery rather than another normal boot.
Kernel panics keep the existing reset policy. This instrumentation does not
claim to fix the boot failure; it is intended to preserve its diagnostic log.

Build bootimage with the existing lineage_GM8_sprout-userdebug configuration.
Install only the newly generated boot image to the active boot slot. Retain
the original boot image for rollback. The image contains Lineage Recovery.

After one failed boot, enter Lineage Recovery from this same image without
powering off. Its init.rc mounts pstore. Pull /sys/fs/pstore over ADB before
another boot attempt. A hard reset/power loss or bootloader RAM clearing may
still destroy the records. The old 2021 TWRP kernel has no pstore backend and
cannot expose these files; loading it can also reuse the reserved RAM.

Source/DTB validation is separate from the on-device retention test. Remove
the gm8-pstore-debug.dtsi include to drop the diagnostic address, reservation,
and init reboot target once bring-up is complete.
