#!/bin/sh
set -eu
source_dir=${top_srcdir:-$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)}
build_dir=${top_builddir:-$source_dir}
grep -Eq '^xterm\*[[:space:]]+1[[:space:]]+2[[:space:]]+1[[:space:]]+1' \
  "$source_dir/examples/termcap.tnt"
tmp=$(mktemp -d)
trap 'rm -f "$tmp/test.c" "$tmp/test" "$tmp/resize.c" "$tmp/resize"; rmdir "$tmp"' EXIT HUP INT TERM

cat > "$tmp/test.c" <<'EOF'
#include "tnt.h"
#include "terminal.h"
#include "xmon.h"
#include <assert.h>
#include <locale.h>

struct termcaplist {
  char term[83];
  int termcap, color, charconv, xtermkeys, supp_hicntl, auto_newline;
  struct termcaplist *next;
};
static struct termcaplist *termcaplist_root;
static int tnt_daemon;
static int size_blocked;
static char frontend_output[128];
static size_t frontend_used;
int charconv = 1, ibm_umlaut_flag = 1;
extern int umlaut;
extern void init_conv(void);
extern int conv_rx_to_local(char code, char *first, char *second);
extern int conv_local_to_tx(char code, char *first, char *second);
extern int conv_local_to_umlaut(char code, char *first, char *second);
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
static int termcap = 1;
static int COLS = 80, auto_newline, supp_hicntl, ins_char;
static struct win_info real_info[5];
static struct window input_window;
static char att_normal;
static char cells[4 * MAXCOLS];
static int act_mode = M_CONNECT, act_channel = 1, xmon_screen;
static int tnc_channels = 11, ax25k_active, queue_activ;
static int exit_flag, chan_flag, chan_val1, esc_flag, pass_flag;
static int wordwrap_flag, txecho_flag, selected;
static time_t sysopactiv;
static char func_str[12], input_str[MAXCOLS + 1];
static struct channel_stat channels[11], *ch_stat = channels;
static struct { char string[100]; } macro_strings[13];
#include "keys.h"
static void set_cursor(int win) { (void)win; }
static void redraw_editor(struct window *win) { (void)win; assert(0); }
static int last_phy_line(struct window *win, int line)
{ (void)win; (void)line; return 0; }
static void phy_charins(void) { assert(0); }
static void phy_cr(void) { assert(0); }
static void update_line(struct window *win, int line)
{ (void)win; (void)line; assert(0); }
static void phy_gotoxy(int x, int y) { (void)x; (void)y; assert(0); }
static void phy_curright(void) { assert(0); }
static void newline(struct window *win) { win->column = 0; }
static void crstatline(struct window *win) { win->column = 0; }
#define UNUSED_EDIT(name) \
  static void name(struct window *win) { (void)win; assert(0); }
UNUSED_EDIT(backspace)
UNUSED_EDIT(cur_left)
UNUSED_EDIT(cur_right)
UNUSED_EDIT(cur_up)
UNUSED_EDIT(cur_down)
UNUSED_EDIT(cur_up_page)
UNUSED_EDIT(cur_down_page)
UNUSED_EDIT(cur_top)
UNUSED_EDIT(cur_bot)
UNUSED_EDIT(cur_stline)
UNUSED_EDIT(cur_eoline)
UNUSED_EDIT(cur_eroline)
UNUSED_EDIT(cur_delchar)
static void statlin_update(void) {}
static void phy_no_transl(void) { assert(0); }
static void phy_normal_transl(void) { assert(0); }
EOF
for signature in 'static void win_charout_noctl(' 'void win_charout_cntl(' \
                 'static int get_line2('; do
  awk -v signature="$signature" '
    index($0, signature) == 1 && $0 !~ /;[ \t]*$/ { found = 1 }
    found { print }
    found && /^}/ { exit }
    END { if (!found) exit 1 }
  ' "$source_dir/src/window.c" >> "$tmp/test.c"
done
cat >> "$tmp/test.c" <<'EOF'
static void pre_charout(int channel, char ch)
{ assert(channel == 1); win_charout_cntl(ch, &input_window); }
static void pre_charout_nobnd(int channel, char ch) { pre_charout(channel, ch); }
static void pre_charout_cntl(int channel, char ch) { pre_charout(channel, ch); }
static void cmd_charout(int channel, char ch) { pre_charout(channel, ch); }
static void cmd_charout_nobnd(int channel, char ch) { pre_charout(channel, ch); }
static int pre_lineend(int channel) { (void)channel; return 0; }
static int pre_wordwrap(int channel, char *str)
{ (void)channel; (void)str; assert(0); return 0; }
static int pre_newline(int channel, char *str)
{ assert(channel == 1); return get_line2(str, &input_window); }
static int cmd_newline(int channel, char *str) { return pre_newline(channel, str); }
static char transmitted[MAXCOLS + 1];
static int transmitted_len;
static void dat_input(int channel, char *str, int len)
{
  assert(channel == 1 && len <= MAXCOLS + 1);
  memcpy(transmitted, str, len);
  transmitted_len = len;
}
static void cmd_input(int channel, int mode, char *str, int len, int flag)
{ (void)mode; (void)flag; dat_input(channel, str, len); }
static void cmd_display(int mode, int channel, char *str, int cr)
{ (void)mode; (void)channel; (void)str; (void)cr; assert(0); }
static void beep(void) { assert(0); }
static int data_allowed(int channel) { (void)channel; return 1; }
static void rem_sendonact(void) { assert(0); }
static void open_dwnfile(int *state) { (void)state; assert(0); }
static void rem_data_display(int channel, char *str)
{ (void)channel; (void)str; assert(0); }
static void rem_data_display_buf(int channel, char *str, int len)
{ (void)channel; (void)str; (void)len; assert(0); }
static void insert_cr_rx(int channel) { (void)channel; assert(0); }
static void real_window_up(int part, int page)
{ (void)part; (void)page; assert(0); }
static void real_window_down(int part, int page)
{ (void)part; (void)page; assert(0); }
static void real_window_top(int part) { (void)part; assert(0); }
static void real_window_bot(int part) { (void)part; assert(0); }
static void sel_connect(int channel) { selected = channel; }
static void sel_monitor(void) { selected = 20; }
static void sel_command(void) { selected = 21; }
static void sel_xmon(int screen) { (void)screen; selected = 22; }
static void sel_heardlist(void) { selected = 23; }
static void sel_boxlist(void) { selected = 24; }
static void sel_help(void) { selected = 25; }
#ifdef USE_IFACE
static void sel_mailbox(void) { selected = 26; }
#endif
EOF
# Use the production Escape dispatcher and connect/command editor branches.
awk '
  /^void keyboard_server\(/ { found = 1 }
  found && /^    case M_MONITOR:/ { print "#endif\n    }\n  }\n}"; exit }
  found { print }
  END { if (!found) exit 1 }
' "$source_dir/src/keyboard.c" >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
static void feed_keys(const char *text)
{
  int state = S_NORMAL;
  for (; *text; ++text) {
    char ch = *text;
    if (terminal_input(&ch))
      keyboard_server(&state, &ch);
  }
}

static void check_keyboard(void)
{
  const char latin1[] = "\xe4\xf6\xfc\xc4\xd6\xdc\xdf";
  const char utf8[] = "\xc3\xa4\xc3\xb6\xc3\xbc\xc3\x84\xc3\x96\xc3\x9c\xc3\x9f";
  const char *locales[] = {"C", "C.UTF-8"};
  const char *keys[] = {"\033OP", "\033OQ", "\033OR", "\033OS", "\033[15~",
                       "\033[17~", "\033[18~", "\033[19~", "\033[20~",
                       "\033[21~", "\033[23~", "\033[24~",
                       "\033m", "\033b", "\033l", "\033c"};
  const int targets[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 20, 21, 20, 26, 24, 21};
  memset(&input_window, 0, sizeof(input_window));
  input_window.real = -1;
  input_window.num_lines = 2;
  input_window.window_buffer = cells;
  for (size_t locale = 0; locale < 2; ++locale) {
    assert(setenv("LC_ALL", locales[locale], 1) == 0);
    terminal_encoding_init();
    for (umlaut = 0; umlaut <= 1; ++umlaut) {
      for (int command = 0; command <= 1; ++command) {
        const char *expected = umlaut ? "aeoeueAeOeUess" : latin1;
        size_t length = strlen(expected);
        input_window.column = selected = transmitted_len = 0;
        act_mode = command ? M_COMMAND : M_CONNECT;
        feed_keys(locale ? utf8 : latin1);
        assert(selected == 0 && input_window.column == (int)length);
        for (size_t i = 0; i < length; ++i)
          assert(cells[2 * i + 1] == expected[i]);
        feed_keys("\n");
        assert(transmitted_len == (int)length + 1);
        assert(memcmp(transmitted, expected, length) == 0);
        assert(transmitted[length] == CR);
      }
    }
    /* Latin-1 text must not be mistaken for synthetic Alt screen codes. */
    input_window.column = selected = 0;
    umlaut = 0;
    feed_keys(locale ? "\xc2\xa3\xc2\xa4\xc2\xa5" : "\xa3\xa4\xa5");
    assert(selected == 0 && input_window.column == 3);
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
#ifndef USE_IFACE
      if (targets[i] == 26) continue;
#endif
      act_mode = M_COMMAND;
      selected = 0;
      int column = input_window.column;
      feed_keys(keys[i]);
      assert(selected == targets[i] && input_window.column == column);
    }
    /* The former UTF-8 lead-byte workaround must not swallow Latin-1 A-tilde. */
    input_window.column = 0;
    feed_keys(locale ? "\xc3\x83" : "\xc3");
    assert(input_window.column == 1 && cells[1] == (char)0xc3);
  }
  /* Radio/file IBM mappings, including the Atari sharp-s alias. */
  const unsigned char radio[] = {0x84, 0x94, 0x81, 0x8e, 0x99, 0x9a, 0xe1, 0x9e};
  const char transliterated[] = "aeoeueAeOeUessss";
  for (size_t i = 0; i < sizeof(radio); ++i) {
    char first, second;
    umlaut = 0;
    assert(conv_rx_to_local(radio[i], &first, &second) == 1);
    assert(first == latin1[i < 7 ? i : 6]);
    assert(conv_local_to_tx(first, &first, &second) == 1);
    assert((unsigned char)first == radio[i < 7 ? i : 6]);
    umlaut = 1;
    assert(conv_rx_to_local(radio[i], &first, &second) == 2);
    assert(first == transliterated[2 * i] && second == transliterated[2 * i + 1]);
    assert(conv_local_to_tx(latin1[i < 7 ? i : 6], &first, &second) == 2);
    assert(first == transliterated[2 * i] && second == transliterated[2 * i + 1]);
    charconv = 0;
    umlaut = 0;
    assert(conv_local_to_umlaut(latin1[i < 7 ? i : 6], &first, &second) == 1);
    assert((unsigned char)first == radio[i < 7 ? i : 6]);
    charconv = 1;
  }
}

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
  init_conv();
  assert(umlaut == 0);
  check_profiles();
  check_unicode();
  check_keyboard();
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
  "$source_dir/src/terminal.c" "$source_dir/src/codconv.c" -o "$tmp/test"
"$tmp/test"

cat > "$tmp/resize.c" <<'EOF'
#define terminal_write capture_terminal_write
#include "window.c"
#undef terminal_write
#include <assert.h>
#include <pty.h>

char attc_normal, attc_statline, attc_monitor, attc_cstatline;
char attc_controlchar, attc_remote, attc_special, attc_monport1, attc_monport2;
char attm_normal, attm_statline, attm_monitor, attm_cstatline;
char attm_controlchar, attm_remote, attm_special, attm_monport1, attm_monport2;
int color = 2, color_save, termcap, termcap_save;
int wholeline_flag, bscrhold_flag, tabexp_flag, input_linelen = 200;
int ibm_umlaut_flag = 1;
char tnt_termcapfile[MAXCHAR];
static char rendered[262144];
static size_t rendered_size;

int capture_terminal_write(int fd, const char *buffer, size_t length)
{
  assert(fd == 1 && rendered_size + length < sizeof(rendered));
  memcpy(rendered + rendered_size, buffer, length);
  rendered_size += length;
  rendered[rendered_size] = '\0';
  return 0;
}
void statlin_update(void) {}

static void dimensions(int rows, int cols)
{
  struct winsize size = {0};
  size.ws_row = rows;
  size.ws_col = cols;
  assert(ioctl(0, TIOCSWINSZ, &size) == 0);
  rendered_size = 0;
  rendered[0] = '\0';
}

static void check_resize(void)
{
  struct window input = {0}, history = {0}, saved;
  struct real_layout screen[LAYOUTPARTS] = {0};
  char line[MAXCOLS + 1], expected[121];
  int master, slave, old_stdin;

  assert(openpty(&master, &slave, NULL, NULL, NULL) == 0);
  old_stdin = dup(0);
  assert(old_stdin >= 0 && dup2(slave, 0) == 0);
  assert(close(slave) == 0);
  COLS = 146;
  LINES = 56;
  att_normal = 0;
  cur_always_vis = 1;
  CL = "\033[H\033[2J";
  WR = "\r";
  BC = "\b";
  CE = "\033[K";
  ND = "\033[C";
  AL = "\033[L";
  DL = "\033[M";
  strcpy(CRNL, "\r\n");
  strcpy(RUBOUT, "\b \b");
  assert(open_window(4, &input, 1) == 0);
  assert(open_window(4, &history, 1) == 0);
  for (int i = 0; i < 120; ++i) {
    expected[i] = 'a' + i % 26;
    win_charout_cntl(expected[i], &input);
    win_charout(expected[i], &history, 0);
  }
  expected[120] = '\0';
  screen[0].win = &input;
  screen[0].win_num_lines = 2;
  screen[0].pagesize = 1;
  screen[1].win = &history;
  screen[1].first_real_line = 2;
  screen[1].win_num_lines = 2;
  screen[1].pagesize = 1;
  real_screen(screen);
  dimensions(24, 80);
  assert(resize_terminal() == 1 && COLS == 80 && LINES == 24);
  assert(input.real == -1 && history.real == -1);
  real_screen(screen);
  assert(input.view_column == 41 && input.column == 120);
  assert(history.view_column == 0);
  assert(get_line(line, &input) == 121);
  assert(memcmp(line, expected, 120) == 0 && line[120] == CR);
  assert(get_line(line, &history) == 121 && memcmp(line, expected, 120) == 0);
  win_charout_cntl(BS, &input);
  assert(input.column == 119 && input.view_column == 40);
  win_charout_cntl(expected[119], &input);
  assert(input.column == 120 && input.view_column == 41);
  win_charout_cntl(C_STLINE, &input);
  assert(input.column == 0 && input.view_column == 0);
  win_charout_cntl(C_EOLINE, &input);
  assert(input.column == 120 && input.view_column == 41);
  win_charout_cntl(C_CULEFT, &input);
  win_charout_cntl(C_DELCHAR, &input);
  assert(input.column == 119);
  win_charout_cntl(expected[119], &input);
  dimensions(60, 200);
  assert(resize_terminal() == 1);
  real_screen(screen);
  assert(input.view_column == 0 && input.column == 120);
  assert(get_line(line, &input) == 121 && memcmp(line, expected, 120) == 0);
  assert(get_line(line, &history) == 121 && memcmp(line, expected, 120) == 0);
  /* Ownership moves used by saved mailbox boxlists need no resize registry. */
  saved = history;
  memset(&history, 0, sizeof(history));
  screen[1].win = &saved;
  dimensions(16, 256);
  assert(resize_terminal() == 1 && COLS == MAXCOLS);
  real_screen(screen);
  assert(get_line(line, &saved) == 121 && memcmp(line, expected, 120) == 0);
  dimensions(15, 79);
  assert(resize_terminal() == 0 && terminal_size_blocked());
  assert(strstr(rendered, "minimum size 80 x 16") != NULL);
  size_t notice_size = rendered_size;
  win_charout('!', &saved, 0);
  assert(rendered_size == notice_size); /* incoming data cannot corrupt notice */
  assert(get_line(line, &saved) == 122 && line[120] == '!');
  dimensions(16, 256);
  assert(resize_terminal() == 1 && !terminal_size_blocked());
  real_screen(screen);
  dimensions(24, 300);
  assert(resize_terminal() == 1 && COLS == MAXCOLS && LINES == 24);
  assert(resize_terminal() == 0);
  /* Switching from a five-part to a two-part screen clears stale bindings. */
  real_info[4].win = &input;
  real_screen(screen);
  assert(real_info[4].win == NULL);
  dimensions(24, 80);
  assert(resize_terminal() == 1);
  real_screen(screen);
  int old_line = saved.phys_line;
  win_charout('Z', &saved, 0);
  assert(saved.column == 1 && get_line(line, &saved) == 2 && line[0] == 'Z');
  for (int i = 0; i < 120; ++i)
    assert(saved.window_buffer[old_line * 2 * MAXCOLS + 2 * i + 1] == expected[i]);
  assert(input.column == 120 && input.view_column == 41);
  old_line = input.phys_line;
  win_charout_cntl(CR, &input);
  assert(input.column == 0 && input.view_column == 0);
  for (int i = 0; i < 120; ++i)
    assert(input.window_buffer[old_line * 2 * MAXCOLS + 2 * i + 1] == expected[i]);
  clear_window(&input);
  real_screen(screen);
  for (int i = 0; i < 199; ++i)
    win_charout_cntl(i == 20 ? SPACE : 'b', &input);
  assert(input.column == 199 && input.view_column == 120);
  win_charout_cntl(C_CURIGHT, &input);
  assert(input.column == 199);
  assert(wordwrap(line, &input) == 21 && line[20] == CR);
  assert(input.column == 178 && input.view_column == 99);
  assert(get_line(line, &input) == 179 && line[178] == CR);
  for (int i = 0; i < 178; ++i) assert(line[i] == 'b');
  set_cursor_column(&input, 0);
  wholeline_flag = 1;
  assert(get_line(line, &input) == 179 && input.view_column == 99);
  wholeline_flag = 0;
  /* Scrolled ring rows survive resizing as well as newly allocated rows. */
  for (int i = 0; i < 6; ++i) {
    win_charout('R', &saved, 0);
    win_charout(CR, &saved, 0);
  }
  assert(saved.line_offset != 0);
  old_line = saved.line_offset;
  dimensions(56, 146);
  assert(resize_terminal() == 1);
  real_screen(screen);
  assert(saved.line_offset == old_line);
  assert(input.column == 178 && input.view_column == 33);
  assert(get_line(line, &input) == 179);
  assert(close_window(&input) == 0);
  assert(close_window(&saved) == 0);
  assert(dup2(old_stdin, 0) == 0);
  assert(close(old_stdin) == 0 && close(master) == 0);
}

EOF
cat >> "$tmp/resize.c" <<'EOF'
#include "xmon.h"
static char *ax_state[] = {"************"};
static char *ax_state_alt[] = {" discon  "};
static struct channel_stat channel_status[2];
static struct channel_stat *ch_stat = channel_status;
static struct window statwin[2], xmonstatwin[XMON_SCREENS];
static struct xmon_stat xm_stat[MAX_XMON];
static void strip_call_log(char *str, int channel)
{
  assert(channel == 1);
  strcpy(str, "DBW400");
}
EOF
for signature in 'static void append_char(' 'static void append_spaces(' \
                 'static void append_limited(' 'void stat_display_org(' \
                 'void stat_display_alt('; do
  awk -v signature="$signature" '
    index($0, signature) == 1 && $0 !~ /;[ \t]*$/ { found = 1 }
    found { print }
    found && /^}/ { exit }
    END { if (!found) exit 1 }
  ' "$source_dir/src/display.c" >> "$tmp/resize.c"
done
awk '
  /^static void xmonstat_display\(/ && $0 !~ /;[ \t]*$/ { found = 1 }
  found { print }
  found && /^}/ { exit }
  END { if (!found) exit 1 }
' "$source_dir/src/xmon.c" >> "$tmp/resize.c"
cat >> "$tmp/resize.c" <<'EOF'
static void check_status_widths(void)
{
  int widths[] = {80, 146, 200, MAXCOLS};
  char line[MAXCOLS + 2];
  assert(open_window(1, &statwin[0], 1) == 0);
  assert(open_window(1, &statwin[1], 1) == 0);
  assert(open_window(1, &xmonstatwin[0], 1) == 0);
  strcpy(ch_stat[1].call, "CHANNEL NOT CONNECTED");
  for (size_t w = 0; w < sizeof(widths) / sizeof(widths[0]); ++w) {
    COLS = widths[w];
    for (int channel = 0; channel < 2; ++channel) {
      stat_display_org(channel);
      assert(get_line_noconv(line, &statwin[channel]) == COLS + 1);
      assert(line[COLS - 1] == '*');
      stat_display_alt(channel);
      assert(get_line_noconv(line, &statwin[channel]) == COLS + 1);
    }
    memset(xm_stat, 0, sizeof(xm_stat));
    xmonstat_display(0);
    assert(get_line_noconv(line, &xmonstatwin[0]) == COLS + 1);
    assert(line[COLS - 1] == '*');
    for (int i = 0; i < MAX_XMON; ++i) {
      xm_stat[i].active = 1;
      strcpy(xm_stat[i].srccall, "DBW400-15");
      strcpy(xm_stat[i].destcall, "DNO484-15");
    }
    xmonstat_display(0);
    assert(get_line_noconv(line, &xmonstatwin[0]) == COLS + 1);
    assert(memcmp(line + 20, "DBW400-15->DNO484-15", 19) == 0);
  }
  assert(close_window(&statwin[0]) == 0);
  assert(close_window(&statwin[1]) == 0);
  assert(close_window(&xmonstatwin[0]) == 0);
}

int main(void)
{
  check_resize();
  check_status_widths();
  return 0;
}
EOF
${CC:-cc} -g -O1 -funsigned-char -ffunction-sections -fdata-sections \
  -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Werror \
  -fsanitize=address,undefined -Wl,--gc-sections \
  -I"$build_dir" -I"$source_dir/src" "$tmp/resize.c" \
  "$source_dir/src/terminal.c" "$source_dir/src/codconv.c" -ltinfo -lutil \
  -o "$tmp/resize"
"$tmp/resize"
