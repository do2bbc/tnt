#!/bin/sh
set -eu
source_dir=${top_srcdir:-$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)}
build_dir=${top_builddir:-$source_dir}
grep -q '^#define DEF_ALTSTAT 1$' "$source_dir/src/init.h"
grep -q '^altstat 1$' "$source_dir/examples/tnt.ini.in"
tmp=$(mktemp -d)
trap 'rm -f "$tmp/test.c" "$tmp/test" "$tmp/sys" "$tmp/blcmd"; rmdir "$tmp"' EXIT HUP INT TERM

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
#include <assert.h>
#include "tnt.h"
#include "macro.h"
static struct channel_stat channels[5];
static struct channel_stat *ch_stat = channels;
int tnc_channels = 5, altstat = 1;
static int status_updates[5], overview_updates, fail_add;
static char overview_call[257];
static char output[2048], stored_name[90] = "Old name";
static const char *calls[] = {"", "DNO284", "DNO284-1", "DNO484", "DNO284"};
static char rem_name_err[] = "Name write error";
static char rem_name_str1[] = "Hello ", rem_name_str2[] = "!";
static char tnt_sysfile[MAXCHAR], tnt_blcmdfile[MAXCHAR];

static void cmd_display(int mode, int channel, char *text, int cr)
{
  (void)mode; (void)channel; (void)cr;
  assert(strlen(output) + strlen(text) + 2 < sizeof(output));
  strcat(output, text);
  strcat(output, "\n");
}

static void strip_call_log(char *call, int channel)
{
  strcpy(call, ch_stat[channel].call[0] ? ch_stat[channel].call : calls[channel]);
}

static int macro_getname(char *call, char *name)
{
  strcpy(name, strncmp(call, "DNO284", 6) == 0 ? stored_name : call);
  return TNT_OK;
}

static int find_line_for_call(char *call, char *name)
{
  (void)call; (void)name;
  return LINE_NOTFOUND;
}

static int delete_line_for_call(char *call)
{
  (void)call;
  assert(0);
  return 1;
}

static int add_line_for_call(char *call, char *name)
{
  assert(strcmp(call, "DNO284") == 0);
  if (fail_add) return 1;
  strcpy(stored_name, name);
  return 0;
}

static void stat_display(int channel)
{
  ++status_updates[channel];
}

void statlin_update(void)
{
  strcpy(overview_call, ch_stat[1].call);
  ++overview_updates;
}
static int allocation_budget = -1;
static void *test_malloc(size_t size)
{
  if (allocation_budget == 0) return NULL;
  if (allocation_budget > 0) --allocation_budget;
  return malloc(size);
}
EOF
extract 'void update_name(' serial.c
extract 'void rem_name(' remote.c
awk '
  /^struct sysoplist \{/ { found = 1 }
  found { print }
  found && /^};/ { exit }
  END { if (!found) exit 1 }
' "$source_dir/src/remote.c" >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
static struct sysoplist *sysoplist_root;
#define malloc test_malloc
EOF
extract 'static int sysline_analyse(' remote.c
# Accept both the original and the corrected return type for reproduction.
awk '
  /^static (void|int) load_sysfile\(/ { found = 1 }
  found { print }
  found && /^}/ { exit }
  END { if (!found) exit 1 }
' "$source_dir/src/remote.c" >> "$tmp/test.c"
extract 'static void clear_sysfile(' remote.c
extract 'void cmd_loadsys(' remote.c
awk '
  /^struct blcmdstr \{/ { found = 1 }
  found { print }
  /^static struct blcmdlist \*blcmdlist_root;/ { exit }
  END { if (!found) exit 1 }
' "$source_dir/src/boxlist.c" >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
#define BOXLINE_ALL -1
#define BOXLINE_UNKNOWN 0
#define BOXLINE_CHECK 1
#define BOXLINE_RUNC 2
#define BOXLINE_LIST 3
#define BOXLINE_DIR 4
EOF
awk '
  /^static (void|int) load_blcmdfile\(/ { found = 1 }
  found { print }
  found && /^}/ { exit }
  END { if (!found) exit 1 }
' "$source_dir/src/boxlist.c" >> "$tmp/test.c"
extract 'static void clear_blcmdfile(' boxlist.c
extract 'void cmd_ldblcmd(' boxlist.c
cat >> "$tmp/test.c" <<'EOF'
#undef malloc
static int req_info, req_flag, block_send, disc_on_start, fill_result, target_state;
static void send_block(int channel, char code, int len, char *data)
{
  assert(channel == 1 && code == X_COMM && len == 1);
  assert(strcmp(data, "I") == 0 || strcmp(data, "C") == 0);
}
static void set_pwmode(int channel) { assert(channel == 1); }
static void clear_pwmode(int channel) { assert(channel == 1); }
static void set_remmode(int channel) { assert(channel == 1); }
static void set_flchkmode(int channel) { assert(channel == 1); }
static int fill_status(int channel, char *data)
{
  (void)data;
  assert(channel == 1);
  ch_stat[channel].oldstate = ch_stat[channel].state;
  ch_stat[channel].state = target_state;
  return fill_result;
}
#ifdef USE_IFACE
static int frames;
static int calc_xframes(int channel) { assert(channel == 1); return 0; }
static void send_cstatus(int channel, int count) { assert(channel == 1 && count == 0); }
#endif
static void setup_to_conn(int channel) { ch_stat[channel].conn_state = CS_CONN; }
static void next_sendframe(int channel) { assert(channel == 1); }
static int queue_cmd_data(int channel, char code, int len, int flag, char *data)
{
  (void)flag;
  assert(channel == 1 && code == X_COMM && len == 1 && strcmp(data, "D") == 0);
  return 0;
}
static void hostmode_reply(int reply, int state, char *rxbuffer)
{
  int channel = 1, res;
  switch (reply) {
EOF
# Compile the actual reply cases, not a second implementation of their logic.
awk '
  /^    case RQ_CHSTAT:/ { found = 1 }
  found && /^    case RQ_CURCALL:/ { exit }
  found { print }
  END { if (!found) exit 1 }
' "$source_dir/src/serial.c" | sed 's/\*state/state/g' >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
  }
}
static void next_command(int *state) { assert(*state == S_TNT_UP); }
static void hostmode_failure(int reply, int *state, char *rxbuffer)
{
  int channel = 1;
  switch (reply) {
EOF
awk '
  /^  case R_FAILMESS:/ { failure = 1 }
  failure && /^    case RQ_RECONNECT:/ { found = 1 }
  found && /^    case RQ_REDISC:/ { exit }
  found { print }
  END { if (!found) exit 1 }
' "$source_dir/src/serial.c" >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
  }
}
static void write_file(const char *path, const char *text)
{
  FILE *file = fopen(path, "w");
  assert(file != NULL && fputs(text, file) >= 0);
  assert(fclose(file) == 0);
  output[0] = '\0';
}
static void test_names(void)
{
  char new_name[] = "New name";
  for (int i = 1; i < 4; ++i) {
    ch_stat[i].conn_state = CS_CONN;
    update_name(i);
  }
  ch_stat[4].conn_state = CS_DISCON;
  rem_name(0, 0, 1, strlen(new_name), M_CONNECT, new_name);
  assert(strcmp(ch_stat[1].name, new_name) == 0);
  assert(strcmp(ch_stat[2].name, new_name) == 0);
  assert(ch_stat[3].name[0] == '\0');
  assert(ch_stat[4].name[0] == '\0' && status_updates[4] == 0);
  assert(status_updates[1] > 0 && status_updates[2] > 0);
  memset(status_updates, 0, sizeof(status_updates));
  fail_add = 1;
  rem_name(0, 0, 1, 7, M_CONNECT, "Ignored");
  assert(strcmp(ch_stat[1].name, new_name) == 0 && status_updates[1] == 0);
  fail_add = 0;
  rem_name(0, 0, 1, 0, M_CONNECT, "");
  assert(status_updates[1] == 0);
  altstat = 0;
  rem_name(0, 0, 1, 5, M_CONNECT, "Other");
  assert(status_updates[1] == 0 && strcmp(stored_name, "Other") == 0);
  puts("NAME refreshes connected channel caches and displays: PASS");
}
static void test_hostmode(void)
{
  char response[] = {1, R_SUCCMESS, 'D', 'N', 'O', '2', '8', '4', '\0'};
  int previous;
  ch_stat[1].conn_state = CS_CONN;
  hostmode_reply(RQ_CALL, S_TNT_UP, response);
  assert(strcmp(ch_stat[1].call, "DNO284") == 0);
  assert(strcmp(overview_call, "DNO284") == 0);
  assert(overview_updates == 1);
  previous = overview_updates;
  fill_result = 1;
  target_state = 4;
  hostmode_reply(RQ_CHSTAT, S_TNT_UP, response);
  assert(overview_updates == previous + 1);
  previous = overview_updates;
  /* Frame counters alone must not trigger an unnecessary overview redraw. */
  hostmode_reply(RQ_CHSTAT, S_TNT_UP, response);
  assert(overview_updates == previous);
  target_state = 0;
  hostmode_reply(RQ_CHSTAT, S_TNT_UP, response);
  assert(overview_updates == previous + 1);
  previous = overview_updates;
  fill_result = 0;
  hostmode_reply(RQ_CHSTAT, S_TNT_UP, response);
  assert(overview_updates == previous);
  assert(req_info == RQ_CURCALL && block_send == 1);
  strcpy(ch_stat[1].call_save, "DNO484");
  strcpy(ch_stat[1].call, "DNO284");
  strcpy(ch_stat[1].name, "Wrong cached name");
  previous = status_updates[1];
  {
    int state = S_TNT_UP;
    hostmode_failure(RQ_RECONNECT, &state, response);
  }
  assert(strcmp(ch_stat[1].call, "DNO484") == 0);
  assert(ch_stat[1].name[0] == '\0');
  assert(status_updates[1] == previous + 1);
  assert(strcmp(overview_call, "DNO484") == 0);
  puts("Hostmode callsign and changed status refresh overview immediately: PASS");
}
static void test_sys_reload(void)
{
  struct sysoplist *previous;
  write_file(tnt_sysfile, "DNO284 /synthetic/old.pw 0\n");
  cmd_loadsys(0, 0, 0, 0, M_COMMAND, NULL);
  assert(sysoplist_root && strcmp(sysoplist_root->callsign, "DNO284") == 0);
  previous = sysoplist_root;
  assert(unlink(tnt_sysfile) == 0);
  output[0] = '\0';
  cmd_loadsys(0, 0, 0, 0, M_COMMAND, NULL);
  assert(sysoplist_root == previous && strstr(output, OK_TEXT) == NULL);
  assert(output[0] != '\0');
  for (int budget = 0; budget < 2; ++budget) {
    write_file(tnt_sysfile, "DNO484 /synthetic/new.pw 1\nDNO486 /synthetic/other.pw 0\n");
    allocation_budget = budget;
    cmd_loadsys(0, 0, 0, 0, M_COMMAND, NULL);
    allocation_budget = -1;
    assert(sysoplist_root == previous && strstr(output, OK_TEXT) == NULL);
  }
  write_file(tnt_sysfile, "DNO484 /synthetic/new.pw 1\ninvalid\n");
  cmd_loadsys(0, 0, 0, 0, M_COMMAND, NULL);
  assert(sysoplist_root == previous && strstr(output, OK_TEXT) == NULL);
  ch_stat[1].pwwait = 5;
  strcpy(ch_stat[1].pwstr, "abcde");
  ch_stat[1].rootlevel = 1;
  ch_stat[1].access_level = 1;
  write_file(tnt_sysfile, "DNO484 /synthetic/new.pw 1\n");
  cmd_loadsys(0, 0, 0, 0, M_COMMAND, NULL);
  assert(strcmp(sysoplist_root->callsign, "DNO484") == 0);
  assert(ch_stat[1].pwwait == 0 && ch_stat[1].pwstr[0] == '\0');
  assert(ch_stat[1].rootlevel == 0 && ch_stat[1].access_level == 1);
  write_file(tnt_sysfile, "");
  cmd_loadsys(0, 0, 0, 0, M_COMMAND, NULL);
  assert(sysoplist_root == NULL);
  puts("LOADSYS retains valid tables on failure and resets pending challenges: PASS");
}
static void test_boxlist_reload(void)
{
  struct blcmdlist *previous;
  write_file(tnt_blcmdfile, "C*#R#READ\n");
  cmd_ldblcmd(0, 0, 0, 0, M_COMMAND, NULL);
  assert(blcmdlist_root && strcmp(blcmdlist_root->blcmd->command, "READ") == 0);
  previous = blcmdlist_root;
  assert(unlink(tnt_blcmdfile) == 0);
  output[0] = '\0';
  cmd_ldblcmd(0, 0, 0, 0, M_COMMAND, NULL);
  assert(blcmdlist_root == previous && strstr(output, OK_TEXT) == NULL);
  assert(output[0] != '\0');
  for (int budget = 0; budget < 3; ++budget) {
    write_file(tnt_blcmdfile, "C*#R#NEW\nX*#R#APPEND\n");
    allocation_budget = budget;
    cmd_ldblcmd(0, 0, 0, 0, M_COMMAND, NULL);
    allocation_budget = -1;
    assert(blcmdlist_root == previous && strstr(output, OK_TEXT) == NULL);
  }
  write_file(tnt_blcmdfile, "C*#R#NEW\ninvalid\n");
  cmd_ldblcmd(0, 0, 0, 0, M_COMMAND, NULL);
  assert(blcmdlist_root == previous && strstr(output, OK_TEXT) == NULL);
  write_file(tnt_blcmdfile, "C*#R#NEW\nX*#R#APPEND\n");
  cmd_ldblcmd(0, 0, 0, 0, M_COMMAND, NULL);
  assert(strcmp(blcmdlist_root->blcmd->command, "NEW") == 0);
  assert(strcmp(blcmdlist_root->blcmd->next->command, "APPEND") == 0);
  write_file(tnt_blcmdfile, "");
  cmd_ldblcmd(0, 0, 0, 0, M_COMMAND, NULL);
  assert(blcmdlist_root == NULL);
  puts("LDBLCMD retains valid commands on failure and replaces valid tables: PASS");
}
int main(int argc, char **argv)
{
  assert(argc == 3);
  assert(snprintf(tnt_sysfile, sizeof(tnt_sysfile), "%s/sys", argv[1])
         < (int)sizeof(tnt_sysfile));
  assert(snprintf(tnt_blcmdfile, sizeof(tnt_blcmdfile), "%s/blcmd", argv[1])
         < (int)sizeof(tnt_blcmdfile));
  if (strcmp(argv[2], "names") == 0) test_names();
  else if (strcmp(argv[2], "hostmode") == 0) test_hostmode();
  else if (strcmp(argv[2], "sys") == 0) test_sys_reload();
  else if (strcmp(argv[2], "boxlist") == 0) test_boxlist_reload();
  else assert(0);
  clear_sysfile();
  clear_blcmdfile();
  return 0;
}
EOF
${CC:-cc} -g -O2 -Wall -Wextra -Wstrict-prototypes \
  -Wold-style-definition -Werror -D_FORTIFY_SOURCE=3 \
  -fsanitize=address,undefined -I"$build_dir" -I"$source_dir/src" \
  "$tmp/test.c" -o "$tmp/test"
result=0
for case in names hostmode sys boxlist; do
  if "$tmp/test" "$tmp" "$case"; then
    :
  else
    result=1
  fi
done
exit "$result"
