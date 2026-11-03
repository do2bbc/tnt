#!/bin/sh
set -eu
source_dir=${top_srcdir:-$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)}
build_dir=${top_builddir:-$source_dir}
grep -Eq '^xterm\*[[:space:]]+1[[:space:]]+2[[:space:]]+1[[:space:]]+1' \
  "$source_dir/examples/termcap.tnt"
tmp=$(mktemp -d)
trap 'rm -f "$tmp/test.c" "$tmp/test"; rmdir "$tmp"' EXIT HUP INT TERM

cat > "$tmp/test.c" <<'EOF'
#include "tnt.h"
#include "terminal.h"
#include <assert.h>
#include <locale.h>

struct termcaplist {
  char term[83];
  int termcap, color, charconv, xtermkeys, supp_hicntl, auto_newline;
  struct termcaplist *next;
};
static struct termcaplist *termcaplist_root;
static int tnt_daemon;
static char frontend_output[128];
static size_t frontend_used;
static void write_frontend(char *buffer, int length)
{
  assert(length >= 0 && frontend_used + length <= sizeof(frontend_output));
  memcpy(frontend_output + frontend_used, buffer, length);
  frontend_used += length;
}
EOF
awk '
  /^static int find_termcap\(/ { found = 1 }
  found { print }
  found && /^}/ { exit }
  END { if (!found) exit 1 }
' "$source_dir/src/window.c" >> "$tmp/test.c"
for signature in 'static void phy_strout(' 'static int phy_charout('; do
  awk -v signature="$signature" '
    index($0, signature) == 1 { found = 1 }
    found { print }
    found && /^}/ { exit }
    END { if (!found) exit 1 }
  ' "$source_dir/src/window.c" >> "$tmp/test.c"
done
cat >> "$tmp/test.c" <<'EOF'
static void check_profiles(void)
{
  struct termcaplist xterm = {"xterm*", 1, 2, 1, 1, 0, 0, NULL};
  struct termcaplist fallback = {"*", 1, 0, 1, 0, 0, 0, NULL};
  int tc, color, conv, keys, suppress, newline;
  xterm.next = &fallback;
  termcaplist_root = &xterm;
  assert(find_termcap("xterm-256color", &tc, &color, &conv, &keys,
                     &suppress, &newline) == 1);
  assert(tc == 1 && color == 2 && conv == 1 && keys == 1);
  assert(find_termcap("vt100", &tc, &color, &conv, &keys,
                     &suppress, &newline) == 1);
  assert(color == 0 && keys == 0);
}

static void check_unicode(void)
{
  unsigned int state = 0;
  unsigned int byte;
  char encoded[2], decoded = 0;
  for (byte = 0; byte <= 255; ++byte) {
    size_t size = terminal_utf8_output(byte, encoded);
    state = 0;
    if (size == 2)
      assert(terminal_utf8_input((unsigned char)encoded[0], &state, &decoded) == 0);
    assert(terminal_utf8_input((unsigned char)encoded[size - 1], &state, &decoded) == 1);
    assert((unsigned char)decoded == byte && state == 0);
  }
  assert(terminal_utf8_input(0xe2, &state, &decoded) == 0);
  assert(terminal_utf8_input(0x82, &state, &decoded) == 0);
  assert(terminal_utf8_input(0xac, &state, &decoded) == -1); /* euro */
  assert(state == 0);
  assert(terminal_utf8_input(0xf0, &state, &decoded) == 0);
  assert(terminal_utf8_input(0x9f, &state, &decoded) == 0);
  assert(terminal_utf8_input(0x98, &state, &decoded) == 0);
  assert(terminal_utf8_input(0x80, &state, &decoded) == -1); /* emoji */
  assert(terminal_utf8_input(0xc0, &state, &decoded) == -1); /* overlong */
  assert(terminal_utf8_input(0x80, &state, &decoded) == -1); /* stray continuation */
  assert(terminal_utf8_input(0xc3, &state, &decoded) == 0);
  assert(terminal_utf8_input(27, &state, &decoded) == 1);
  assert(decoded == 27 && state == 0);
}

static void check_boundary(const char *locale, int utf8)
{
  int fd[2];
  char output[1024], input[512], ch;
  size_t used = 0;
  unsigned int i;
  ssize_t received;
  assert(setenv("LC_ALL", locale, 1) == 0);
  terminal_encoding_init();
  assert(strcmp(setlocale(LC_CTYPE, NULL), "C") == 0);
  for (i = 0; i < sizeof(input); ++i)
    input[i] = i % 256;
  assert(pipe(fd) == 0);
  assert(terminal_write(fd[1], input, sizeof(input)) == 0);
  assert(close(fd[1]) == 0);
  while ((received = read(fd[0], output + used, sizeof(output) - used)) > 0)
    used += received;
  assert(received == 0 && close(fd[0]) == 0);
  if (utf8) {
    char expected[2];
    size_t offset = 0;
    for (i = 0; i < sizeof(input); ++i) {
      size_t size = terminal_utf8_output((unsigned char)input[i], expected);
      assert(memcmp(output + offset, expected, size) == 0);
      offset += size;
    }
    assert(offset == used);
    ch = 0xc3;
    assert(terminal_input(&ch) == 0);
    ch = 0xa4;
    assert(terminal_input(&ch) == 1 && (unsigned char)ch == 0xe4);
  }
  else {
    assert(used == sizeof(input) && memcmp(input, output, used) == 0);
    ch = 0xe4;
    assert(terminal_input(&ch) == 1 && (unsigned char)ch == 0xe4);
  }
}

static void check_colors(void)
{
  char output[96], small[2];
  assert(terminal_color_sequence(output, sizeof(output), 0, 0x07, 8, 0) > 0);
  assert(strcmp(output, "\033[0;22;25;37;40m") == 0);
  assert(terminal_color_sequence(output, sizeof(output), 0, 0x8f, 8, 0) > 0);
  assert(strcmp(output, "\033[0;1;5;37;40m") == 0);
  assert(terminal_palette_option("normal", "196,17") == 0);
  assert(terminal_color_sequence(output, sizeof(output), 0, 7, 256, 0) > 0);
  assert(strstr(output, "38;5;196;48;5;17m") != NULL);
  assert(terminal_color_sequence(output, sizeof(output), 0, 7, 8, 0) > 0);
  assert(strstr(output, ";31;44m") != NULL);
  assert(terminal_palette_option("monitor", "#12ab34,#010203") == 0);
  assert(terminal_color_sequence(output, sizeof(output), 2, 7, 256, 1) > 0);
  assert(strstr(output, "38;2;18;171;52;48;2;1;2;3m") != NULL);
  assert(terminal_color_sequence(output, sizeof(output), 2, 7, 256, 0) > 0);
  assert(strstr(output, ";2;18;171;52") == NULL);
  assert(terminal_palette_option("normal", "256,0") == 1);
  assert(terminal_palette_option("normal", "-1,0") == 1);
  assert(terminal_palette_option("normal", "1,0garbage") == 1);
  assert(terminal_palette_option("normal", "#ff,0") == 1);
  assert(terminal_palette_option("unknown", "1,0") == 1);
  assert(terminal_color_sequence(small, sizeof(small), 0, 7, 256, 0) == -1);
  assert(terminal_color_sequence(output, sizeof(output), 0, 7, 0, 0) == -1);
}

static void check_render_paths(void)
{
  int fd[2], saved_stdout;
  char output[128];
  const char expected[] = "\xc3\xa4\033[0;22;25;37;40m\xc3\xb6";
  const char legacy[] = "\xe4\033[0;22;25;37;40m\xf6";
  assert(setenv("LC_ALL", "C.UTF-8", 1) == 0);
  terminal_encoding_init();
  assert(pipe(fd) == 0);
  saved_stdout = dup(1);
  assert(saved_stdout >= 0 && dup2(fd[1], 1) == 1);
  assert(close(fd[1]) == 0);
  tnt_daemon = 0;
  assert(phy_charout(0xe4) == 0);
  phy_strout("\033[0;22;25;37;40m\xf6");
  assert(dup2(saved_stdout, 1) == 1 && close(saved_stdout) == 0);
  assert(read(fd[0], output, sizeof(output)) == (ssize_t)sizeof(expected) - 1);
  assert(memcmp(output, expected, sizeof(expected) - 1) == 0);
  assert(close(fd[0]) == 0);
  tnt_daemon = 1;
  assert(phy_charout(0xe4) == 0);
  phy_strout("\033[0;22;25;37;40m\xf6");
  assert(frontend_used == sizeof(legacy) - 1);
  assert(memcmp(frontend_output, legacy, sizeof(legacy) - 1) == 0);
}

int main(void)
{
  check_profiles();
  check_unicode();
  check_boundary("C", 0);
  /* C.UTF-8 is available on the supported Linux CI platforms and Raspberry Pi. */
  assert(setlocale(LC_CTYPE, "C.UTF-8") != NULL);
  check_boundary("C.UTF-8", 1);
  check_colors();
  check_render_paths();
  return 0;
}
EOF
${CC:-cc} -g -O1 -funsigned-char -Wall -Wextra -Wstrict-prototypes \
  -Wold-style-definition -Werror -fsanitize=address,undefined \
  -I"$build_dir" -I"$source_dir/src" "$tmp/test.c" \
  "$source_dir/src/terminal.c" -o "$tmp/test"
"$tmp/test"
