#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "GEDE01",  # 0
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-2"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.3"
config.objdiff_tag = "v3.6.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.3"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Compiler flags: one profile per toolchain that produced code in the DOL.
# Each profile lists every flag once; no profile overrides another's flags.

# Shared by every profile.
cflags_common = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-use_lmw_stmw on",
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_common.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_common.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_common.append("-W all")
elif args.warn == "off":
    cflags_common.append("-W off")
elif args.warn == "error":
    cflags_common.append("-W error")

includes_game = [
    "-i include",
    f"-i build/{config.version}/include",
    "-i src/msl",
    "-i src/sk",
]

config.linker_version = "GC/1.3.2"

# Silicon Knights' build of the game
mw_version_game = config.linker_version

# MetroTRK v0.10, the same build as Animal Crossing's and Mario Party 4's
cflags_trk = [
    *cflags_common,
    "-sdata 0",
    "-sdata2 0",
    "-inline auto,deferred",
    "-rostr",
    "-char signed",
    "-i src",
    "-i src/msl",
    "-i libs/dolphin/include",
]

cflags_game = [
    *cflags_common,
    "-fp_contract on",
    "-str reuse",
    *includes_game,
]

cflags_lua = [
    *cflags_game,
    "-inline off",
    "-i src/lua",
]

# SK's modified copy of the SDK's DTK, built as game code with peephole and scheduling off
cflags_dtk = [
    *cflags_game,
    "-opt nopeephole",
    "-schedule off",
    "-i libs/dolphin/include",
    "-i libs/dolphin/include/libc",
]

# REL flags
cflags_rel = [
    *cflags_game,
    "-sdata 0",
    "-sdata2 0",
]

# Metrowerks runtime library: GC/1.3 (__fill_mem needs it) with signed char (__va_arg)
mw_version_runtime = "GC/1.3"
cflags_runtime = [
    *cflags_common,
    "-fp_contract on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
    "-char signed",
    *includes_game,
    "-i libs/dolphin/include",
]

# Metrowerks C library: Mario Party 4's MSL build (GC/1.3, deferred inlining, so
# each file's functions are emitted last to first)
cflags_msl = [
    *cflags_common,
    "-fp_contract on",
    "-str reuse,pool,readonly",
    "-common off",
    "-inline auto,deferred",
    "-char signed",
    *includes_game,
]

# Nintendo's Dolphin SDK build (libs/dolphin/Makefile, release)
mw_version_sdk = "GC/1.2.5n"
cflags_sdk = [
    *cflags_common,
    "-fp_contract off",  # SDK float code has no fused multiply-adds
    "-str reuse",
    "-inline auto",
    "-i libs/dolphin/include",
    "-i libs/dolphin/include/libc",
    "-ir libs/dolphin/src",
]


# Helper function for Dolphin libraries.
# char: MWCC's default (signed), as every reference decomp builds the SDK; OSUartExi.c proves it.
def DolphinLib(lib_name: str, objects: List[Object], char: str = "signed", mw_version: str = mw_version_sdk) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": mw_version,
        "cflags": [*cflags_sdk, f"-char {char}"],
        "src_dir": "libs/dolphin/src",
        "progress_category": "sdk",
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": mw_version_game,
        "cflags": cflags_rel,
        "progress_category": "game",
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


def DolphinLibObject(matching: bool, path: str, char: Optional[str] = None) -> Object:
    options: Dict[str, Any] = {"source": path.removeprefix("dolphin/")}
    if char is not None:
        options["cflags"] = [*cflags_sdk, f"-char {char}"]
    return Object(matching, path, **options)


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = True
config.libs = [
    DolphinLib("ai", [
        DolphinLibObject(Matching, "dolphin/ai/ai.c"),
    ]),
    DolphinLib("amcExi2", [
        DolphinLibObject(Matching, "dolphin/amcExi2/AmcExi.c"),
        DolphinLibObject(Matching, "dolphin/amcExi2/AmcExi2Comm.c"),
    ], mw_version="GC/1.2.5"),  # 1.2.5n schedules mtlr last in epilogues; Pikmin uses 1.2.5 too
    DolphinLib("amcnotstub", [
        DolphinLibObject(Matching, "dolphin/amcnotstub/amcnotstub.c"),
    ]),
    DolphinLib("ar", [
        DolphinLibObject(Matching, "dolphin/ar/ar.c"),
        DolphinLibObject(Matching, "dolphin/ar/arq.c"),
    ]),
    DolphinLib("base", [
        DolphinLibObject(Matching, "dolphin/base/PPCArch.c"),
    ]),
    DolphinLib("card", [
        DolphinLibObject(Matching, "dolphin/card/CARDBios.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDUnlock.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDRdwr.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDBlock.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDDir.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDCheck.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDMount.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDFormat.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDOpen.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDCreate.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDRead.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDWrite.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDDelete.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDStat.c"),
        DolphinLibObject(Matching, "dolphin/card/CARDNet.c"),
    ]),
    DolphinLib("db", [
        DolphinLibObject(Matching, "dolphin/db/db.c"),
    ]),
    DolphinLib("dsp", [
        DolphinLibObject(Matching, "dolphin/dsp/dsp.c"),
        DolphinLibObject(Matching, "dolphin/dsp/dsp_debug.c"),
        DolphinLibObject(Matching, "dolphin/dsp/dsp_task.c"),
    ]),
    DolphinLib("dvd", [
        DolphinLibObject(Matching, "dolphin/dvd/dvd.c"),
        DolphinLibObject(Matching, "dolphin/dvd/dvderror.c"),
        DolphinLibObject(Matching, "dolphin/dvd/dvdfs.c"),
        DolphinLibObject(Matching, "dolphin/dvd/dvdlow.c"),
        DolphinLibObject(Matching, "dolphin/dvd/dvdqueue.c"),
        DolphinLibObject(Matching, "dolphin/dvd/fstload.c"),
    ]),
    DolphinLib("fileCache", [
        DolphinLibObject(Matching, "dolphin/fileCache/fileCache.c"),
    ]),
    DolphinLib("gx", [
        DolphinLibObject(Matching, "dolphin/gx/GXInit.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXFifo.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXAttr.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXMisc.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXGeometry.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXFrameBuf.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXLight.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXTexture.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXBump.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXTev.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXPixel.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXDraw.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXStubs.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXDisplayList.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXTransform.c"),
        DolphinLibObject(Matching, "dolphin/gx/GXPerf.c"),
    ]),
    DolphinLib("mtx", [
        DolphinLibObject(Matching, "dolphin/mtx/mtx.c"),
        DolphinLibObject(Matching, "dolphin/mtx/mtxvec.c"),
        DolphinLibObject(Matching, "dolphin/mtx/mtx44.c"),
        DolphinLibObject(Matching, "dolphin/mtx/vec.c"),
        DolphinLibObject(Matching, "dolphin/mtx/quat.c"),
        DolphinLibObject(Matching, "dolphin/mtx/psmtx.c"),
    ], mw_version="GC/1.2.5"),
    DolphinLib("odemustubs", [
        DolphinLibObject(Matching, "dolphin/odemustubs/odemustubs.c"),
    ]),
    DolphinLib("os", [
        DolphinLibObject(Matching, "dolphin/os/OS.c"),
        DolphinLibObject(Matching, "dolphin/os/OSAlarm.c"),
        DolphinLibObject(Matching, "dolphin/os/OSAlloc.c"),
        DolphinLibObject(Matching, "dolphin/os/OSArena.c"),
        DolphinLibObject(Matching, "dolphin/os/OSAudioSystem.c"),
        DolphinLibObject(Matching, "dolphin/os/OSCache.c"),
        DolphinLibObject(Matching, "dolphin/os/OSContext.c"),
        DolphinLibObject(Matching, "dolphin/os/OSError.c"),
        DolphinLibObject(Matching, "dolphin/os/OSExi.c"),
        DolphinLibObject(Matching, "dolphin/os/OSFont.c"),
        DolphinLibObject(Matching, "dolphin/os/OSInterrupt.c"),
        DolphinLibObject(Matching, "dolphin/os/OSLink.c"),
        DolphinLibObject(Matching, "dolphin/os/OSMemory.c"),
        DolphinLibObject(Matching, "dolphin/os/OSMessage.c"),
        DolphinLibObject(Matching, "dolphin/os/OSMutex.c"),
        DolphinLibObject(Matching, "dolphin/os/OSReboot.c"),
        DolphinLibObject(Matching, "dolphin/os/OSRtc.c"),
        DolphinLibObject(Matching, "dolphin/os/OSReset.c"),
        DolphinLibObject(Matching, "dolphin/os/OSResetSW.c"),
        DolphinLibObject(Matching, "dolphin/os/OSSerial.c"),
        DolphinLibObject(Matching, "dolphin/os/OSStopwatch.c"),
        DolphinLibObject(Matching, "dolphin/os/OSSync.c"),
        DolphinLibObject(Matching, "dolphin/os/SISamplingRate.c"),
        DolphinLibObject(Matching, "dolphin/os/OSUartExi.c"),
        DolphinLibObject(Matching, "dolphin/os/OSThread.c"),
        DolphinLibObject(Matching, "dolphin/os/OSTime.c"),
        DolphinLibObject(Matching, "dolphin/os/__ppc_eabi_init.c"),
        DolphinLibObject(Matching, "dolphin/os/__start.c"),
    ]),
    DolphinLib("pad", [
        DolphinLibObject(Matching, "dolphin/pad/Pad.c"),
        DolphinLibObject(Matching, "dolphin/pad/Padclamp.c"),
    ]),
    DolphinLib("perf", [
        DolphinLibObject(Matching, "dolphin/perf/perf.c"),
    ]),
    DolphinLib("support", [
        DolphinLibObject(Matching, "dolphin/support/List.c"),
        DolphinLibObject(Matching, "dolphin/support/string.c"),
    ]),
    DolphinLib("texPalette", [
        DolphinLibObject(Matching, "dolphin/texPalette/texPalette.c"),
    ]),
    DolphinLib("vi", [
        DolphinLibObject(Matching, "dolphin/vi/vi.c"),
    ]),
    {
        "lib": "TRK_MINNOW_DOLPHIN",
        "mw_version": "GC/1.3",
        "cflags": cflags_trk,
        "progress_category": "sdk",  # str | List[str]
        "objects": [
            Object(Matching, "TRK_MINNOW_DOLPHIN/mainloop.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/nubevent.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/nubinit.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/msg.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/msgbuf.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/serpoll.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/usr_put.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/dispatch.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/msghndlr.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/support.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mutex_TRK.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/notify.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/flush_cache.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mem_TRK.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/targimpl.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/targsupp.s"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/__exception.s"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/dolphin_trk.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mpc_7xx_603e.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/main_TRK.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/dolphin_trk_glue.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/targcont.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/target_options.c"),
            Object(Matching, "TRK_MINNOW_DOLPHIN/mslsupp.c"),
        ],
    },
    {
        "lib": "Runtime",
        "mw_version": mw_version_runtime,
        "cflags": cflags_runtime,
        "progress_category": "sdk",  # str | List[str]
        "objects": [
            Object(Matching, "Runtime/__mem.c"),
            Object(Matching, "Runtime/__va_arg.c"),
            Object(Matching, "Runtime/global_destructor_chain.c"),
            Object(Matching, "Runtime/runtime.c"),
            Object(Matching, "Runtime/__init_cpp_exceptions.cpp"),
            Object(Matching, "Runtime/Gecko_setjmp.c"),
            Object(Matching, "Runtime/Gecko_ExceptionPPC.c"),
            Object(Matching, "Runtime/GCN_Mem_Alloc.c"),
        ],
    },
    {
        "lib": "MSL_C.PPCEABI.bare.H",
        "mw_version": mw_version_runtime,
        "cflags": cflags_msl,
        "progress_category": "sdk",
        "objects": [
            Object(Matching, "msl/abort_exit.c"),
            Object(Matching, "msl/alloc.c"),
            Object(Matching, "msl/errno.c"),
            Object(Matching, "msl/ansi_files.c"),
            Object(Matching, "msl/ansi_fp.c"),
            Object(Matching, "msl/assert.c"),
            Object(Matching, "msl/buffer_io.c"),
            Object(Matching, "msl/ctype.c"),
            Object(Matching, "msl/direct_io.c"),
            Object(Matching, "msl/file_io.c"),
            Object(Matching, "msl/FILE_POS.c"),
            Object(Matching, "msl/locale.c"),
            Object(Matching, "msl/mbstring.c"),
            Object(Matching, "msl/mem.c"),
            Object(Matching, "msl/mem_funcs.c"),
            Object(Matching, "msl/misc_io.c"),
            Object(Matching, "msl/printf.c"),
            Object(Matching, "msl/qsort.c"),
            Object(Matching, "msl/rand.c"),
            Object(Matching, "msl/float.c"),
            Object(Matching, "msl/scanf.c"),
            Object(Matching, "msl/signal.c"),
            Object(Matching, "msl/string.c"),
            Object(Matching, "msl/strtold.c"),
            Object(Matching, "msl/strtoul.c"),
            Object(Matching, "msl/uart_console_io.c"),
            Object(Matching, "msl/wchar_io.c"),
            Object(Matching, "msl/e_acos.c"),
            Object(Matching, "msl/e_atan2.c"),
            Object(Matching, "msl/e_exp.c"),
            Object(Matching, "msl/e_log.c"),
            Object(Matching, "msl/e_pow.c"),
            Object(Matching, "msl/e_rem_pio2.c"),
            Object(Matching, "msl/k_cos.c"),
            Object(Matching, "msl/k_rem_pio2.c"),
            Object(Matching, "msl/k_sin.c"),
            Object(Matching, "msl/k_tan.c"),
            Object(Matching, "msl/s_atan.c"),
            Object(Matching, "msl/s_ceil.c"),
            Object(Matching, "msl/s_copysign.c"),
            Object(Matching, "msl/s_cos.c"),
            Object(Matching, "msl/s_floor.c"),
            Object(Matching, "msl/s_frexp.c"),
            Object(Matching, "msl/s_ldexp.c"),
            Object(Matching, "msl/s_modf.c"),
            Object(Matching, "msl/s_nextafter.c"),
            Object(Matching, "msl/s_rint.c"),
            Object(Matching, "msl/s_sin.c"),
            Object(Matching, "msl/s_tan.c"),
            Object(Matching, "msl/w_acos.c"),
            Object(Matching, "msl/w_atan2.c"),
            Object(Matching, "msl/w_exp.c"),
            Object(Matching, "msl/w_log.c"),
            Object(Matching, "msl/w_pow.c"),
            Object(Matching, "msl/math_ppc.c"),
        ],
    },
    {
        # Hudson's HVQM4 video decoder (prebuilt library, linked after MSL)
        "lib": "hvqm4dec",
        "mw_version": "GC/1.2.5",
        "cflags": [*cflags_sdk, "-char signed", "-i include"],
        "progress_category": "sdk",
        "objects": [
            Object(NonMatching, "hvqm4dec/hvqm4dec.c"),
        ],
    },
    {
        # DVD audio streaming: Nintendo's DTK as modified by SK (DTKTrack.offset, two
        # getters) and linked among SK's objects at 0x801B1028
        "lib": "dtk",
        "mw_version": mw_version_game,
        "cflags": cflags_dtk,
        "progress_category": "sdk",
        "objects": [
            Object(Matching, "dtk/dtk.c"),
        ],
    },
    {
        # Embedded Lua 4.0 core (4.0, not 4.0.1: see ldo.c protectedparser, lparser.c
        # retstat, lvm.c traceexec; lstring.c carries the 4.0.1 udata fix). Linked
        # alphabetically lcode..lzio, then lapi.c and lauxlib.c after lzio.c.
        # Note the allocator is *not* stock: every Lua TU passes its own
        # __FILE__/__LINE__ into the Silicon Knights allocator at 0x8016B5CC, so
        # the luaM_* macros in lmem.h were rewritten. Expect local modifications.
        "lib": "lua",
        "mw_version": mw_version_game,
        "cflags": cflags_lua,
        "progress_category": "lua",
        "objects": [
            # Confirmed present via __FILE__ strings, in link order.
            # Addresses are *interior anchors*, not split boundaries.
            Object(Matching, "lua/ldo.c"),       # anchor 0x801604F8
            Object(Matching, "lua/lfunc.c"),     # anchor 0x801610E8
            Object(Matching, "lua/lgc.c"),       # anchor 0x80161B58
            Object(Matching, "lua/lapi.c"),       # anchor ?
            Object(Matching, "lua/lauxlib.c"),   # luaL_openlib only; follows lapi.c
            Object(Matching, "lua/lcode.c"),       # anchor ?
            Object(Matching, "lua/ldebug.c"),       # anchor ?
            Object(Matching, "lua/llex.c"),      # anchor ?
            Object(Matching, "lua/lmem.c"),      # anchor 0x8016393C
            Object(Matching, "lua/lobject.c"),   # anchor 0x80163A9C
            Object(Matching, "lua/lparser.c"),   # anchor 0x80164A64
            Object(Matching, "lua/lstate.c"),       # anchor 0x80166894
            Object(Matching, "lua/lstring.c"),   # anchor 0x80166AC4
            Object(Matching, "lua/ltable.c"),    # anchor 0x80167558
            Object(Matching, "lua/ltm.c"),    # anchor ?
            Object(Matching, "lua/lundump.c"),   # anchor 0x80168114
            Object(Matching, "lua/lvm.c"),    # anchor ?
            Object(Matching, "lua/lzio.c"),    # anchor ?
            # No stdlib (lbaselib, lstrlib, ...) is linked. SK replacements live
            # outside this lib: luaM_realloc (sk/SKtest.c, 0x8016B5CC), strtod (0x8016BA4C).
        ],
    },
    {
        "lib": "sk",
        "mw_version": mw_version_game,
        "cflags": cflags_game,
        "progress_category": "sk",
        "objects": [
            Object(NonMatching, "sk/MemCard.c"),
            Object(NonMatching, "sk/particle.c"),
            Object(NonMatching, "sk/scheduler.c"),
            Object(NonMatching, "sk/sd_sound.c"),
            Object(NonMatching, "sk/sd_debug.c"),
            Object(NonMatching, "sk/stream.c"),
            Object(NonMatching, "sk/ut_BitOperations.c"),
            Object(NonMatching, "sk/ut_register.c"),
            Object(NonMatching, "sk/ut_Menu.c"),
            Object(NonMatching, "sk/SKEngine_Render.c"),
            Object(NonMatching, "sk/SkEngine_AnimTexture.c"),
            Object(NonMatching, "sk/cineeffects.c"),
            Object(NonMatching, "sk/ai_message.c"),
            Object(NonMatching, "sk/ai_gameobject.c"),
            Object(NonMatching, "sk/ai_utils.c"),
            Object(NonMatching, "sk/ai_inventory.c"),
            Object(NonMatching, "sk/dvdfs.c"),
        ]
    },
    {
        "lib": "ed",
        "mw_version": mw_version_game,
        "cflags": cflags_game,
        "progress_category": "game",
        "objects": [
            Object(NonMatching, "ed/SkEngine_FileRead.c"),  # SK-named, but links in block A
            Object(NonMatching, "ed/ED_Reset.c"),
            Object(NonMatching, "ed/ED_AI.c"),
            Object(NonMatching, "ed/ED_Import.c"),
            Object(NonMatching, "ed/ED_PathFinding_AStar.c"),
            Object(NonMatching, "ed/ed_ai_Projectile.c"),
        ],
    },
    {
        # Block B (0x801082D4..0x8015EA7C), between HVQM4 and Lua: the middle layer
        "lib": "data",
        "mw_version": mw_version_game,
        "cflags": cflags_game,
        "progress_category": "game",
        "objects": [
            Object(NonMatching, "data/hvqm4read.c"),
            Object(NonMatching, "data/ch_char.c"),
            Object(NonMatching, "data/ch_seq_mgr.c"),
            Object(NonMatching, "data/EffectControl.c"),
            Object(NonMatching, "data/gos_database.c"),
            Object(NonMatching, "data/Inventory.c"),
            Object(NonMatching, "data/ld_loaddata.c"),
        ],
    },
    {
        # Block D (0x8022E7FC..0x80237EE0), after the SDK: Xiph libogg (bitwise) + libvorbis, SK-modified,
        # ending with SK's threaded Vorbis streamer (~0x802364B8). References: notes/dolphin-decomps/libogg
        "lib": "vorbis",
        "mw_version": mw_version_game,
        "cflags": cflags_game,
        "progress_category": "sk",
        "objects": [
            Object(NonMatching, "vorbis/info.c"),
        ],
    }
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("sdk", "SDK Code"),
    ProgressCategory("lua", "Lua"),
    ProgressCategory("sk", "SK"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
