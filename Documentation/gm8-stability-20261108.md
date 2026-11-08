# GM8 3.18 stability candidate

Prepared for LineageOS 17.1 on 2026-11-08. Full ARM64 build and hardware
acceptance are pending. Base revision:
`79997531c5a3b9cf52d608c09e1c584e379f1ff9`.

## FT8006

Gesture configuration is serialized by a per-device mutex. IRQ wake is
disabled only when that device successfully acquired it; ownership remains
set if the disable call fails. Failed I2C operations propagate errors rather
than being interpreted as a valid enable/readback. Resume attempts to release
the wake vote even after an I2C error. Device teardown removes the sysfs group
and releases gesture wake before the IRQ is freed.

The companion device tree carries a host fault-injection test using the
actual gesture functions. This does not establish physical touch/power
behavior or fully model kernel scheduling/IRQ lifetime.

## QSEE legacy final close

The captured qseecomd stack was blocked on `mutex_lock` inside
`qseecom_release+0x5c`. Disassembly of the matching vmlinux establishes that
this is the listener close branch, before `qseecom_unregister_listener`.
Keymaster was simultaneously sleeping in the non-reentrant SEND_CMD
incomplete-command path. That path retains `app_access_lock` while waiting
for a listener response, while listener final close needs the same mutex.

The candidate adds a listener-local atomic final-close flag. Final release
marks only its matching listener under the existing list spinlock, releases
that spinlock, and wakes the global response queue before trying to acquire
the app mutex. The legacy waiter treats that flag as cancellation and sends
FAILURE through the existing TZ response/cache/clock path. It reports an
error rather than successful key generation. The listener entry and ION
buffer remain alive under the original app-mutex teardown ordering until
the legacy command releases the mutex.

The helper is a no-op when QSEE reentrancy is enabled. Reentrant response
processing and explicit UNREGISTER ioctl behavior are unchanged. There is
no timeout, forced unlock, fake response flag, key deletion or backend
replacement. The reason for daemon termination remains unresolved.

The device companion's `qsee_wait_17_1_test.py` extracts the actual wait,
release and unregister function bodies and runs them against host
pthread/TZ/ION shims. The baseline close/wait deadlock was reproduced.
Candidate tests cover genuine reply, final close, SCM errors, unrelated
listener close, close before wait, reentrancy guard, missing listener,
deregister failure preserving buffers, chained TZ responses and 100 close
races. Both listener message widths and whitelist modes are exercised.
GNU89 strict compilation, ASAN and UBSAN passed. LeakSanitizer could not run
under the preparation environment's ptrace setup and was disabled; no leak
validation is claimed. These tests do not model real TZ or ARM64 weak memory.

Companion changes also include an exact camera property mapping and the
GNSS init interface registration. Build all three repositories from the
matching `lineage-17.1-stability-20261108` branch and keep the existing
release signing identities.
