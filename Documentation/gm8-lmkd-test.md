# GM8 Android 12 userspace-LMK candidate

Experimental branch: `lineage-19.1-lmkd-test`. Base kernel:
`44e09019575c012ad5b332250261d162ea5835d5` (3.18).

Enable the existing memcg v1 and swap-accounting implementations and remove
the in-kernel Android lowmemorykiller. This is a configuration change, not a
PSI, pidfd, scheduler or 4.19 backport. Keep MEMCG_KMEM disabled: this old
implementation is explicitly not suitable for production slab accounting.

Use only with the matching GM8 device/vendor experimental branch. Android
12.1's existing LMKD can subscribe to memory.pressure_level through
cgroup.event_control and eventfd. The device requires /dev/memcg, explicitly
selects vmpressure + framework minfree thresholds, and keeps per-app memcg
disabled. Vendor initialization must bypass absent KLMK/ALMK nodes and leave
Qualcomm PPR disabled. Ordinary VM reclaim, swap, thermal protection and HMP
scheduling remain available.

Memcg adds page/swap metadata and accounting work. Lower displayed RAM usage
is not a success criterion. Compare app-switch stalls, relaunches, OOM/LMKD
events, launch times and then battery under matched workloads. The legacy
vmpressure strategy is not Android's newer PSI/thrashing strategy.

Host validation: actual Kconfig resolves MEMCG, MEMCG_SWAP,
MEMCG_SWAP_ENABLED, SWAP and EVENTFD to y, MEMCG_KMEM and the old LMK to n.
The complete mm/ built-in object plus kernel/cgroup.o, kernel/res_counter.o,
kernel/fork.o and fs/proc/base.o cross-compiled using AOSP GCC 4.9 from
android-10.0.0_r47, toolchain revision 84fb09fafc92a3d9b4d160f049d46c3c784cc941.
The host invocation selected the ELF gcc-4.9.x directly (no Python 2 wrapper)
and HOSTCFLAGS=-fcommon for the old host DTC. No source workaround was applied.

This is not a full linked kernel/Android build or a hardware test. Do not
install a boot image built from this config over the previous unpaired ROM.
Require a complete target-files build and the device's generated-config gate
before signing. Keep a working recovery/ROM backup. Rollback means restoring
the paired device/vendor/kernel baseline and rebuilding the complete package.
