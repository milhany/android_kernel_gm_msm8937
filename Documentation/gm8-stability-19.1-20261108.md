# GM8 Android 12.1 stability port

This ports the reviewed Android 10 QSEE final-close and FT8006 gesture
candidate onto `b0555eee97c41a59830d94134b4c882c75027220`. The underlying
driver files are byte-identical to the Android 10 starting versions.
The Android 12.1 kernel config, other drivers, frequency/voltage tables
and thermal limits are unchanged.

Final release of a non-reentrant QSEE listener now marks that listener's
atomic close flag before acquiring app_access_lock. A blocked legacy
command wakes, sends FAILURE through the existing TZ response path and
reports an error; the listener entry and ION buffer remain owned by the
existing serialized unregister path. Other listeners' close flags do not
cancel the active request. Reentrant QSEE and explicit UNREGISTER ioctl
are deliberately unchanged. The daemon-termination trigger is unresolved.

FT8006 gesture operations use a per-device mutex, propagate I2C errors and
balance only successfully acquired IRQ wake references. Resume and remove
attempt to release owned references even when touch I2C is unavailable.

The paired device tree carries ft8006_gesture_test.py, qsee_wait_test.py
and qsee_host.c. Actual extracted function bodies passed strict GNU89
compilation, fault injection, baseline QSEE deadlock reproduction, 100
close races and ASAN/UBSAN host checks. Host shims do not model real TZ,
IRQ delivery, physical ION or ARM64 weak memory behavior. LeakSanitizer
could not be used in the preparation runtime. Full kernel/Android build
and phone acceptance are pending.

These are correctness candidates, not measured power savings. IRQ wake
ownership and QSEE progress can affect stability, but do not establish the
cause of every battery drain, application stall or thermal observation.
Use with device/vendor `lineage-19.1-stability-20261108`.
