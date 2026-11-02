#!/bin/sh
set -eu
source_dir=${top_srcdir:-$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)}
build_dir=${top_builddir:-$source_dir}
tmp=$(mktemp -d)
trap 'if test -d "$tmp/help/.program"; then rmdir "$tmp/help/.program"; fi; rm -f "$tmp/test.c" "$tmp/test" "$tmp/cookies" "$tmp/output" "$tmp/sysop" "$tmp/stdout" "$tmp/help/program" "$tmp/help/.program"; if test -d "$tmp/help"; then rmdir "$tmp/help"; fi; rmdir "$tmp"' EXIT HUP INT TERM

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
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static int step, failure;
static char transferred[8];
static ssize_t test_write(int fd, const void *buffer, size_t length)
{
  if (fd != -1234) return write(fd, buffer, length);
  if (step++ == 0) { errno = EINTR; return -1; }
  if (failure == 1) { errno = ENOSPC; return -1; }
  if (failure == 2) return 0;
  if (failure == 3) { errno = EAGAIN; return -1; }
  assert(length > 0 && step <= 4);
  transferred[step - 2] = *(const char *)buffer;
  return 1;
}
static ssize_t test_read(int fd, void *buffer, size_t length)
{
  if (fd != -1234) return read(fd, buffer, length);
  if (step++ == 0) { errno = EINTR; return -1; }
  if (failure == 1) { errno = EIO; return -1; }
  if (failure == 2) return 0;
  assert(length > 0 && step <= 4);
  *(char *)buffer = "abc"[step - 2];
  return 1;
}
#define write test_write
#define read test_read
#include "tntio.h"
#undef write
#undef read
#include "tnt.h"
char tnt_cookiefile[512];
char cook_head_str[MAXCHAR] = "header\n";
char cook_dash_str[MAXCHAR] = "--\n";
char cook_foot_str[MAXCHAR] = "footer\n";
static int x_random(int range) { assert(range > 0); return 0; }
EOF
extract 'static int write_cookie(' cookie.c
extract 'int gen_cookie(' cookie.c
cat >> "$tmp/test.c" <<'EOF'
#define LOG_LINELEN 120
EOF
extract 'static void append_log(' log.c
cat >> "$tmp/test.c" <<'EOF'
static char help_dir[512];
static int directory_error;
static struct dirent *test_readdir(DIR *directory)
{
  if (directory_error) {
    errno = EOVERFLOW;
    return NULL;
  }
  return readdir(directory);
}
#undef TNT_RUN_PATH
#define TNT_RUN_PATH help_dir
#define CUR_VER "test"
#define CUR_DAT "test"
#define main help_main
#define readdir test_readdir
EOF
extract 'int main (void)' runprgs/help.c
cat >> "$tmp/test.c" <<'EOF'
#undef main
#undef readdir
EOF
awk '
  /^struct sysoplist \{/ { found = 1 }
  found { print }
  found && /^};/ { exit }
  END { if (!found) exit 1 }
' "$source_dir/src/remote.c" >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
static struct sysoplist *sysoplist_root;
static struct channel_stat channels[1];
static struct channel_stat *ch_stat = channels;
static char rem_tnt_str[] = "TNT: ", rem_inv_str[] = "Invalid", rem_newlin_str[] = "\r";
static int random_calls, replies;
static void strip_call_log(char *call, int channel)
{
  assert(channel == 0);
  strcpy(call, "DB0ABC");
}
static int tnt_random(int first, int last)
{
  assert(first == 1 && last == 5 && random_calls >= 0 && random_calls < 5);
  return ++random_calls;
}
static void rem_data_display(int channel, char *buffer)
{
  assert(channel == 0 && buffer[0] != '\0');
}
static int queue_cmd_data(int channel, char code, int len, int flag, char *data)
{
  assert(channel == 0 && code == X_DATA && flag == 0);
  assert(len == (int)strlen(data));
  replies++;
  return 0;
}
EOF
extract 'void cmd_sysop(' remote.c
cat >> "$tmp/test.c" <<'EOF'
int main(int argc, char **argv)
{
  char result[512], path[512], line[LOG_LINELEN] = "prefix";
  const char expected[] = "header\n--\ntext\n\n--\nfooter\n";
  const char *passwords[] = {"abcde\n", "", "\n", "     \n", "abcd\n"};
  struct sysoplist entry = {0};
  FILE *file;
  int fd, org, i;
  assert(argc == 2);
  assert(tnt_write_all(-1234, "abc", 3) == 0);
  assert(step == 4 && memcmp(transferred, "abc", 3) == 0);
  step = 0;
  assert(tnt_read_all(-1234, result, 3) == 0);
  assert(step == 4 && memcmp(result, "abc", 3) == 0);
  for (failure = 1; failure <= 3; failure++) {
    step = 0;
    assert(tnt_write_all(-1234, "abc", 3) == -1 && step == 2);
  }
  for (failure = 1; failure <= 2; failure++) {
    step = 0;
    assert(tnt_read_all(-1234, result, 3) == -1 && step == 2);
  }
  failure = 0;
  memset(result, 'x', sizeof(result) - 1);
  result[sizeof(result) - 1] = '\0';
  append_log(line, result);
  assert(strlen(line) == LOG_LINELEN - 1);
  assert(strncmp(line, "prefix", 6) == 0);
  append_log(line, "ignored");
  assert(strlen(line) == LOG_LINELEN - 1);

  snprintf(tnt_cookiefile, sizeof(tnt_cookiefile), "%s/cookies", argv[1]);
  file = fopen(tnt_cookiefile, "w");
  assert(file != NULL);
  assert(fputs("\n%\ntext\n%\n", file) >= 0);
  for (i = 0; i < 2048; i++) assert(fputc('x', file) != EOF);
  assert(fclose(file) == 0);
  snprintf(path, sizeof(path), "%s/output", argv[1]);
  assert(gen_cookie(path, 1) == 0);
  fd = open(path, O_RDONLY);
  assert(fd >= 0);
  assert(read(fd, result, sizeof(result)) == (ssize_t)strlen(expected));
  assert(memcmp(result, expected, strlen(expected)) == 0);
  close(fd);
  org = open(tnt_cookiefile, O_RDONLY);
  assert(org >= 0);
  fd = open("/dev/full", O_WRONLY);
  assert(fd >= 0);
  assert(write_cookie(org, fd) != 0);
  close(org);
  close(fd);

  strcpy(entry.callsign, "DB0ABC");
  assert(snprintf(entry.sys_file, sizeof(entry.sys_file), "%s/sysop", argv[1])
         < (int)sizeof(entry.sys_file));
  sysoplist_root = &entry;
  for (i = 0; i < 5; i++) {
    file = fopen(entry.sys_file, "w");
    assert(file != NULL && fputs(passwords[i], file) >= 0);
    assert(fclose(file) == 0);
    random_calls = replies = 0;
    cmd_sysop(0, 0, 0, 0, 0, NULL);
    assert(replies == 1);
    assert(random_calls == (i == 0 ? 5 : 0));
    assert(ch_stat[0].pwwait == (i == 0 ? MAXPWLINES : 0));
    if (i == 0) assert(strcmp(ch_stat[0].pwstr, "abcde") == 0);
    else assert(ch_stat[0].pwstr[0] == '\0');
  }

  snprintf(help_dir, sizeof(help_dir), "%s/help", argv[1]);
  assert(mkdir(help_dir, 0700) == 0);
  snprintf(path, sizeof(path), "%s/help/program", argv[1]);
  file = fopen(path, "w");
  assert(file != NULL && fclose(file) == 0);
  snprintf(path, sizeof(path), "%s/help/.program", argv[1]);
  file = fopen(path, "w");
  assert(file != NULL);
  assert(fputs("help without final newline", file) >= 0);
  assert(fclose(file) == 0);
  assert(help_main() == 0);
  assert(unlink(path) == 0);
  assert(mkdir(path, 0700) == 0);
  assert(help_main() == 1);
  assert(rmdir(path) == 0);
  directory_error = 1;
  assert(help_main() == 1);
  return 0;
}
EOF
${CC:-cc} -g -O2 -Wall -Wextra -Wstrict-prototypes \
  -Wold-style-definition -Werror -D_FORTIFY_SOURCE=3 \
  -fsanitize=address,undefined -I"$build_dir" -I"$source_dir/src" \
  "$tmp/test.c" -o "$tmp/test"
"$tmp/test" "$tmp" > "$tmp/stdout"
grep -q '^help without final newline$' "$tmp/stdout"
