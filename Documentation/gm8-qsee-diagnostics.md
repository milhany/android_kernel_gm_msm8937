# GM8 QSEE listener diagnostics

This temporary lineage-19.1-qsee-diag branch adds kernel log messages around
the existing listener wait and client/listener close locks. It is based on
b0555eee97c41a59830d94134b4c882c75027220. The normal lineage-19.1 kernel branch
is unchanged.

## Evidence from the October 21 capture

The Play Store ANR main thread waits in
IKeystoreSecurityLevel.generateKey / KeyGenerator.generateKey. Later,
system_server waits in IKeystoreAuthorization.onLockScreenEvent. Watchdog kills
system_server at 18:56:41, 19:01:09, 19:03:08 and 19:04:59. Bluetooth ANRs wait in
IKeystoreService.getKeyEntry; subsequent system-server starts also wait on
legacy-keystore VPN profile reads. This is a shared keystore failure, rather
than proof of a GPU crash.

Thread snapshots show Keymaster in __qseecom_process_incomplete_cmd. Live dmesg
contains repeated kernel stacks with qseecomd exiting through do_exit/__fput
and blocked on mutex_lock inside qseecom_release; Gatekeeper and fingerprint
also wait on mutex_lock inside qseecom_ioctl. The source holds app_access_lock
across the client request/listener wait and takes it in the close path.

The samples do not identify which listener is missing a response or prove
which handle the exiting process closes. Keystore ptrace remains denied in
the user build. No raw bugreports or personal identifiers are committed here.

## Added messages

- GM8 QSEE wait: process/thread IDs, command name, app ID, listener ID and
  reentrancy mode before the existing wait.
- GM8 QSEE wait returned: IDs and the existing abort state after it returns.
- GM8 QSEE close listener / close app: handle metadata before the existing
  app_access_lock acquisition, followed by a locked marker after acquisition.

No key material, command payload, buffer address, credential or certificate
is logged. These are diagnostic messages, not a deadlock repair. They add no
timeout, retry, unlock, fake success, key deletion or backend fallback. The
existing waits, mutex operations, response checks and return values are
unchanged. Logging can affect timing; keep this on the temporary branch and
capture a short reproduction.

## Build and capture

Use the existing Android 12.1 device/vendor trees and release signing keys.
Select this kernel branch before the normal target-files user build:

```sh
git -C kernel/gm/msm8937 fetch \
  https://github.com/milhany/android_kernel_gm_msm8937.git lineage-19.1-qsee-diag &&
git -C kernel/gm/msm8937 switch --detach FETCH_HEAD &&
bash device/gm/GM8_sprout/tools/build-19.1.sh target-files user
```

Sign/install through the existing GM8 release helper. Preserve the current
keys and normal update path. The ordinary all-fixes command selects the normal
kernel branch, so replace its kernel fetch branch with lineage-19.1-qsee-diag
for this diagnostic build. A userdebug transition is not needed for this
kernel-log experiment.

After a full boot, reproduce the first key-operation stall, then collect:

```bat
adb shell ps -A -T > gm8-qsee-threadler.txt
adb bugreport gm8-qsee-diag.zip
```

The earlier user bugreport contains live dmesg, so use that section to find the
GM8 QSEE markers. Preserve the report before another full reboot; framework-only
restarts can leave the native secure services stuck. Look for a wait without a
matching return and a close without a following locked marker. Compare listener
and app IDs instead of assuming a particular service from its process name.

Host validation removes the six marked logging blocks and compares the remaining
source to the pinned base, and compiles the extracted log statements for type/
format correctness. A complete arm64 kernel build and phone test still run on
the owner's build host. To return to the normal kernel, select lineage-19.1 and
produce another signed update using the same keys.
