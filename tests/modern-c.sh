#!/bin/sh
set -eu
source_dir=${top_srcdir:-$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)}
build_dir=${top_builddir:-$source_dir}
tmp=$(mktemp -d)
trap 'rm -f "$tmp/test.c" "$tmp/test"; rmdir "$tmp"' EXIT HUP INT TERM

extract()
{
  awk -v signature="$1" '
    index($0, signature) == 1 && $0 !~ /;[ \t]*$/ { found = 1 }
    found { print }
    found && /^}/ { exit }
    END { if (!found) exit 1 }
  ' "$source_dir/src/$2" >> "$tmp/test.c"
}

cat > "$tmp/test.c" <<'EOF'
#include "tnt.h"
#include <assert.h>
static char captured[16];
static int used;
static int tnt_daemon, charconv = 1, tabexp_flag;
static char proc_file[80] = "tnntest.pid";
static int act_channel = 7, xmon_screen = 2, selected;
int enchufmem(char *input, long size, char **output, long *output_size,
              char *filename, int convert);
int dechufmem(char *input, long size, char **output, long *output_size,
              char *filename, int convert);
static void capture(char ch)
{
  assert(used < (int)sizeof(captured));
  captured[used++] = ch;
}
static void write_frontend(char *buffer, int len)
{
  assert(len == 1);
  capture(*buffer);
}
static ssize_t capture_write(int fd, const void *buffer, size_t len)
{
  assert(fd == 1 && len == 1);
  capture(*(const char *)buffer);
  return 0;
}
#define tnt_write_all capture_write
#define terminal_write capture_write
EOF
extract 'static int phy_charout(' window.c
cat >> "$tmp/test.c" <<'EOF'
#undef tnt_write_all
#undef terminal_write
static void win_charout_noctl(char ch, struct window *win, int insert)
{
  (void)insert;
  capture(ch);
  ++win->column;
}
static void newline(struct window *win) { win->column = 0; }
static void crstatline(struct window *win) { win->column = 0; }
static int conv_rx_to_local(char ch, char *first, char *second)
{
  assert(ch == 0xe4);
  *first = 'a';
  *second = 'e';
  return 2;
}
static void sel_connect(int channel) { assert(channel == 7); selected = 1; }
static void sel_command(void) { selected = 2; }
static void sel_monitor(void) { selected = 3; }
static void sel_heardlist(void) { selected = 4; }
static void sel_xmon(int screen) { assert(screen == 2); selected = 5; }
static void sel_help(void) { selected = 6; }
static void sel_boxlist(void) { selected = 7; }
#ifdef USE_IFACE
static void sel_mailbox(void) { selected = 8; }
#endif
static void cmd_display(int mode, int channel, char *text, int cr)
{
  (void)mode; (void)channel; (void)cr;
  assert(strcmp(text, OK_TEXT) == 0);
}
EOF
extract 'void win_charout_2(' window.c
extract 'static int split_udp_endpoint(' serial.c
extract 'static int build_fallback_proc_file(' init.c
extract 'static void select_current_connect(' keyboard.c
extract 'static void select_current_xmon(' keyboard.c
awk '
  /^struct screen_list \{/ { found = 1 }
  found { print }
  found && /^};/ { if (++ends == 2) exit }
  END { if (ends != 2) exit 1 }
' "$source_dir/src/keyboard.c" >> "$tmp/test.c"
extract 'static void cmd_selscreen(' keyboard.c
cat >> "$tmp/test.c" <<'EOF'
#include "keys.h"
static int esc_flag, chan_flag, analysed;
static char func_str[11];
static void statlin_update(void) {}
static void dispatch_control(char *ch)
{
EOF
awk '
  /^  if \(\(\*ch < SPACE\)/ { found = 1 }
  found { print }
  found && /^    if \(analysed\) return;/ {
    print "  }"
    exit
  }
  END { if (!found) exit 1 }
' "$source_dir/src/keyboard.c" >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
}
static void check_mailbox_shortcuts(void)
{
#ifdef USE_IFACE
  const char *shortcuts[] = {"\033b", "\033B"};
  for (size_t key = 0; key < sizeof(shortcuts) / sizeof(shortcuts[0]); ++key) {
    int found = 0;
    for (size_t i = 0; i < sizeof(special_keys) / sizeof(special_keys[0]); ++i) {
      if (strcmp(special_keys[i].code, shortcuts[key]) == 0) {
        char ch = special_keys[i].res_code;
        selected = 0;
        assert(special_keys[i].sel_channel == 0 && ch == C_MAILBOX);
        dispatch_control(&ch);
        assert(analysed == 1 && selected == 8);
        found = 1;
        break;
      }
    }
    assert(found);
  }
#else
  char ch = C_HELP;
  assert(sizeof(special_keys) > 0);
  dispatch_control(&ch);
  assert(selected == 6);
#endif
}
EOF
cat >> "$tmp/test.c" <<'EOF'
int main(void)
{
  struct window win = {0};
  int (*put_character)(int) = phy_charout;
  char endpoint[80], filename[80], longdir[100];
  int port = -1;
  char text[4096], *compressed = NULL, *decoded = NULL;
  long compressed_size, decoded_size;
  check_mailbox_shortcuts();
  win.num_lines = 2;
  put_character(0xe4);
  tnt_daemon = 1;
  put_character(0xf6);
  assert(used == 2 && captured[0] == 0xe4 && captured[1] == 0xf6);
  used = 0;
  win_charout_2(0xe4, &win, 1);
  assert(used == 2 && memcmp(captured, "ae", 2) == 0);
  used = 0;
  win_charout_2(0xe4, &win, 0);
  assert(used == 1 && captured[0] == 0xe4);
  for (int i = 0; screen_list[i].screen[0]; ++i) {
    char *screen = screen_list[i].screen;
    cmd_selscreen(0, 0, 0, strlen(screen), 0, screen);
    assert(selected == i + 1);
  }
  assert(split_udp_endpoint("host:93", endpoint, sizeof(endpoint), &port) == 0);
  assert(port == 0 && strcmp(endpoint, "host:93") == 0);
  assert(split_udp_endpoint("10093@host:93", endpoint, sizeof(endpoint), &port) == 0);
  assert(port == 10093 && strcmp(endpoint, "host:93") == 0);
  assert(split_udp_endpoint("65536@host:93", endpoint, sizeof(endpoint), &port) == 1);
  assert(split_udp_endpoint("10093@host:93", endpoint, 4, &port) == 1);
  for (size_t i = 0; i < sizeof(text); ++i) text[i] = i % 256;
  assert(enchufmem(text, sizeof(text), &compressed, &compressed_size, "/dev/null", 0) == 0);
  assert(compressed != NULL && compressed_size > 0);
  assert(dechufmem(compressed, compressed_size, &decoded, &decoded_size, "/dev/null", 0) == 0);
  assert(decoded != NULL && decoded_size == sizeof(text));
  assert(memcmp(text, decoded, sizeof(text)) == 0);
  free(decoded);
  free(compressed);
  assert(setenv("XDG_RUNTIME_DIR", "/tmp", 1) == 0);
  assert(build_fallback_proc_file(filename, sizeof(filename)) == 0);
  assert(strncmp(filename, "/tmp/tnt-", 9) == 0);
  assert(strstr(filename, "-tnntest.pid") != NULL);
  memset(longdir, 'x', sizeof(longdir) - 1);
  longdir[sizeof(longdir) - 1] = '\0';
  assert(setenv("XDG_RUNTIME_DIR", longdir, 1) == 0);
  assert(build_fallback_proc_file(filename, sizeof(filename)) == 1);
  puts("Modern C callbacks, characters, compression, UDP endpoints and paths: PASS");
  return 0;
}
EOF
${CC:-cc} -g -O1 -funsigned-char -Wall -Wextra -Wstrict-prototypes \
  -Wold-style-definition -Werror -fsanitize=address,undefined \
  -I"$build_dir" -I"$source_dir/src" "$tmp/test.c" \
  "$source_dir/src/huffman.c" -o "$tmp/test"
"$tmp/test"
