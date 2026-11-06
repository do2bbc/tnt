#!/bin/sh
set -eu
source_dir=${top_srcdir:-$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)}
build_dir=${top_builddir:-$source_dir}
tmp=$(mktemp -d)
trap 'rm -f "$tmp/test" "$tmp/socket"; rmdir "$tmp"' EXIT HUP INT TERM
${CC:-cc} -g -O1 -funsigned-char \
  -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Werror \
  -fsanitize=address,undefined -I"$build_dir" -I"$source_dir/src" \
  "$source_dir/tests/backends.c" "$source_dir/src/backend.c" -o "$tmp/test"
"$tmp/test" "$tmp/socket"
