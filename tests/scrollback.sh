#!/bin/sh
set -eu
source_dir=${top_srcdir:-$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)}
build_dir=${top_builddir:-$source_dir}
tmp=$(mktemp -d)
trap 'rm -f "$tmp/test.c" "$tmp/test" "$tmp/scrollback.dat" "$tmp/scrollback.dat.tmp" "$tmp/scrollback.journal"; rmdir "$tmp"' EXIT HUP INT TERM

cat > "$tmp/test.c" <<'EOF'
#include "tnt.h"
#include <assert.h>
#include <sys/wait.h>

static int tnc_channels = 2, insertmode, bscrhold_flag, scroll_reg;
static char att_normal;
static struct window cmdwin, monwin, inputs[2], outputs[2];
static struct window *prewin = inputs, *textwin = outputs;
static struct win_info real_info[5];
static char tnt_work_dir[4096];
static int sync_calls, fail_sync;

static int test_fsync(int fd)
{
  sync_calls++;
  if (fail_sync) { errno = EIO; return -1; }
  return fsync(fd);
}
#define fsync test_fsync
EOF
awk '
  /^#define SCROLLBACK_FILE / { found = 1 }
  found && /^#endif/ { exit }
  found { print }
' "$source_dir/src/display.c" >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
#undef fsync
static void win_scrollup(int part) { (void)part; assert(0); }
static void phy_newline(void) { assert(0); }
static void phy_gotoxy(int x, int y) { (void)x; (void)y; assert(0); }
static void real_window_down(int part, int page)
{ (void)part; (void)page; assert(0); }
static void update_window(int part) { (void)part; assert(0); }
static void set_cursor(int part) { (void)part; assert(0); }
EOF
for signature in 'int open_window(' 'static void inc_line_offset(' \
  'static void inc_phys_line(' 'static void cl_eoline(' 'static void newline('; do
  awk -v signature="$signature" '
    index($0,signature) == 1 { found = 1 }
    found { print }
    found && /^}/ { exit }
    END { if (!found) exit 1 }
  ' "$source_dir/src/window.c" >> "$tmp/test.c"
done
cat >> "$tmp/test.c" <<'EOF'
static void reset_windows(int rows)
{
  int i;
  if (scrollback_fd >= 0) assert(close(scrollback_fd) == 0);
  scrollback_fd = -1;
  scrollback_sequence = 0;
  scrollback_failed = scrollback_dirty = 0;
  scrollback_bytes = 0;
  free(cmdwin.window_buffer);
  free(monwin.window_buffer);
  assert(open_window(rows,&cmdwin,1) == 0);
  assert(open_window(rows,&monwin,1) == 0);
  for (i = 0; i < tnc_channels; i++) {
    free(inputs[i].window_buffer);
    free(outputs[i].window_buffer);
    assert(open_window(rows,&inputs[i],1) == 0);
    assert(open_window(rows,&outputs[i],1) == 0);
  }
}

static void put_line(struct window *win, char marker)
{
  win->window_buffer[win->phys_line * 2 * MAXCOLS + 1] = marker;
  win->column = 1;
  newline(win);
}

static char marker(struct window *win, int logical)
{
  return win->window_buffer[((logical + win->line_offset) % win->num_lines) *
                            2 * MAXCOLS + 1];
}

static off_t journal_size(void)
{
  struct stat st;
  assert(fstat(scrollback_fd,&st) == 0);
  return st.st_size;
}

static void legacy_snapshot(void)
{
  FILE *fp = fopen("scrollback.dat","wb");
  char row[2 * MAXCOLS];
  int kind, i, cell;
  assert(fp != NULL);
  assert(fwrite("TNTSCB1",1,8,fp) == 8);
  assert(scrollback_write_uint(fp,2));
  for (cell = 0; cell < 2 * MAXCOLS; cell += 2) {
    row[cell] = 0; row[cell + 1] = SPACE;
  }
  for (kind = SCROLLBACK_COMMAND; kind <= SCROLLBACK_MONITOR; kind++) {
    assert(scrollback_write_uint(fp,(unsigned long)kind));
    assert(scrollback_write_uint(fp,0));
    assert(scrollback_write_uint(fp,3));
    assert(scrollback_write_uint(fp,0));
    assert(scrollback_write_uint(fp,0));
    for (i = 0; i < 3; i++) {
      row[1] = kind == SCROLLBACK_COMMAND ? 'L' : 'M';
      assert(fwrite(row,1,sizeof(row),fp) == sizeof(row));
    }
  }
  assert(fclose(fp) == 0);
}

int main(int argc, char **argv)
{
  pid_t child;
  int status, before, i;
  FILE *fp;
  char magic[8];
  unsigned long records;
  struct stat st;
  unsigned char saved[SCROLLBACK_RECORD_SIZE];
  struct timespec begin, end;

  assert(argc == 2);
  assert(chdir(argv[1]) == 0);
  assert(snprintf(tnt_work_dir,sizeof(tnt_work_dir),"%s/",argv[1]) > 0);
  reset_windows(3);
  legacy_snapshot();
  init_scrollback();
  assert(!scrollback_failed);
  assert(marker(&cmdwin,0) == 'L');
  assert(marker(&monwin,0) == SPACE);
  assert(monwin.line_complete == NULL);
  fp = fopen("scrollback.dat","rb");
  assert(fp != NULL && fread(magic,1,8,fp) == 8);
  assert(memcmp(magic,"TNTSCB2",8) == 0);
  assert(scrollback_read_uint(fp,&records) && records == 5);
  assert(fclose(fp) == 0);
  assert(stat("scrollback.dat",&st) == 0 && (st.st_mode & 0777) == 0600);
  assert(stat("scrollback.journal",&st) == 0 && (st.st_mode & 0777) == 0600);

  child = fork();
  assert(child >= 0);
  if (child == 0) {
    reset_windows(3);
    init_scrollback();
    _exit(scrollback_failed && scrollback_fd == -1 ? 0 : 1);
  }
  assert(waitpid(child,&status,0) == child && WIFEXITED(status));
  assert(WEXITSTATUS(status) == 0);

  child = fork();
  assert(child >= 0);
  if (child == 0) {
    put_line(&outputs[1],'A');
    put_line(&outputs[1],'B');
    put_line(&inputs[1],'T');
    before = (int)journal_size();
    inputs[1].window_buffer[inputs[1].phys_line * 2 * MAXCOLS + 1] = 'U';
    inputs[1].column = 1;
    put_line(&monwin,'M');
    assert(journal_size() == before);
    assert(kill(getpid(),SIGKILL) == 0);
    _exit(1);
  }
  assert(waitpid(child,&status,0) == child && WIFSIGNALED(status));
  assert(WTERMSIG(status) == SIGKILL);
  reset_windows(3);
  init_scrollback();
  assert(!scrollback_failed && scrollback_sequence == 3);
  assert(marker(&outputs[1],0) == 'A' && marker(&outputs[1],1) == 'B');
  assert(marker(&outputs[1],2) == SPACE);
  assert(marker(&inputs[1],1) == 'T' && marker(&inputs[1],2) == SPACE);
  assert(marker(&monwin,1) == SPACE);
  assert(journal_size() == 0);

  put_line(&outputs[1],'C');
  assert(scrollback_sync_pending());
  before = sync_calls;
  scrollback_tick();
  assert(sync_calls == before);
  scrollback_last_sync.tv_sec -= 2;
  scrollback_tick();
  assert(sync_calls == before + 1 && !scrollback_sync_pending());
  scrollback_tick();
  assert(sync_calls == before + 1);
  /* Crash after snapshot rename but before journal truncation. */
  assert(save_scrollback());
  assert(journal_size() == SCROLLBACK_RECORD_SIZE);
  reset_windows(3);
  init_scrollback();
  assert(!scrollback_failed && marker(&outputs[1],0) == 'B');
  assert(marker(&outputs[1],1) == 'C' && marker(&outputs[1],2) == SPACE);

  put_line(&outputs[1],'D');
  assert(tnt_write_all(scrollback_fd,"short",5) == 0);
  reset_windows(2);
  init_scrollback();
  assert(!scrollback_failed);
  assert(marker(&outputs[1],0) == 'D' && marker(&outputs[1],1) == SPACE);

  /* A torn final checksummed record must not hide earlier valid records. */
  put_line(&outputs[1],'E');
  assert(pread(scrollback_fd,saved,sizeof(saved),0) == sizeof(saved));
  saved[40] ^= 1;
  assert(tnt_write_all(scrollback_fd,saved,sizeof(saved)) == 0);
  reset_windows(4);
  init_scrollback();
  assert(!scrollback_failed);
  assert(marker(&outputs[1],2) == 'E' && marker(&outputs[1],3) == SPACE);

  /* Cursor history edits survive replay without shifting the bottom row. */
  inputs[0].line = 1;
  inputs[0].phys_line = (inputs[0].line + inputs[0].line_offset) % 4;
  put_line(&inputs[0],'H');
  reset_windows(4);
  init_scrollback();
  assert(!scrollback_failed && marker(&inputs[0],1) == 'H');
  assert(inputs[0].line == 2);

  assert(clock_gettime(CLOCK_MONOTONIC,&begin) == 0);
  before = sync_calls;
  for (i = 0; i < 10000; i++) put_line(&outputs[0],'P');
  assert(clock_gettime(CLOCK_MONOTONIC,&end) == 0);
  assert(sync_calls == before);
  printf("10000 journal appends: %.3f seconds, %d bytes/line\n",
    (double)(end.tv_sec - begin.tv_sec) +
    (end.tv_nsec - begin.tv_nsec) / 1000000000.0, SCROLLBACK_RECORD_SIZE);
  scrollback_bytes = SCROLLBACK_COMPACT_SIZE;
  scrollback_last_sync.tv_sec -= 2;
  scrollback_tick();
  assert(!scrollback_failed && journal_size() == 0);

  /* Corruption in the middle must disable persistence, not overwrite evidence. */
  put_line(&outputs[1],'F');
  put_line(&outputs[1],'G');
  assert(pread(scrollback_fd,saved,sizeof(saved),0) == sizeof(saved));
  saved[40] ^= 1;
  {
    int fd = open("scrollback.journal",O_WRONLY);
    assert(fd >= 0 && pwrite(fd,saved,sizeof(saved),0) == sizeof(saved));
    assert(close(fd) == 0);
  }
  assert(stat("scrollback.journal",&st) == 0);
  {
    off_t size = st.st_size;
    reset_windows(4);
    init_scrollback();
    assert(scrollback_failed && scrollback_fd == -1);
    assert(stat("scrollback.journal",&st) == 0 && st.st_size == size);
  }
  assert(remove("scrollback.journal") == 0);
  reset_windows(4);
  init_scrollback();
  assert(!scrollback_failed);
  put_line(&outputs[0],'Z');
  fail_sync = 1;
  scrollback_last_sync.tv_sec -= 2;
  scrollback_tick();
  assert(scrollback_failed && scrollback_fd == -1);
  fail_sync = 0;
  reset_windows(4);
  init_scrollback();
  assert(!scrollback_failed && marker(&outputs[0],2) == 'Z');
  exit_scrollback();
  free(cmdwin.window_buffer);
  free(monwin.window_buffer);
  for (i = 0; i < 2; i++) {
    free(inputs[i].window_buffer);
    free(outputs[i].window_buffer);
  }
  return 0;
}
EOF
${CC:-cc} -Wall -Wextra -Werror -funsigned-char \
  -fsanitize=address,undefined -g -I"$build_dir" -I"$source_dir/src" \
  "$tmp/test.c" -o "$tmp/test"
"$tmp/test" "$tmp"
