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

Configure enables `-Wall`, `-Wextra`, `-Wstrict-prototypes` and
`-Wold-style-definition` when supported. No legacy-warning suppressions or
forced pre-C23 dialect are added.

To treat these warnings as errors during compilation:

```bash
make CFLAGS="$(sed -n 's/^CFLAGS = //p' Makefile) -Werror"
make check
```

Run configure normally before this strict build: some historical Autoconf
feature probes deliberately compile snippets that warn on current compilers.
The tests compile extracted production functions with AddressSanitizer and
UndefinedBehaviorSanitizer. They cover typed screen callbacks, character
conversion, byte output, compression round trips, AX25UDP endpoint parsing,
process-file path limits and the INP/NET-ROM monitor.
They run offline and do not open a TNC or connect to a node.

Verified configurations: GCC 12 on Debian 12/ARM64, GCC 16 with its default
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
