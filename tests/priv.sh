#!/bin/sh
set -eu
source_dir=${top_srcdir:-$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)}
build_dir=${top_builddir:-$source_dir}
tmp=$(mktemp -d)
trap 'rm -f "$tmp/test.c" "$tmp/test" "$tmp/passwords" "$tmp/new.pw"; rmdir "$tmp"' EXIT HUP INT TERM
cat > "$tmp/test.c" <<'EOF'
#include <assert.h>
#include "priv.c"

int tnc_channels = 4;
char tnt_conf_dir[MAXCHAR];
char rem_newlin_str[] = "\r";
static struct channel_stat channels[4];
struct channel_stat *ch_stat = channels;
static const char *calls[] = {"", "DNO284", "DNO484", "DNO486"};
static char output[2048];
static char sent[80];

void cmd_display(int flag, int channel, char *buffer, int cr)
{
  (void)flag; (void)channel; (void)cr;
  assert(strlen(output) + strlen(buffer) + 2 < sizeof(output));
  strcat(output, buffer);
  strcat(output, "\n");
}

int queue_cmd_data(int channel, char code, int len, int flag, char *data)
{
  assert(channel == 1 && code == X_DATA);
  (void)flag;
  assert(len >= 0 && (size_t)len < sizeof(sent));
  memcpy(sent, data, len);
  sent[len] = '\0';
  return 0;
}

void rem_data_display(int channel, char *buffer)
{
  (void)channel; (void)buffer;
}

void strip_call_log(char *call, int channel)
{
  strcpy(call, calls[channel]);
}

int x_random(int range)
{
  assert(range > 0);
  return 0;
}

static void write_passwords(const char *text)
{
  FILE *file = fopen(tnt_pwfile, "w");
  assert(file != NULL);
  assert(fputs(text, file) >= 0);
  assert(fclose(file) == 0);
}

static void reload(void)
{
  output[0] = '\0';
  cmd_loadpriv(0, 0, 0, 0, M_COMMAND, NULL);
}

int main(int argc, char **argv)
{
  struct calllist *old_entry;
  char path[MAXCHAR];
  FILE *file;
  assert(argc == 2);
  assert(snprintf(tnt_pwfile, sizeof(tnt_pwfile), "%s/passwords", argv[1])
         < (int)sizeof(tnt_pwfile));
  assert(snprintf(tnt_conf_dir, sizeof(tnt_conf_dir), "%s/", argv[1])
         < (int)sizeof(tnt_conf_dir));
  assert(snprintf(path, sizeof(path), "%s/new.pw", argv[1]) < (int)sizeof(path));
  file = fopen(path, "w");
  assert(file != NULL && fputs("synthetic-test-password\n", file) >= 0);
  assert(fclose(file) == 0);
  assert(alloc_priv() == 0);
  write_passwords("DNO484 FLEXNET 12345\n");
  init_priv();
  channels[1].conn_state = CS_CONN;
  channels[2].conn_state = CS_CONN;
  set_pwmode(1);
  set_pwmode(2);
  assert(pw_stat[1].pwmode == PW_NONE);
  assert(pw_stat[2].pwmode == PW_FLEXNET);

  write_passwords("DNO284 THENET new.pw 0 SYS\nDNO484 BAYCOM other.pw 0\n"
                  "DNO486 FLEXNET 54321\n");
  reload();
  assert(pw_stat[1].pwmode == PW_THENET);
  assert(strcmp(pw_stat[1].entry->pw_file, "new.pw") == 0);
  assert(pw_stat[2].pwmode == PW_BAYCOM && pw_stat[2].pass_wait == 1);
  assert(pw_stat[2].tries == 30);
  assert(pw_stat[3].pwmode == PW_NONE && pw_stat[3].entry == NULL);
  assert(strstr(output, OK_TEXT) != NULL);
  output[0] = '\0';
  cmd_priv(0, 0, 1, 0, M_COMMAND, NULL);
  assert(strcmp(sent, "SYS\r") == 0);
  assert(pw_stat[1].pass_wait == 1);
  assert(output[0] == '\0');

  /* Removed entries and pending challenges must not survive the reload. */
  pw_stat[1].pass_wait = 1;
  pw_stat[1].tries = 5;
  pw_stat[1].valid_try = 2;
  strcpy(pw_stat[1].box_login, "pending");
  pw_stat[3] = pw_stat[2];
  write_passwords("DNO284 THENET replacement.pw 0 SYS\n");
  reload();
  assert(strcmp(pw_stat[1].entry->pw_file, "replacement.pw") == 0);
  assert(pw_stat[1].pass_wait == 0 && pw_stat[1].tries == 0);
  assert(pw_stat[1].valid_try == 0 && pw_stat[1].box_login[0] == '\0');
  assert(pw_stat[2].pwmode == PW_NONE && pw_stat[2].entry == NULL);
  assert(pw_stat[2].pass_wait == 0 && pw_stat[2].tries == 0);
  assert(pw_stat[3].pwmode == PW_NONE && pw_stat[3].entry == NULL);
  output[0] = '\0';
  cmd_priv(0, 0, 2, 0, M_COMMAND, NULL);
  assert(strstr(output, "No password available") != NULL);

  /* An unreadable or malformed replacement retains the last valid table. */
  old_entry = pw_stat[1].entry;
  assert(unlink(tnt_pwfile) == 0);
  reload();
  assert(pw_stat[1].entry == old_entry);
  assert(strstr(output, OK_TEXT) == NULL);
  assert(output[0] != '\0');
  write_passwords("DNO284 FLEXNET 12345\ninvalid\n");
  reload();
  assert(pw_stat[1].entry == old_entry);
  assert(strcmp(pw_stat[1].entry->pw_file, "replacement.pw") == 0);
  assert(strstr(output, OK_TEXT) == NULL);

  write_passwords("");
  reload();
  assert(calllist_root == NULL);
  assert(pw_stat[1].pwmode == PW_NONE && pw_stat[1].entry == NULL);
  exit_priv();
  free_priv();
  puts("Password reload: connected, updated, removed and failed entries: PASS");
  return 0;
}
EOF
${CC:-cc} -g -O2 -Wall -Wextra -Wstrict-prototypes \
  -Wold-style-definition -Werror -D_FORTIFY_SOURCE=3 \
  -fsanitize=address,undefined -I"$build_dir" -I"$source_dir/src" \
  "$tmp/test.c" -o "$tmp/test"
"$tmp/test" "$tmp"
