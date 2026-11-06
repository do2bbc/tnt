# Building TNT on Modern Linux Systems

## Overview
TNT (Hostmode Terminal for TNC) uses typed C function declarations and
definitions, including the legacy code translated from Pascal. It builds with
modern GCC without suppressing implicit declarations or old-style definitions.
CMake respects the compiler's default C dialect and explicit user flags.

## Compilation Instructions

### Ready-to-install release packages

Linux release candidate 2.0.0-rc3 provides Debian 12 `.deb` packages for
amd64, arm64 and ARMv7 armhf, plus an openSUSE Leap 16 x86_64 RPM.
See [package installation and upgrade notes](packaging/README.packages).
No package starts a service or overwrites an active `tnt.ini`.
Verify downloads with the release's `SHA256SUMS`.
These packages do not update a running TNT process or a private binary.
The RPM is not advertised as compatible with other RPM distributions.

### Reloading password assignments

After editing the file configured as `tnt_pwfile`, enter `LOADPRIV` in TNT's
command window. Connected channels immediately use the updated assignments;
no reconnect or restart is required. Pending password challenges are reset,
so request authentication again with `PRIV`. Removed entries become unavailable.
If the replacement file cannot be read or is malformed, TNT reports an error
and retains the previous assignments. An empty valid file clears all assignments.
These reload fixes are included in 2.0.0-rc2, but not the original RC1 packages.
The same applies to the immediate lower channel overview refresh after received
`connected to` / `reconnected to` messages; no additional received line is needed.

### Other runtime updates (included in RC2)

- The extended channel status line is enabled by default (`altstat 1`).
  It shows a database name before the remote callsign when available.
  Existing configurations keep their explicit setting; use `altstat 0` for
  the classic display. Changing this setting requires restarting TNT.
- `NAME <name>` refreshes cached names and extended status displays on connected
  channels, including channels that use an SSID fallback from the names database.
  Editing the names file externally is not a new automatic reload feature.
- Hostmode callsign replies and link-state changes refresh the lower overview
  immediately; counter-only polls do not redraw it unnecessarily. A rejected
  reconnect command also restores cached names and both status displays.
- `LOADSYS` and `LDBLCMD` replace their in-memory tables only after a successful
  read. Missing, unreadable or malformed replacements report an error and retain
  the last valid table. Valid empty files deliberately clear their tables.
- Successful `LOADSYS` resets pending sysop challenges. Request a new challenge
  using the remote `SYSOP` command. Existing authenticated access levels are
  unchanged; reloading is not a command for revoking already granted access.

### Prerequisites
- C compiler such as GCC
- CMake 3.20 or newer
- Ninja (recommended) or Make
- ncurses development library
- GNU gettext
- Python 3 for the full native regression suite (`BUILD_TESTING=ON`)
- dialog, for the interactive `tnt_setup` configuration scripts

### Build Steps

1. **Configure in a separate build directory:**
   ```bash
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   ```

2. **Compile and test:**
   ```bash
   cmake --build build --parallel 2
   ctest --test-dir build --output-on-failure
   ```

3. **Install (optional):**
   ```bash
   cmake --install build
   ```

Omit `-G Ninja` to use the default Make generator. Autoconf, Automake and
`autoreconf` are no longer used. In-source CMake builds are rejected.
Executables, generated examples, setup scripts and `config.h` live in `build/`.
`build/compile_commands.json` supports VS Code and clang-based tooling.

### Options and installation compatibility

| Former configure option | CMake equivalent |
|---|---|
| `--disable-serial` | `-DTNT_SERIAL=OFF` |
| `--disable-sound` | `-DTNT_SOUND=OFF` |
| `--disable-nls` | `-DTNT_NLS=OFF` |
| `--enable-ax25k` | `-DTNT_AX25K=ON` |
| `--enable-ax25k2` | `-DTNT_AX25K2=ON` |
| `--enable-hibaud` | `-DTNT_HIBAUD=ON` |
| `--enable-genuser` | `-DTNT_GENUSER=ON` |
| `--enable-flat` | `-DTNT_FLAT=ON` |
| `--with-language=de` | `-DTNT_LANGUAGE=de` |
| `--prefix=PATH` | `-DCMAKE_INSTALL_PREFIX=PATH` |
| `--sysconfdir=PATH` | `-DCMAKE_INSTALL_SYSCONFDIR=PATH` |
| `--localstatedir=PATH` | `-DTNT_LOCALSTATEDIR=PATH` |

Serial, sound and translations default to ON; optional kernel AX.25, HIBAUD,
user generation and flat installation default to OFF. The language defaults to
English. The already-broken historical DPBOXT option now fails at configuration
with an explicit explanation; it is not repaired by this migration.
Other legacy-only limitations also remain: HIBAUD currently misses the
`waitpid` declaration on modern Linux compilers, and user generation reports
a signedness warning under `-Werror`. Neither is enabled by default.

Release builds retain the previous `-O2` optimization level. Set
`CMAKE_C_FLAGS`, `CMAKE_C_FLAGS_RELEASE` or `CMAKE_C_COMPILER` explicitly when
needed. Use `-DBUILD_TESTING=OFF` to omit test registration.
`-DTNT_EDITOR=/path/to/editor` selects the external editor. If no editor is
available during the build, CMake warns and retains the historical runtime
default `ed`; the editor must be available when that command is used.

Platform settings are retained for Linux, NetBSD/OpenBSD and Solaris; this
migration is validated on Linux. The obsolete Interactive UNIX (ISC) build
configuration is not carried forward.

The existing installation layout is retained: configuration examples have
`.ex` suffixes, helper programs live under `PREFIX/libexec/tnt`, and runtime
state defaults to `/var`, even with a custom prefix. For a private installation
matching the Pi configuration:

```bash
cmake -S . -B build-pi -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local" \
  -DTNT_LOCALSTATEDIR="$HOME/.local/var" \
  -DTNT_SOUND=OFF -DTNT_LANGUAGE=de
```

Installation does not start TNT, replace an active `tnt.ini`, or delete history.
Use `DESTDIR="$PWD/stage" cmake --install build` to inspect a staged install.
Set the prefix at configuration time: generated configuration and helper paths
are compiled in, so `cmake --install --prefix` is not a relocation mechanism.
The installation test uses its own staging directory inside the build tree.
`cmake --build build --target uninstall` removes installed files using the
build's install manifest; use the same `DESTDIR` for a staged uninstall.
Active configuration and runtime history are not installed files and remain.

### Raspberry Pi / ARM builds

On Raspberry Pi OS, install the standard build dependencies:

```bash
sudo apt install build-essential cmake ninja-build libncurses-dev gettext dialog python3
```

Then build normally:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

The GitHub Actions workflow builds on `amd64`, native `arm64` Ubuntu runners, and 32-bit `armhf` Debian under QEMU. That covers 64-bit Raspberry Pi OS on Raspberry Pi 3, 4 and 5, plus the 32-bit Raspberry Pi OS userland. For AX.25 kernel support on Raspberry Pi OS, install the distribution's AX.25 development package and configure with `-DTNT_AX25K2=ON`.

UDP soft-TNC mode (`soft_tnc 3`) transports hostmode commands, not raw AXUDP
frames. Its `device` syntax is `host:port` or `localport@host:port`, and the peer
must understand that hostmode transport. For a raw AXUDP node, configure the
UDP route in a TNN backend and connect TNT to its hostmode socket instead.

### Two isolated hostmode backends in one terminal

With serial support enabled, two Unix-domain TNC/TNN hostmode sockets can be
combined without forwarding packet frames between the backends:

```
soft_tnc 1
device /path/to/first/tnn-socket
tnc_channels 10
backend2_device /path/to/second/tnn-socket
backend2_channels 10
backend1_name CB
backend2_name HAM
```

Channels 1-10 belong to the first backend; channels 11-20 map to channels 1-10
of the second. Set the correct own callsign and network routes independently
in each backend before starting TNT. A second backend is a hostmode endpoint,
not a raw AXUDP destination. Separate TNN instances are needed when one is a
CB build and the other is an amateur-radio build. Never share their routing
tables or working directories.

The upper channel status line identifies the backend. `BACKEND 1` or
`BACKEND 2` selects the shared monitor and unproto target; `MONITOR` commands
follow that selection. Monitor headers are tagged with their source backend.
Connected-channel commands and text always follow their channel assignment,
regardless of the monitor selection. `BACKEND` without arguments shows both
channel ranges. Configure monitor reception on each backend as needed; the
shared monitor polls only the selected backend, not both simultaneously.

Hostmode transactions are serialized and use ordinary channel polling instead
of the single-backend extended poll. Buffer accounting and duplicate-connection
checks remain backend-specific. Legacy automatic channel allocation for
mailbox/interfaces/sockets stays on the first backend. There is no automatic
CB/HAM bridge or automatic connection to the configured remote node.
If either active backend disconnects or times out, TNT reports the failure and
closes rather than rerouting queued traffic to the other network; the backend
services remain running. Restart TNT after restoring both sockets.

The second backend is opt-in. Existing single-backend configurations and
extended polling retain their previous behavior. Regression tests also require
Python 3 and exercise a real TNT process against two isolated mock backends.

### Slackware 14.2 builds

On Slackware 14.2, install CMake 3.20 or newer in addition to the compiler,
ncurses and gettext development tools. The original distribution's older CMake
may need upgrading. Make remains a supported backend; Ninja is optional.

Then use the same CMake flow:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
```

## Compiler Checks and Tests

### Terminal modernization / 2.0.0-rc3

The release candidate identifies itself as `V2.0.0-rc3`. It uses the system
ncurses/libtinfo library and system gettext; the historical bundled gettext
sources are no longer compiled or used as a fallback. Install gettext tools
for catalog compilation, or configure with `-DTNT_NLS=OFF`. Translation
catalogs are maintained as UTF-8 and checked for printf-format mismatches.

Terminal profiles recognize `xterm*`, `rxvt*`, `screen*` and `tmux*`, including
their 256-color variants. Color output is disabled when the terminal database
reports fewer than eight colors. Existing `attc_*` settings retain their
meaning. Optional `palette_<role> foreground,background` settings accept
indexes 0-255 or `#RRGGBB`; see `examples/tnt.ini.in` for role names.
RGB is enabled for local terminals advertising `COLORTERM=truecolor` or
`COLORTERM=24bit`. Otherwise RGB and out-of-range palette indexes are mapped
to the nearest supported palette color. Daemon clients conservatively use
the palette supported by their advertised `TERM`, not the server's
`COLORTERM`. Actual colors still depend on the terminal's configured palette.

On UTF-8 terminals, TNT converts its existing Latin-1 screen bytes to UTF-8
at the terminal boundary and decodes representable keyboard input back to
legacy bytes. This also applies to the `tntc` frontend. Radio traffic, files,
screen-buffer layout and protocol encodings are **not** changed to UTF-8.
Characters outside Latin-1 (for example emoji, combining marks and CJK) are
rejected with a bell instead of being sent as corrupted radio text. This is
UTF-8 terminal compatibility, not a Unicode editor or UTF-8 radio protocol.
Legacy locales, including the Pi's existing `de_DE` locale, keep byte-for-byte
terminal I/O. Use a valid UTF-8 locale and the matching terminal encoding to
enable conversion; changing the locale alone does not change LXTerminal's
encoding preference. No system-wide locale change is required.
`UMLAUT ON` enables transliteration to `ae`, `oe`, `ue`, `Ae`, `Oe`,
`Ue` and `ss`; `UMLAUT OFF` preserves umlauts (the startup default).
This release candidate intentionally reverses the historical ON/OFF meaning, consistently
for keyboard input, received text and log-file conversion. Update startup
scripts that explicitly set `UMLAUT` to retain their intended behavior.
The legacy IBM/Latin-1
conversion tables remain separate from terminal UTF-8 decoding. Ordinary
high-bit text characters do not trigger Alt screen shortcuts.

When upgrading an existing installation, merge the new terminal-profile
patterns into the user's active `termcap.tnt`; installing example files does
not override private profiles.

Local TNT terminals follow `SIGWINCH` size changes without reconnecting.
Existing output and boxlist rows are clipped, not reflowed; their hidden
right-hand contents return when the terminal is widened. Unsent input remains
intact and scrolls horizontally when its cursor is beyond the visible width.
Buffers use a fixed 256-column stride, independent of the visible width.
Below 80 columns or 16 rows, TNT shows a size notice and ignores local
keyboard commands until the terminal is enlarged; network processing continues.
Widths above 256 columns use the leftmost 256 columns. The desktop launcher's
geometry remains the initial size only. The `tntc` frontend still uses its
connection-time dimensions; reconnect it after changing its terminal size.

The channel status separator uses `attc_cstatline` / `attm_cstatline` and the
optional `palette_cstatline`. To match the channel overview, give these the
same values as `attc_statline` / `attm_statline` / `palette_statline`.

Channel scrollback is journaled as lines are completed, including submitted
input, and restored after a TNT process crash. The command and monitor windows
are never persisted, so each command window starts with only the current
version banner. Journal writes go immediately to Linux; changed data is
`fsync`ed approximately once per second, so power loss can still lose recent
records.
Snapshots and the bounded journal live in `tnt_work_dir` as `scrollback.dat`
and `scrollback.journal`, with owner-only permissions. Existing snapshot-only
files are migrated automatically. Normal/reduced buffer defaults are
10,000/2,000 rows; explicit `lines_*` settings in existing configurations still
take precedence. See the manual for checkpointing and recovery details.

In local xterm-compatible terminals, left-click a channel entry in the lower
overview to switch channels. Connected entries displaying a callsign are
clickable too. Hit targets follow the displayed channel group and status-row
position after resizing or changing screens; clicks elsewhere, releases,
other buttons and modified clicks do not switch channels. Unsent input stays
with its channel. Hold Shift to use the terminal's own text selection, paste
or scrolling instead of TNT's mouse reporting. Mouse tracking is disabled on
exit and is not enabled for `tntc` clients.

With `LAYER3 ON`, the monitor decodes INP routing information frames (RIF):
callsign, advertised hops, transport time in milliseconds (wire values are
10-ms ticks), alias and IPv4 prefix options. Zero metrics and the TNN routing
horizon of 60000 or higher are shown as withdrawn routes. Unknown options
remain visible by tag and length; malformed/truncated entries are reported.
TNN `L3RTT:` probes to the `L3RTT` pseudo-destination show sender uptime,
reported SRTT/RTT, peer identification, INP capability (`$N`) and the advertised
maximum transport time (`$M`). These are reported values, not a local
measurement of the round trip. Classic NET/ROM NODES broadcasts and ordinary
Layer-4 traffic retain their existing interpretation.

The protocol interpretation was checked against the Layer-3/4 description
in TheNetNode 1.78 manual, appendix F, and the TNN 1.79 INP implementation
(`src/l3inp.c`, `src/l3rtt.c`, `include/l3local.h` in the TNN source tree).
The older appendix describes the general framing and RTT mechanism, not the
complete newer INP option format.

The terminal regression suite tests all 256 byte conversions, split UTF-8
input, unsupported/invalid sequences, legacy and UTF-8 I/O, terminal-profile
matching, intensity reset, indexed/RGB colors, reduced-palette fallback and
attribute-buffer limits under ASan/UBSan. It also exercises the production
keyboard dispatcher and connect/command editors through to their screen
buffers and submitted bytes: both UMLAUT settings, legacy/UTF-8 input,
F1-F12 and the Alt-M/B/L/C shortcuts.
Mouse reports are exercised through the keyboard dispatcher in legacy and
UTF-8 locales, and click targets are checked against rendered channel rows.
Resize tests use a pseudo-terminal to verify size limits, redraws, retained
history, saved boxlist-window ownership and editing of hidden input tails.
Status separators are tested through 256 columns, including densely populated
extended-monitor status lines clipped to the current width.

CMake enables `-Wall`, `-Wextra`, `-Wstrict-prototypes` and
`-Wold-style-definition` when supported. No legacy-warning suppressions or
forced pre-C23 dialect are added.

To treat these warnings as errors during compilation:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTNT_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

CI additionally enables `_FORTIFY_SOURCE=3` on all three architectures.
To reproduce those checks locally, add `-DCMAKE_C_FLAGS=-D_FORTIFY_SOURCE=3`
when configuring the build.
The QEMU-based `armhf` job sets `ASAN_OPTIONS=detect_leaks=0` because
LeakSanitizer cannot inspect processes under userspace emulation. AddressSanitizer
and UndefinedBehaviorSanitizer remain enabled; native jobs also check leaks.

The tests compile extracted production functions with AddressSanitizer and
UndefinedBehaviorSanitizer. They cover typed screen callbacks, character
conversion, byte output, compression round trips, AX25UDP endpoint parsing,
process-file path limits and the INP/NET-ROM monitor. I/O regressions also cover
interrupted and partial transfers, full output devices, premature EOF, cookie
header/footer lengths, bounded log lines, empty/short sysop password rejection
and help files without a final newline.
They run offline and do not open a TNC or connect to a node.
CTest also checks a staged installation, including repeated installation without
overwriting active configuration or scrollback data. The standard and AX.25
builds are checked with warnings as errors. Disabling gettext exposes additional
pre-existing format-size warnings in legacy runtime code; the minimal CI build
keeps warnings enabled but does not promote them to errors.

Verified configurations: GCC 12 on Debian 12/ARM64, GCC 13 on Ubuntu 24.04
(also a full 32-bit `gcc -m32` build), GCC 16 with its default
C23 dialect on x86_64 (also with AX.25 kernel support), and Clang 23/C23 with
serial disabled. All were compiled with the enabled warnings as errors.

## Build Artifacts

After successful compilation, the following executables will be created:

- `build/src/tnt` - Main TNT terminal application
- `build/src/tntc` - TNT terminal client
- `build/src/runprgs/tnt-help` - Help utility (installed as `help`)
- `build/src/runprgs/logcall` - Log utility
- `build/src/runprgs/tnt-info` - Information utility (`info` symlink)

The help build artifact uses `tnt-help` to avoid Ninja's reserved `help` target;
its installed name and runtime behavior are unchanged.

## Troubleshooting

**Error: "too many arguments for function"**
- Make sure all source files are from the same revision and configure a fresh
  build directory.

**Error: "implicit declaration of function"**
- Make sure you have the required development libraries installed (ncurses, gettext)

**Error: missing CMake or a required library**
- Install CMake 3.20 or newer, ncurses development headers and gettext tools.
- Use `-DTNT_NLS=OFF` only if translations are intentionally disabled.

## Clean Build

To perform a clean rebuild:
```bash
cmake -S . -B build-clean -DCMAKE_BUILD_TYPE=Release
cmake --build build-clean --parallel 2
ctest --test-dir build-clean --output-on-failure
```

`cmake --build build --target package_source` creates a source tarball using
CPack. Debian and RPM build scripts use CMake too; cross-builds set the CMake
target system and compiler and do not run native shell tests on target binaries.
