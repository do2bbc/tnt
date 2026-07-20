# Building TNT on Modern Linux Systems

## Overview
TNT (Hostmode Terminal for TNC) is a legacy application originally written in Pascal and translated to C using the p2c translator. The codebase uses old-style K&R function definitions. The configure script now probes for compiler options that keep those legacy declarations compatible with modern C compilers.

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

## Compiler Flags Explanation

The configure script probes for these non-standard compiler flags when the compiler supports them:

- **`-std=gnu99`**: Uses GNU C99 standard instead of strict C99. This allows POSIX extensions and old-style function definitions to compile without errors.

- **`-Wno-implicit-int`**: Suppresses warnings about implicit integer types in old-style function parameter declarations.

- **`-Wno-old-style-definition`**: Allows K&R-style function definitions where parameters are declared after the parameter list, a common pattern in code translated from Pascal.

## Build Artifacts

After successful compilation, the following executables will be created:

- `src/tnt` - Main TNT terminal application
- `src/tntc` - TNT terminal client
- `src/runprgs/help` - Help utility
- `src/runprgs/info` - Information utility

## Why These Flags Are Needed

Modern C23-era compilers can interpret empty parameter lists `()` in function declarations as accepting exactly 0 arguments, not variable arguments. The TNT codebase contains many function declarations and definitions using old-style parameters. The configure-time checks allow:

1. Old-style K&R function definitions to be recognized
2. POSIX functions like `popen()` to be available without triggering errors
3. Legacy code patterns from the Pascal-to-C translation to compile successfully

## Troubleshooting

**Error: "too many arguments for function"**
- Run `autoreconf -fi` and `./configure` again so the compatibility checks refresh generated Makefiles.

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
