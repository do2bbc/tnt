# Building TNT on Modern Linux Systems

## Overview
TNT (Hostmode Terminal for TNC) uses typed C function declarations and
definitions, including the legacy code translated from Pascal. It builds with
modern GCC without suppressing implicit declarations or old-style definitions.
Configure respects the compiler's default C dialect and explicit user flags.

## Compilation Instructions

### Prerequisites
- C compiler such as GCC
- GNU Autotools (autoconf, automake)
- ncurses development library
- GNU gettext
- dialog, for the interactive `tnt_setup` configuration scripts

### Build Steps

1. **Regenerate autotools files when building from Git:**
   ```bash
   autoreconf -fi
   ```

2. **Configure the project:**
   ```bash
   ./configure
   ```

   To build TNT without opening or polling a serial/soft-TNC device, configure with:
   ```bash
   ./configure --disable-serial
   ```

3. **Compile:**
   ```bash
   make
   ```

4. **Install (optional):**
   ```bash
   make install
   ```

### Raspberry Pi / ARM builds

On Raspberry Pi OS, install the standard build dependencies:

```bash
sudo apt install build-essential autoconf automake libncurses-dev gettext dialog
```

Then build normally:

```bash
autoreconf -fi
./configure
make
```

The GitHub Actions workflow builds on `amd64`, native `arm64` Ubuntu runners, and 32-bit `armhf` Debian under QEMU. That covers 64-bit Raspberry Pi OS on Raspberry Pi 3, 4 and 5, plus the 32-bit Raspberry Pi OS userland. For AX.25 kernel support on Raspberry Pi OS, install the distribution's AX.25 development package and configure with `--enable-ax25k2`.

For AX25UDP soft-TNC mode, configure TNT with `soft_tnc 3` and set `device` to `host:port` or `localport@host:port`, for example `10093@44.148.230.93:93`.

### Slackware 14.2 builds

On Slackware 14.2, make sure the development tools and gettext tools are installed before bootstrapping from Git. A full development install normally provides these; on minimal systems, install the `d` series tools plus `gettext-tools` and the runtime libraries needed by Slackware's `gcc`, `make`, and gettext packages.

Then use the same autotools flow:

```bash
autoreconf -fi
./configure
make
```

## Compiler Checks and Tests

### Terminal modernization beta

The `beta` branch identifies itself as `V1.9.3-beta1`. It uses the system
ncurses/libtinfo library and system gettext; the historical bundled gettext
sources are no longer compiled or used as a fallback. Install gettext tools
for catalog compilation, or configure with `--disable-nls`. Translation
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
This beta intentionally reverses the historical ON/OFF meaning, consistently
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

The terminal regression suite tests all 256 byte conversions, split UTF-8
input, unsupported/invalid sequences, legacy and UTF-8 I/O, terminal-profile
matching, intensity reset, indexed/RGB colors, reduced-palette fallback and
attribute-buffer limits under ASan/UBSan. It also exercises the production
keyboard dispatcher and connect/command editors through to their screen
buffers and submitted bytes: both UMLAUT settings, legacy/UTF-8 input,
F1-F12 and the Alt-M/B/L/C shortcuts.
Resize tests use a pseudo-terminal to verify size limits, redraws, retained
history, saved boxlist-window ownership and editing of hidden input tails.

Configure enables `-Wall`, `-Wextra`, `-Wstrict-prototypes` and
`-Wold-style-definition` when supported. No legacy-warning suppressions or
forced pre-C23 dialect are added.

To treat these warnings as errors during compilation:

```bash
make CFLAGS="$(sed -n 's/^CFLAGS = //p' Makefile) -Werror"
make check
```

CI additionally enables `_FORTIFY_SOURCE=3` on all three architectures.
To reproduce those checks locally, add `-D_FORTIFY_SOURCE=3` to the build's
`CFLAGS` above (after running `make clean` if objects already exist).
The QEMU-based `armhf` job sets `ASAN_OPTIONS=detect_leaks=0` because
LeakSanitizer cannot inspect processes under userspace emulation. AddressSanitizer
and UndefinedBehaviorSanitizer remain enabled; native jobs also check leaks.

Run configure normally before this strict build: some historical Autoconf
feature probes deliberately compile snippets that warn on current compilers.
The tests compile extracted production functions with AddressSanitizer and
UndefinedBehaviorSanitizer. They cover typed screen callbacks, character
conversion, byte output, compression round trips, AX25UDP endpoint parsing,
process-file path limits and the INP/NET-ROM monitor. I/O regressions also cover
interrupted and partial transfers, full output devices, premature EOF, cookie
header/footer lengths, bounded log lines, empty/short sysop password rejection
and help files without a final newline.
They run offline and do not open a TNC or connect to a node.

Verified configurations: GCC 12 on Debian 12/ARM64, GCC 13 on Ubuntu 24.04
(also a full 32-bit `gcc -m32` build), GCC 16 with its default
C23 dialect on x86_64 (also with `--enable-ax25k2`), and Clang 23/C23 with
`--disable-serial`. All were compiled with the enabled warnings as errors.

## Build Artifacts

After successful compilation, the following executables will be created:

- `src/tnt` - Main TNT terminal application
- `src/tntc` - TNT terminal client
- `src/runprgs/help` - Help utility
- `src/runprgs/info` - Information utility

## Troubleshooting

**Error: "too many arguments for function"**
- Make sure all source files are from the same revision, then run
  `autoreconf -fi` and `./configure` again.

**Error: "implicit declaration of function"**
- Make sure you have the required development libraries installed (ncurses, gettext)

**Error: "install-sh not found"**
- Run `autoreconf -i` to regenerate build scripts

## Clean Build

To perform a clean rebuild:
```bash
make distclean
autoreconf -fi
./configure
make
```
