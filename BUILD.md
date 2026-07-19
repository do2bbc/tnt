# Building TNT on Modern Linux Systems

## Overview
TNT (Hostmode Terminal for TNC) is a legacy application originally written in Pascal and translated to C using the p2c translator. The codebase uses old-style K&R function definitions that require special compiler flags to work with modern versions of GCC.

## Compilation Instructions

### Prerequisites
- GCC compiler (tested with GCC 14.x)
- GNU Autotools (autoconf, automake)
- ncurses development library
- GNU gettext

### Build Steps

1. **Configure the project with appropriate compiler flags:**
   ```bash
   CFLAGS="-g -O2 -DTNT_LINUX -std=gnu99 -Wno-implicit-int -Wno-old-style-definition" ./configure
   ```

2. **Compile:**
   ```bash
   make
   ```

3. **Install (optional):**
   ```bash
   make install
   ```

## Compiler Flags Explanation

The following non-standard compiler flags are required due to the legacy nature of the codebase:

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

Modern GCC interprets empty parameter lists `()` in function declarations as accepting exactly 0 arguments, not variable arguments. The TNT codebase contains many function declarations and definitions using old-style parameters. The flags above allow:

1. Old-style K&R function definitions to be recognized
2. POSIX functions like `popen()` to be available without triggering errors
3. Legacy code patterns from the Pascal-to-C translation to compile successfully

## Troubleshooting

**Error: "too many arguments for function"**
- Ensure you're using the flags specified above, particularly `-std=gnu99`

**Error: "implicit declaration of function"**
- Make sure you have the required development libraries installed (ncurses, gettext)

**Error: "install-sh not found"**
- Run `autoreconf -i` to regenerate build scripts

## Clean Build

To perform a clean rebuild:
```bash
make distclean
CFLAGS="-g -O2 -DTNT_LINUX -std=gnu99 -Wno-implicit-int -Wno-old-style-definition" ./configure
make
```
