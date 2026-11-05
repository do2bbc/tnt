/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993-1997 by Mark Wahl
   For license details see documentation
   Procedures for display (display.c)
   created: Mark Wahl DL4YBG 93/08/01
   updated: Mark Wahl DL4YBG 97/02/02
   updated: Matthias Hensler WS1LS 99/09/08
   updated: Johann Hanne DH3MB 99/01/02
   updated: Peter Mack DL3FCJ 99/03/19
   updated: Walter Koch DG9EP 99/08/15
*/

#include "tnt.h"
#ifndef DPBOXT
#include "xmon.h"
#endif

static void append_char(char *dst, size_t dst_size, char ch)
{
  size_t len = strlen(dst);

  if (len + 1 < dst_size) {
    dst[len] = ch;
    dst[len + 1] = '\0';
  }
}

static void append_spaces(char *dst, size_t dst_size, int count)
{
  size_t len;
  int i;

  if (count <= 0) return;
  len = strlen(dst);
  for (i = 0; (i < count) && (len + 1 < dst_size); i++) {
    dst[len++] = ' ';
  }
  dst[len] = '\0';
}

static void append_limited(char *dst, size_t dst_size, const char *src, size_t max_src)
{
  size_t len = strlen(dst);

  if (len + 1 < dst_size) {
    snprintf(dst + len, dst_size - len, "%.*s", (int)max_src, src);
  }
}

/* external function declarations */
extern int act_abin_rx(int channel);
extern int act_yapp_tx(int channel);
extern int act_yapp_rx(int channel);
extern void ana_response_yapp(int channel,char *buffer,int len);
extern int analyse_bcast_body(char *buffer);
extern void analyse_mon_body(char *buffer);
extern int analyse_mon_header(char *buffer);
extern void beep(void);
extern void boxcut(int channel, char *buffer, int len);
extern void boxcut_nocr(int channel, char *buffer, int len);
extern void boxcut_rest(int channel, char *buffer, int len);
extern int check_autoyapp(int channel, char *buffer, int len);
extern void check_mbeacon(char *buffer);
extern void check_uireq(char *buffer);
extern int close_window(struct window *win);
extern void connect_update(int channel, char *buffer, int len);
extern int decstathuf(char *src, char *dest);
extern int display_layer3(char *buffer, int is_i);
extern int extcomp_active(int screen);
extern int get_line(char *str, struct window *win);
extern int iface_active(int channel);
extern int insert_active(void);
extern void insert_cr_tx(int channel);
extern void mb_pre_display(char *buffer, int cr);
extern int open_window(int lines, struct window *win, int bot);
extern void out_cmd_socket(int channel, char *buffer);
extern int queue_cmd_data(int channel,char code,int len,int flag,char *data);
extern void real_screen(struct real_layout *layout);
extern void reinit_blist(void);
extern void reinit_mb(void);
extern void reinit_mheard(void);
extern void reinit_xmon(void);
extern int remote_input(int channel, char *buffer, int len);
extern void scan_login_received(int channel, char *buffer, int len);
extern void scan_pw_request(int channel, char *buffer, int len);
extern void send_tntresponse(int channel,int mode,char *str,int cr);
extern int shell_active(int channel);
extern void sock_status_out(int channel, char *str);
extern void strip_call_log(char *call, int channel);
extern void test_sysresponse(int channel,char *buffer,int len);
extern void win_attrib(char ch, struct window *win);
extern void win_charout(char ch, struct window *win, int conv);
extern void win_charout_cntl(char ch, struct window *win);
extern void win_stringout(char *str, struct window *win, int conv);
extern void win_textout(char *str, struct window *win, int conv);
extern void win_textout_len(char *str, int len, struct window *win, int conv);
extern void window_exit(void);
extern void window_init(void);
extern int wordwrap(char *str, struct window *win);
extern void write_file(int channel, int len, int flag, char *str);
extern void write_file_abin(int channel, int *len, int flag, char *str);
extern void write_file_yapp(int channel, char *str, int len);
extern void write_iface(int channel, int len, char *str);
extern void write_mon_info(char *buffer, int xmon_ch);
extern void write_monfile(int len, char *str, int append_cr);
extern int tnt_daemon;
extern int write_pty(int channel, int len, char *str);
extern int xconnect_active(int channel);
extern void xm_pre_display(int screen, char *buffer, int cr);

#ifdef DPBOXT
extern int act_channel;
extern int act_mode;
extern struct channel_stat *ch_stat;
extern char signon[];
extern char add_signon[];
extern int is_root;
extern int chan_flag;
extern int chan_val1;
extern char att_normal;
extern char att_statline;
extern char att_monitor;
extern char att_cstatline;
extern char att_controlchar; 
extern char att_remote;
extern char att_monport1;
extern char att_monport2;
extern int input_linelen;
extern int LINES,COLS;
extern int bl_mode;
extern int tabexp_flag;
extern int ptyecho_flag;

static struct window helpwin;
struct window statlin;

static struct real_layout help_layout[LAYOUTPARTS];
extern struct real_layout (*boxlist_layout)[LAYOUTPARTS];
extern struct real_layout mb_layout[LAYOUTPARTS];
extern struct real_layout mbboxlist_layout[LAYOUTPARTS];
#else /* DPBOXT */

extern int tnc_channels;
extern int r_channels;
extern int act_channel;
extern int act_mode;
extern int xmon_screen;
extern struct channel_stat *ch_stat;
extern struct rx_file *rx_file;
extern char signon[];
extern char add_signon[];
extern int is_root;
extern int chan_flag;
extern int chan_val1;
extern int resync;
extern char att_normal;
extern char att_statline;
extern char att_monitor;
extern char att_cstatline;
extern char att_controlchar; 
extern char att_remote;
extern char att_monport1;
extern char att_monport2;
extern int lines_command;
extern int lines_monitor;
extern int lines_input;
extern int lines_output;
extern int lines_moncon;
extern int lines_r_input;
extern int lines_r_output;
extern int input_linelen;
extern char tnt_work_dir[];
extern char rem_tnt_str[];
extern char rem_newlin_str[];
extern int infobell_flag;
extern int cbell_flag;
extern int layer3_flag;
extern int tabexp_flag;
extern int ptyecho_flag;
extern int rxbell_flag;
extern int LINES,COLS;
extern int tnc_busy;
extern struct tx_file tx_file_com;
extern int moni_decomp;
extern int moni_chkbin;
extern int scrollhold_flag;
#ifdef USE_IFACE
extern int bl_mode;
#endif

int scr_divide;
int altstat;
static int xmon_ch;
static int layer3_frame;
static int layer3_inp;
static struct window *prewin;
static struct window *statwin;
static struct window *textwin;
static struct window cmdwin;
static struct window monwin;
static struct window helpwin;
struct window statlin;

struct real_layout (*layout)[LAYOUTPARTS];
static struct real_layout cmd_layout[LAYOUTPARTS];
static struct real_layout mon_layout[LAYOUTPARTS];
static struct real_layout help_layout[LAYOUTPARTS];
extern struct real_layout xmon_layout[XMON_SCREENS][LAYOUTPARTS];
extern struct real_layout (*boxlist_layout)[LAYOUTPARTS];
extern struct real_layout heard_layout[LAYOUTPARTS];
#ifdef USE_IFACE
extern struct real_layout mb_layout[LAYOUTPARTS];
extern struct real_layout mbboxlist_layout[LAYOUTPARTS];
extern int scanmbea_valid;
#endif /* USE_IFACE */
#endif /* DPBOXT */

#ifndef DPBOXT
#define SCROLLBACK_FILE "scrollback.dat"
#define SCROLLBACK_TMP_FILE "scrollback.dat.tmp"
#define SCROLLBACK_JOURNAL "scrollback.journal"
#define SCROLLBACK_COMPACT_SIZE (32 * 1024 * 1024)
#define SCROLLBACK_RECORD_SIZE (8 * 4 + 2 * MAXCOLS)
#define SCROLLBACK_MAX_LINES 100000
#define SCROLLBACK_COMMAND 1
#define SCROLLBACK_MONITOR 2
#define SCROLLBACK_INPUT 3
#define SCROLLBACK_OUTPUT 4

static const char scrollback_magic[8] = {
  'T', 'N', 'T', 'S', 'C', 'B', '2', '\0'
};
static unsigned long scrollback_sequence;
static int scrollback_fd = -1;
static int scrollback_failed;
static int scrollback_dirty;
static off_t scrollback_bytes;
static struct timespec scrollback_last_sync;

static void scrollback_encode_uint(unsigned char *data, unsigned long value)
{
  data[0] = (unsigned char)(value & 0xff);
  data[1] = (unsigned char)((value >> 8) & 0xff);
  data[2] = (unsigned char)((value >> 16) & 0xff);
  data[3] = (unsigned char)((value >> 24) & 0xff);
}

static unsigned long scrollback_decode_uint(const unsigned char *data)
{
  return (unsigned long)data[0] |
           ((unsigned long)data[1] << 8) |
           ((unsigned long)data[2] << 16) |
           ((unsigned long)data[3] << 24);
}

static int scrollback_write_uint(FILE *fp, unsigned long value)
{
  unsigned char data[4];

  scrollback_encode_uint(data,value);
  return fwrite(data,1,sizeof(data),fp) == sizeof(data);
}

static int scrollback_read_uint(FILE *fp, unsigned long *value)
{
  unsigned char data[4];

  if (fread(data,1,sizeof(data),fp) != sizeof(data)) return(0);
  *value = scrollback_decode_uint(data);
  return(1);
}

static struct window *scrollback_window(unsigned long kind,
                                        unsigned long channel)
{
  switch (kind) {
  case SCROLLBACK_COMMAND:
    if (channel == 0) return(&cmdwin);
    break;
  case SCROLLBACK_MONITOR:
    break;
  case SCROLLBACK_INPUT:
    if (channel < (unsigned long)tnc_channels) return(&prewin[channel]);
    break;
  case SCROLLBACK_OUTPUT:
    if (channel < (unsigned long)tnc_channels) return(&textwin[channel]);
    break;
  }
  return(NULL);
}

static int scrollback_write_window(FILE *fp, unsigned long kind,
                                   unsigned long channel, struct window *win)
{
  int i;
  int physical_line;
  size_t row_size = 2 * MAXCOLS;

  if (!scrollback_write_uint(fp,kind) ||
      !scrollback_write_uint(fp,channel) ||
      !scrollback_write_uint(fp,(unsigned long)win->num_lines) ||
      !scrollback_write_uint(fp,(unsigned long)win->column) ||
      !scrollback_write_uint(fp,(unsigned char)win->attribut) ||
      !scrollback_write_uint(fp,(unsigned long)win->line)) return(0);

  for (i = 0; i < win->num_lines; i++) {
    physical_line = i + win->line_offset;
    if (physical_line >= win->num_lines) physical_line -= win->num_lines;
    if (fwrite(win->window_buffer + physical_line * row_size,
               1,row_size,fp) != row_size) return(0);
  }
  return(1);
}

static int scrollback_read_window(FILE *fp, int legacy)
{
  unsigned long kind;
  unsigned long channel;
  unsigned long source_lines;
  unsigned long column;
  unsigned long attribut;
  unsigned long line;
  unsigned long source_start;
  unsigned long copy_lines;
  unsigned long i;
  unsigned long destination_line;
  char row[2 * MAXCOLS];
  struct window *win;

  if (!scrollback_read_uint(fp,&kind) ||
      !scrollback_read_uint(fp,&channel) ||
      !scrollback_read_uint(fp,&source_lines) ||
      !scrollback_read_uint(fp,&column) ||
      !scrollback_read_uint(fp,&attribut) ||
      source_lines == 0 || source_lines > SCROLLBACK_MAX_LINES ||
      column >= MAXCOLS || attribut >= 9) return(0);
  line = source_lines - 1;
  if (!legacy && (!scrollback_read_uint(fp,&line) || line >= source_lines))
    return(0);

  win = scrollback_window(kind,channel);
  copy_lines = 0;
  source_start = source_lines;
  if ((win != NULL) && (win->window_buffer != NULL)) {
    copy_lines = source_lines;
    if (copy_lines > (unsigned long)win->num_lines)
      copy_lines = (unsigned long)win->num_lines;
    source_start = source_lines - copy_lines;
  }

  for (i = 0; i < source_lines; i++) {
    size_t cell;
    if (fread(row,1,sizeof(row),fp) != sizeof(row)) return(0);
    for (cell = 0; cell < sizeof(row); cell += 2)
      if ((unsigned char)row[cell] >= 9) return(0);
    if (i >= source_start) {
      destination_line = (unsigned long)win->num_lines - copy_lines +
                         i - source_start;
      memcpy(win->window_buffer + destination_line * sizeof(row),
             row,sizeof(row));
    }
  }

  if (win != NULL) {
    long restored_line = (long)win->num_lines - (long)source_lines + (long)line;
    win->line = restored_line < 0 ? 0 : (int)restored_line;
    win->line_offset = 0;
    win->phys_line = win->line;
    win->column = (int)column;
    win->attribut = (char)attribut;
    win->first_win_line = 0;
    win->view_column = 0;
  }
  return(1);
}

static void scrollback_path(char *path, size_t size, const char *filename)
{
  snprintf(path,size,"%s%s",tnt_work_dir,filename);
}

static int load_scrollback(void)
{
  char path[MAXCHAR * 2];
  char magic[sizeof(scrollback_magic)];
  unsigned long records;
  unsigned long i;
  FILE *fp;
  int legacy;

  scrollback_path(path,sizeof(path),SCROLLBACK_FILE);
  fp = fopen(path,"rb");
  if (fp == NULL) {
    if (errno == ENOENT) return(1);
    perror(path);
    return(0);
  }

  if (fread(magic,1,sizeof(magic),fp) != sizeof(magic) ||
      (memcmp(magic,scrollback_magic,sizeof(magic)) != 0 &&
       memcmp(magic,"TNTSCB1",sizeof(magic)) != 0) ||
      !scrollback_read_uint(fp,&records) ||
      records > 10000) {
    fprintf(stderr,_("ERROR: invalid scrollback file %s\n"),path);
    fclose(fp);
    return(0);
  }
  legacy = magic[6] == '1';
  if (!legacy && !scrollback_read_uint(fp,&scrollback_sequence)) {
    fprintf(stderr,_("ERROR: invalid scrollback file %s\n"),path);
    fclose(fp);
    return(0);
  }

  for (i = 0; i < records; i++) {
    if (!scrollback_read_window(fp,legacy)) {
      fprintf(stderr,_("ERROR: invalid scrollback file %s\n"),path);
      fclose(fp);
      return(0);
    }
  }
  if (fgetc(fp) != EOF || ferror(fp)) {
    fprintf(stderr,_("ERROR: invalid scrollback file %s\n"),path);
    fclose(fp);
    return(0);
  }
  return fclose(fp) == 0;
}

static int save_scrollback(void)
{
  char path[MAXCHAR * 2];
  char tmp_path[MAXCHAR * 2];
  unsigned long records = 1 + 2 * (unsigned long)tnc_channels;
  int i;
  int ok;
  FILE *fp;
  int fd;

  scrollback_path(path,sizeof(path),SCROLLBACK_FILE);
  scrollback_path(tmp_path,sizeof(tmp_path),SCROLLBACK_TMP_FILE);
  fd = open(tmp_path,O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW,0600);
  if (fd < 0) {
    perror(tmp_path);
    return(0);
  }
  if (fchmod(fd,S_IRUSR | S_IWUSR) < 0) {
    perror(tmp_path);
    close(fd);
    remove(tmp_path);
    return(0);
  }
  fp = fdopen(fd,"wb");
  if (fp == NULL) {
    perror(tmp_path);
    close(fd);
    remove(tmp_path);
    return(0);
  }

  ok = fwrite(scrollback_magic,1,sizeof(scrollback_magic),fp) ==
       sizeof(scrollback_magic);
  if (ok) ok = scrollback_write_uint(fp,records);
  if (ok) ok = scrollback_write_uint(fp,scrollback_sequence);
  if (ok) ok = scrollback_write_window(fp,SCROLLBACK_COMMAND,0,&cmdwin);
  for (i = 0; ok && (i < tnc_channels); i++)
    ok = scrollback_write_window(fp,SCROLLBACK_INPUT,(unsigned long)i,
                                 &prewin[i]);
  for (i = 0; ok && (i < tnc_channels); i++)
    ok = scrollback_write_window(fp,SCROLLBACK_OUTPUT,(unsigned long)i,
                                 &textwin[i]);
  if (fflush(fp) != 0) ok = 0;
  if (ok && fsync(fd) < 0) ok = 0;
  if (fclose(fp) != 0) ok = 0;

  if (!ok) {
    fprintf(stderr,_("ERROR: cannot save scrollback file %s\n"),path);
    remove(tmp_path);
    return(0);
  }
  if (rename(tmp_path,path) < 0) {
    perror(path);
    remove(tmp_path);
    return(0);
  }
  fd = open(tnt_work_dir,O_RDONLY | O_DIRECTORY);
  if (fd < 0) {
    perror(tnt_work_dir);
    return(0);
  }
  ok = fsync(fd) == 0;
  if (!ok) perror(tnt_work_dir);
  if (close(fd) < 0) {
    perror(tnt_work_dir);
    ok = 0;
  }
  return(ok);
}

static void scrollback_disable(void)
{
  fprintf(stderr,_("ERROR: live scrollback persistence disabled; check scrollback files and disk access\n"));
  scrollback_failed = 1;
  if (scrollback_fd >= 0) {
    close(scrollback_fd);
    scrollback_fd = -1;
  }
}

static unsigned long scrollback_checksum(const unsigned char *data, size_t size)
{
  unsigned long hash = 2166136261UL;
  size_t i;

  for (i = 0; i < size; i++)
    hash = ((hash ^ data[i]) * 16777619UL) & 0xffffffffUL;
  return(hash);
}

static int scrollback_replay(void)
{
  unsigned char record[SCROLLBACK_RECORD_SIZE];
  unsigned long previous = 0;
  struct stat st;
  off_t valid_bytes = 0;
  size_t used = 0;
  ssize_t count;

  if (fstat(scrollback_fd,&st) < 0) {
    perror("scrollback journal stat");
    return(0);
  }
  for (;;) {
    count = read(scrollback_fd,record + used,sizeof(record) - used);
    if (count < 0 && errno == EINTR) continue;
    if (count < 0) {
      perror("scrollback journal read");
      return(0);
    }
    if (count == 0) {
      if (used != 0) {
        fprintf(stderr,_("WARNING: discarding incomplete final scrollback journal record\n"));
        if (ftruncate(scrollback_fd,valid_bytes) < 0) {
          perror("scrollback journal truncate");
          return(0);
        }
      }
      break;
    }
    used += (size_t)count;
    if (used == sizeof(record)) {
      unsigned long sequence = scrollback_decode_uint(record + 4);
      unsigned long kind = scrollback_decode_uint(record + 8);
      unsigned long channel = scrollback_decode_uint(record + 12);
      unsigned long lines = scrollback_decode_uint(record + 16);
      unsigned long line = scrollback_decode_uint(record + 20);
      unsigned long attribut = scrollback_decode_uint(record + 24);
      struct window *win;
      long destination;
      int physical;
      size_t cell;

      if (scrollback_decode_uint(record) != 0x314a4253UL ||
          scrollback_decode_uint(record + sizeof(record) - 4) !=
            scrollback_checksum(record,sizeof(record) - 4)) {
        if (valid_bytes + (off_t)sizeof(record) == st.st_size) {
          fprintf(stderr,_("WARNING: discarding damaged final scrollback journal record\n"));
          if (ftruncate(scrollback_fd,valid_bytes) < 0) {
            perror("scrollback journal truncate");
            return(0);
          }
          break;
        }
        fprintf(stderr,_("ERROR: corrupt scrollback journal\n"));
        return(0);
      }
      if (sequence == 0 || (previous != 0 && sequence != previous + 1) ||
          (sequence > scrollback_sequence &&
           sequence != scrollback_sequence + 1) ||
          lines == 0 || lines > SCROLLBACK_MAX_LINES || line >= lines ||
          attribut >= 9 ||
          (kind != SCROLLBACK_COMMAND && kind != SCROLLBACK_INPUT &&
           kind != SCROLLBACK_OUTPUT) ||
          (kind == SCROLLBACK_COMMAND && channel != 0)) {
        fprintf(stderr,_("ERROR: invalid scrollback journal record\n"));
        return(0);
      }
      for (cell = 28; cell < sizeof(record) - 4; cell += 2) {
        if (record[cell] >= 9) {
          fprintf(stderr,_("ERROR: invalid scrollback journal attribute\n"));
          return(0);
        }
      }
      previous = sequence;
      if (sequence > scrollback_sequence) {
        win = scrollback_window(kind,channel);
        if (win != NULL) {
          destination = (long)win->num_lines - (long)lines + (long)line;
          if (destination >= 0) {
            physical = ((int)destination + win->line_offset) % win->num_lines;
            memcpy(win->window_buffer + physical * 2 * MAXCOLS,
                   record + 28,2 * MAXCOLS);
            win->line = (int)destination;
            if (win->line == win->num_lines - 1) {
              win->line_offset = (win->line_offset + 1) % win->num_lines;
              physical = (physical + 1) % win->num_lines;
              for (cell = 0; cell < 2 * MAXCOLS; cell += 2) {
                win->window_buffer[physical * 2 * MAXCOLS + cell] = 0;
                win->window_buffer[physical * 2 * MAXCOLS + cell + 1] = SPACE;
              }
            }
            else {
              win->line++;
              physical = (physical + 1) % win->num_lines;
            }
            win->phys_line = physical;
            win->column = 0;
            win->attribut = (char)attribut;
          }
        }
        scrollback_sequence = sequence;
      }
      valid_bytes += sizeof(record);
      used = 0;
    }
  }
  scrollback_bytes = valid_bytes;
  return(1);
}

static int scrollback_checkpoint(void)
{
  if (!save_scrollback()) return(0);
  /* The durable snapshot watermark makes an old journal safe to replay. */
  if (ftruncate(scrollback_fd,0) < 0 || fsync(scrollback_fd) < 0) {
    perror("scrollback journal checkpoint");
    return(0);
  }
  scrollback_bytes = 0;
  scrollback_dirty = 0;
  return(1);
}

static void scrollback_complete_line(struct window *win)
{
  unsigned char record[SCROLLBACK_RECORD_SIZE];
  unsigned long kind = SCROLLBACK_COMMAND;
  unsigned long channel = 0;
  int i;

  if (scrollback_fd < 0 || scrollback_failed) return;
  if (win != &cmdwin) {
    for (i = 0; i < tnc_channels; i++) {
      if (win == &prewin[i] || win == &textwin[i]) {
        kind = win == &prewin[i] ? SCROLLBACK_INPUT : SCROLLBACK_OUTPUT;
        channel = (unsigned long)i;
        break;
      }
    }
    if (i == tnc_channels) return;
  }
  if (scrollback_sequence == 0xffffffffUL) {
    fprintf(stderr,_("ERROR: scrollback journal sequence exhausted\n"));
    scrollback_disable();
    return;
  }
  scrollback_encode_uint(record,0x314a4253UL);
  scrollback_encode_uint(record + 4,scrollback_sequence + 1);
  scrollback_encode_uint(record + 8,kind);
  scrollback_encode_uint(record + 12,channel);
  scrollback_encode_uint(record + 16,(unsigned long)win->num_lines);
  scrollback_encode_uint(record + 20,(unsigned long)win->line);
  scrollback_encode_uint(record + 24,(unsigned char)win->attribut);
  memcpy(record + 28,win->window_buffer + win->phys_line * 2 * MAXCOLS,
         2 * MAXCOLS);
  scrollback_encode_uint(record + sizeof(record) - 4,
                         scrollback_checksum(record,sizeof(record) - 4));
  if (tnt_write_all(scrollback_fd,record,sizeof(record)) < 0) {
    scrollback_disable();
    return;
  }
  scrollback_sequence++;
  scrollback_bytes += sizeof(record);
  scrollback_dirty = 1;
}

void scrollback_tick(void)
{
  struct timespec now;
  double elapsed;

  if (scrollback_fd < 0 || !scrollback_dirty) return;
  if (clock_gettime(CLOCK_MONOTONIC,&now) < 0) {
    perror("scrollback clock");
    scrollback_disable();
    return;
  }
  elapsed = (double)(now.tv_sec - scrollback_last_sync.tv_sec) +
            (now.tv_nsec - scrollback_last_sync.tv_nsec) / 1000000000.0;
  if (elapsed < 1.0) return;
  if (fsync(scrollback_fd) < 0) {
    perror("scrollback journal sync");
    scrollback_disable();
    return;
  }
  scrollback_last_sync = now;
  scrollback_dirty = 0;
  if (scrollback_bytes >= SCROLLBACK_COMPACT_SIZE && !scrollback_checkpoint())
    scrollback_disable();
}

int scrollback_sync_pending(void)
{
  return scrollback_fd >= 0 && scrollback_dirty;
}

static void init_scrollback(void)
{
  char path[MAXCHAR * 2];
  int i;
  struct flock lock;

  if (!load_scrollback()) {
    scrollback_disable();
    return;
  }
  scrollback_path(path,sizeof(path),SCROLLBACK_JOURNAL);
  scrollback_fd = open(path,O_RDWR | O_CREAT | O_APPEND | O_NOFOLLOW,0600);
  if (scrollback_fd < 0) {
    perror(path);
    scrollback_disable();
    return;
  }
  memset(&lock,0,sizeof(lock));
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  if (fcntl(scrollback_fd,F_SETLK,&lock) < 0) {
    perror("scrollback journal lock");
    scrollback_disable();
    return;
  }
  if (fchmod(scrollback_fd,0600) < 0 ||
      fcntl(scrollback_fd,F_SETFD,FD_CLOEXEC) < 0) {
    perror(path);
    scrollback_disable();
    return;
  }
  if (!scrollback_replay() || !scrollback_checkpoint()) {
    scrollback_disable();
    return;
  }
  if (clock_gettime(CLOCK_MONOTONIC,&scrollback_last_sync) < 0) {
    perror("scrollback clock");
    scrollback_disable();
    return;
  }
  cmdwin.line_complete = scrollback_complete_line;
  for (i = 0; i < tnc_channels; i++) {
    prewin[i].line_complete = scrollback_complete_line;
    textwin[i].line_complete = scrollback_complete_line;
  }
}

static void exit_scrollback(void)
{
  if (scrollback_fd < 0) return;
  if (!scrollback_checkpoint()) scrollback_disable();
  if (scrollback_fd >= 0) {
    if (close(scrollback_fd) < 0) perror("scrollback journal close");
    scrollback_fd = -1;
  }
}
#endif

static int old_mode;
#ifndef DPBOXT
static int channel_at_column[MAXCOLS];
void sel_connect(int channel);
#endif
static int help_available;
char tnt_help_file[MAXCHAR];

#ifndef DPBOXT
static char *ax_state[] = {
  "************",
  " LINK SETUP ",
  " FRAME REJ *",
  " DISC. LINK ",
  " INFO XFER *",
  " REJECT SND ",
  " WAIT ACK **",
  " LOCAL BUSY ",
  " REM. BUSY *",
  " BOTH BUSY *",
  " LOCAL BUSY ",
  " REM. BUSY *",
  " BOTH BUSY *",
  " LOCAL BUSY ",
  " REM. BUSY *",
  " BOTH BUSY *"
};

static char *ax_state_alt[] = {
  " discon  ",
  " SET to  ",
  " FRMR fm ",
  " DRQ to  ",
  " IXF wid ",
  " REJ to  ",
  " WACK fm ",
  " LOCBUSY ",
  " BUSY fm ",
  " BUSY    ",
  " LOCBUSY ",
  " BUSY fm ",
  " BUSY    ",
  " LOCBUSY  ",
  " BUSY fm ",
  " BUSY    "
};
#endif
  
void statlin_update(void)
{
  int i,j;
  char tmp = '*';
  char *tmpstr;
  int channel;
  char ch_str[5];
  int ch_offset;
  char tmp_call[13];
  char call_sign[12];

  tmpstr = (char *)malloc(COLS+2);
#ifndef DPBOXT
  for (i = 0; i < MAXCOLS; ++i) channel_at_column[i] = -1;
#endif
  switch (act_mode) {
#ifdef DPBOXT
  case M_BOXLIST:
    strcpy(tmpstr,"\015** BOXLIST *");
    tmp = '*';
    break;  
  case M_HELP:
    strcpy(tmpstr,"\015*** HELP ***");
    tmp = '*';
    break;  
  case M_MAILBOX:
    strcpy(tmpstr,"\015** MAILBOX *");
    if (insert_active())
      tmp = 'I';
    else 
      tmp = '*';
    break;  
  }
  append_char(tmpstr, COLS + 2, tmp);
  if (act_mode == M_MAILBOX) {
    strcat(tmpstr,"**************");
  }
  else if ((act_mode == M_BOXLIST) && (bl_mode == M_MAILBOX)) {
    strcat(tmpstr,"* MAILBOX ****");
  }
  show_rxfile(act_channel,tmpstr);
  tmp = '*';
  append_char(tmpstr, COLS + 2, tmp);
  show_txfile(act_channel,tmpstr);
  for (i=strlen(tmpstr);i<COLS+1;i++) {
    tmpstr[i] = '*';
  }
  tmpstr[COLS+1] = '\0';
#else
  case M_CONNECT:
    if (ch_stat[act_channel].huffcod)
      strcpy(tmpstr,"\015C(H)");
    else
      strcpy(tmpstr,"\015*C*");
    if (resync)
      tmp = 'S';
    else if (tnc_busy)
      tmp = 'B';
    else if (ch_stat[act_channel].pause_flag)
      tmp = 'P';
    else if (xconnect_active(act_channel))
      tmp = 'X';
    else if (insert_active())
      tmp = 'I';
    else
      tmp = '*';
    break;
  case M_COMMAND:
    strcpy(tmpstr,"\015*Cmd*");
    if (resync)
      tmp = 'S';
    else if (tnc_busy)
      tmp = 'B';
    else if (insert_active())
      tmp = 'I';
    else 
      tmp = '*';
    break;
  case M_MONITOR:
    strcpy(tmpstr,"\015*Mon*");
    if (resync)
      tmp = 'S';
    else if (tnc_busy)
      tmp = 'B';
    else if (ch_stat[0].pause_flag)
      tmp = 'P';
    else 
      tmp = '*';
    break;  
  case M_EXTMON:
    if (extcomp_active(xmon_screen))
      strcpy(tmpstr,"\015*X(H)*");
    else
      strcpy(tmpstr,"\015*XMon*");
    if (resync)
      tmp = 'S';
    else if (tnc_busy)
      tmp = 'B';
    else if (insert_active())
      tmp = 'I';
    else 
      tmp = '*';
    break;  
  case M_BOXLIST:
    strcpy(tmpstr,"\015*BList*");
    if (resync)
      tmp = 'S';
    else if (tnc_busy)
      tmp = 'B';
    else 
      tmp = '*';
    break;  
  case M_HEARD:
    strcpy(tmpstr,"\015*MH*");
    if (resync)
      tmp = 'S';
    else if (tnc_busy)
      tmp = 'B';
    else 
      tmp = '*';
    break;  
  case M_HELP:
    strcpy(tmpstr,"\015*Help*");
    if (resync)
      tmp = 'S';
    else if (tnc_busy)
      tmp = 'B';
    else 
      tmp = '*';
    break;  
#ifdef USE_IFACE
  case M_MAILBOX:
    strcpy(tmpstr,"\015*Box*");
    if (resync)
      tmp = 'S';
    else if (tnc_busy)
      tmp = 'B';
    else if (insert_active())
      tmp = 'I';
    else 
      tmp = '*';
    break;  
#endif
  }
  append_char(tmpstr, COLS + 2, tmp);
#ifdef USE_IFACE
  if ((act_mode == M_MAILBOX) && (!chan_flag)) {
    strcat(tmpstr,"******");
  }
  else if ((act_mode == M_BOXLIST) && (bl_mode == M_MAILBOX) && (!chan_flag)) {
    strcat(tmpstr,"*MB*");
  }
  else {
/*    strcat(tmpstr,"*"); */
    if (chan_flag == 1) {
      strcat(tmpstr,"??");
    }
    else if (chan_flag == 2) {
      sprintf(ch_str,"%01d?",chan_val1);
      strcat(tmpstr,ch_str);
    }
    else {
      if (act_mode == M_EXTMON) {
        channel = xmon_screen;
      }
      else {
        channel = act_channel;
      }
      sprintf(ch_str,"%02d",channel);
      strcat(tmpstr,ch_str);
    }
    strcat(tmpstr,"*");
  }
#else
/*  strcat(tmpstr,"*"); */
  if (chan_flag) {
    strcat(tmpstr,"??");
  }
  else {
    if (act_mode == M_EXTMON) {
      channel = xmon_screen;
    }
    else {
      channel = act_channel;
    }
    sprintf(ch_str,"%0.2d",channel);
    strcat(tmpstr,ch_str);
  }
  strcat(tmpstr,"*");
#endif
/*  show_rxfile(act_channel,tmpstr);
  tmp = '*';
  strncat(tmpstr,&tmp,1);
  show_txfile(act_channel,tmpstr); */
  strcat(tmpstr,"*****");
  tmpstr[11] = '\0';
/*  ch_offset = (act_channel / 10) * 10;*/
  ch_offset = (((act_channel-1)/10) *10) +1;
  for (i = (0+ch_offset); i < (10+ch_offset); i++) {
    if (i >= tnc_channels) {
      strcpy(tmp_call, "      ");
    }
    else {
      if (i != (0+ch_offset)) strcpy(tmp_call, " ");
      else tmp_call[0] = '\0';
      if ((i == act_channel) && (act_mode == M_CONNECT)) {
        ch_stat[i].not_disp = 0;
      }
      if (ch_stat[i].not_disp) {
        strip_call_log(call_sign, i);
        if( strncmp(call_sign, "CHANNE", 6) == 0) {
          snprintf(ch_str, sizeof(ch_str), "%02d", i % 100);
          strcat(tmp_call, ch_str);
          strcat(tmp_call, ":DSC");
        } else {
          strcat(tmp_call, call_sign);
          for(tmp=strlen(call_sign); tmp<6; tmp++) strcat(tmp_call, " ");
        }
      }
      else if (ch_stat[i].state) {
        strip_call_log(call_sign, i);
        for(j=0; j<(int)strlen(call_sign); j++) {
          ch_str[0] = tolower(call_sign[j]);
          append_char(tmp_call, sizeof(tmp_call), ch_str[0]);
        }
        for(tmp=strlen(call_sign); tmp<6; tmp++) strcat(tmp_call, " ");
      }
      else {
        snprintf(ch_str, sizeof(ch_str), "%02d", i % 100);
        strcat(tmp_call, ch_str);
        strcat(tmp_call, ":===");
      }
    }
      for(j=0; j<(int)strlen(tmp_call); j++) {
      if(tmp_call[j] == '-') tmp_call[j] = '\0';
    while(strlen(tmp_call) < 6) strcat(tmp_call, " ");
    }
    {
      size_t start = strlen(tmpstr) - 1;
      size_t end = start + strlen(tmp_call);
      if (i < tnc_channels) {
        for (size_t col = start; col < end && col < (size_t)COLS; ++col)
          channel_at_column[col] = i;
      }
    }
    strcat(tmpstr,tmp_call);
  }
  for (i=strlen(tmpstr);i<COLS+1;i++) {
    tmpstr[i] = '*';
  }
  tmpstr[COLS+1] = '\0';
#endif
  win_stringout(tmpstr,&statlin,0);
  free(tmpstr);
}

#ifndef DPBOXT
void statlin_mouse(unsigned int button, unsigned int column, unsigned int row)
{
  int channel;
  if (button != 0 || column == 0 || column > (unsigned int)COLS ||
      row == 0 || row > (unsigned int)LINES ||
      !window_on_row(&statlin, (int)row - 1))
    return;
  channel = channel_at_column[column - 1];
  if (channel < 0 || channel >= tnc_channels) return;
  chan_flag = 0;
  sel_connect(channel);
  statlin_update();
}

void newdata(int channel)
{
  ch_stat[channel].not_disp = 1;
  statlin_update();
}

void sel_monitor(void)
{
  if (act_mode == M_MONITOR) {
    switch (old_mode) {
    case M_COMMAND:
      real_screen(cmd_layout);
      break;
    case M_CONNECT:
      real_screen(layout[act_channel]);
      break;
    case M_EXTMON:
      real_screen(xmon_layout[xmon_screen]);
      break;
    case M_HEARD:
      real_screen(heard_layout);
      break;
    case M_HELP:
      real_screen(help_layout);
      break;
    case M_BOXLIST:
#ifdef USE_IFACE
      if (bl_mode == M_MAILBOX) {
        real_screen(mbboxlist_layout);
      }
      else {
        real_screen(boxlist_layout[act_channel]);
      }
#else
      real_screen(boxlist_layout[act_channel]);
#endif
      break;
#ifdef USE_IFACE
    case M_MAILBOX:
      real_screen(mb_layout);
      break;
#endif
    }
    act_mode = old_mode;
  }
  else {
    old_mode = act_mode;
    real_screen(mon_layout);
    act_mode = M_MONITOR;
  }
  statlin_update();
}

void sel_command(void)
{
  if (act_mode == M_COMMAND) return;
  real_screen(cmd_layout);
  act_mode = M_COMMAND;
  statlin_update();
}

void sel_connect_force(int channel)
/* dg9ep 30.08.98 */
{
  if ( scrollhold_flag && (textwin[act_channel].num_lines > 0) )
    textwin[act_channel].holdflag = 1;

  real_screen(layout[channel]);
  act_mode = M_CONNECT;
  act_channel = channel;
  statlin_update();
}

void sel_connect(int channel)
{
  if ((channel == act_channel) && (act_mode == M_CONNECT)) return;
  sel_connect_force(channel);
}

void stat_display_org(int channel)
{
  int stat_len;
  int snd_frms;
  char tmp;
  char *stat_str;
  
  stat_str = (char *)malloc(COLS+2);
  strcpy(stat_str,"");
  strcat(stat_str,"\015**");
  if (channel == 0) {
    memset(stat_str + 3, '*', COLS - 2);
    stat_str[COLS+1] = '\0';
    memcpy(&stat_str[(COLS/2 - 9) + 1]," UNPROTO CHANNEL ",17);
  }
  else {
    strcat(stat_str,ax_state[ch_stat[channel].state & 0x0f]);
    strcat(stat_str,"**");
    if (ch_stat[channel].snd_frms) {
      strcat(stat_str," S=");
      snd_frms = ch_stat[channel].snd_frms + ch_stat[channel].snd_queue_frms;
      if (snd_frms > 9) {
        tmp = snd_frms/10+'0';
        append_char(stat_str, COLS + 2, tmp);
        tmp = snd_frms%10+'0';
        append_char(stat_str, COLS + 2, tmp);
        tmp = SPACE;
        append_char(stat_str, COLS + 2, tmp);
      }
      else {
        tmp = snd_frms+'0';
        append_char(stat_str, COLS + 2, tmp);
        strcat(stat_str," *");
      }
    }
    else strcat(stat_str,"******");
    strcat(stat_str,"**");
    if (ch_stat[channel].unacked) {
      strcat(stat_str," U=");
      tmp = ch_stat[channel].unacked+'0';
      append_char(stat_str, COLS + 2, tmp);
      tmp = SPACE;
      append_char(stat_str, COLS + 2, tmp);
    }
    else strcat(stat_str,"*****");
    strcat(stat_str,"**");
    if (ch_stat[channel].tries) {
      strcat(stat_str," R=");
      if (ch_stat[channel].tries > 9) {
        tmp = ch_stat[channel].tries/10+'0';
        append_char(stat_str, COLS + 2, tmp);
        tmp = ch_stat[channel].tries%10+'0';
        append_char(stat_str, COLS + 2, tmp);
        tmp = SPACE;
        append_char(stat_str, COLS + 2, tmp);
      }
      else {
        tmp = ch_stat[channel].tries+'0';
        append_char(stat_str, COLS + 2, tmp);
        strcat(stat_str," *");
      }
    }
    else strcat(stat_str,"******");
    strcat(stat_str,"** ");

    if (ch_stat[channel].disp_call[0] == '\0')
      append_limited(stat_str, COLS + 2, ch_stat[channel].call, 40);
    else
      append_limited(stat_str, COLS + 2, ch_stat[channel].disp_call, 40);
    if ((stat_len = strlen(stat_str)) < (COLS + 1)) {
      stat_str[stat_len++] = ' ';
      memset(stat_str + stat_len, '*', COLS + 1 - stat_len);
      stat_str[COLS+1] = '\0';
    }
  }
  win_stringout(stat_str,&statwin[channel],0);
  free(stat_str);
}

void stat_display_alt(int channel)
{
  int stat_len;
  int snd_frms;
  char tmpstring[256];
  char tmp;
  char *stat_str;
  
  stat_str = (char *)malloc(COLS+2);
  strcpy(stat_str,"");
  strcat(stat_str,"\015");
  if (channel == 0) {
    memset(stat_str + 1, '*', COLS);
    stat_str[COLS+1] = '\0';
    memcpy(&stat_str[(COLS/2 - 9) + 1]," UNPROTO CHANNEL ",17);
  }
  else
   {
    strcat(stat_str,"Ch:");            /* CH:*/
                                       /* ChannelNr einblenden */    
    tmp = channel/10+'0';              /* HighByte */
    append_char(stat_str, COLS + 2, tmp);
    
    tmp = channel%10+'0';              /* LowByte */
    append_char(stat_str, COLS + 2, tmp);
    
    strcat(stat_str," Stat:");          /* Stat: */ 
    strcat(stat_str,ax_state_alt[ch_stat[channel].state & 0x0f]);
             
    /* Wenn Name vorhanden, Name ausgeben */
    if (ch_stat[channel].name[0] != '\0')
      {
       append_limited(stat_str, COLS + 2, ch_stat[channel].name, 20);
       tmp = ':'; 
      append_char(stat_str, COLS + 2, tmp);
      }
    
    /*Distant call */
    strip_call_log(tmpstring,channel);
    if (strncmp(tmpstring,"CHANNEL",7) != 0)
       strcat(stat_str,tmpstring);
     append_spaces(stat_str, COLS + 2, 9 - (int)strlen(tmpstring));
    
    /* Leerzeichen einfuegen */
    if ((stat_len = strlen(stat_str)) < (COLS - 26))
     {
      append_spaces(stat_str, COLS + 2, COLS - 26 - stat_len);
                                                                                                                               }
                                                                                                                               
    
    
    /* MyCall */
    /* Wenn curcall mal staendig richtig gesetzt ist, kann man hier etwas
       einsparen. Eigentlich muesste ja noch auf Mycall=0 getestet werden
       um dann das Monitorcall anzuzeigen.*/
    strcat(stat_str,"My: ");
    if (ch_stat[channel].curcall[0] != '\0')
     {
      /* Currcall ausgeben */
      append_limited(stat_str, COLS + 2, ch_stat[channel].curcall, 9);
      append_spaces(stat_str, COLS + 2, 9 - (int)strlen(ch_stat[channel].curcall));
     }
     else
     {
      /* MyCall ausgeben */
      append_limited(stat_str, COLS + 2, ch_stat[channel].mycall, 9);
      append_spaces(stat_str, COLS + 2, 9 - (int)strlen(ch_stat[channel].mycall));
     }                      
     
    /* Linkstatus */  
    strcat(stat_str," AxSt:");
                                  
    /* Unsent */
    snd_frms = ch_stat[channel].snd_frms + ch_stat[channel].snd_queue_frms;
    /*High Nibble */
    tmp = snd_frms/10+'0';
    append_char(stat_str, COLS + 2, tmp);
    tmp = snd_frms%10+'0';
    append_char(stat_str, COLS + 2, tmp);
    tmp = '|';
    append_char(stat_str, COLS + 2, tmp);
    
    /* UnAck */
    tmp = ch_stat[channel].unacked/10+'0';
    append_char(stat_str, COLS + 2, tmp);
    tmp = ch_stat[channel].unacked%10+'0';
    append_char(stat_str, COLS + 2, tmp);
    tmp = '|';
    append_char(stat_str, COLS + 2, tmp);
    
    /* Retries */
    tmp = ch_stat[channel].tries/10+'0';
    append_char(stat_str, COLS + 2, tmp);
    tmp = ch_stat[channel].tries%10+'0';
    append_char(stat_str, COLS + 2, tmp);
    
  }
  win_stringout(stat_str,&statwin[channel],0);
  free(stat_str);
}

void stat_display(int channel)
{
  if (altstat)
    stat_display_alt(channel);
  else
    stat_display_org(channel);
}

static int output_screen(int channel)
{
#ifdef USE_IFACE
  return(ptyecho_flag || (!iface_active(channel) && !shell_active(channel)));
#else
  return(ptyecho_flag || (!shell_active(channel)));
#endif
}

void data_display(int channel, char *buffer)
{
  char tmpstring[280];
  
  strcpy(tmpstring,"\r*** ");
  strcat(tmpstring,buffer);
  strcat(tmpstring,"\r");
#ifdef HAVE_SOCKET
  sock_status_out(channel,tmpstring);
#endif
  if (output_screen(channel)) {
    if ((cbell_flag == 1) ||
        ((cbell_flag == 2) && ((act_channel != channel) ||
         (act_mode != M_CONNECT))))
      beep();
    win_stringout(tmpstring,&textwin[channel],1);
    newdata(channel);
  }
  write_file(channel,strlen(tmpstring),RX_RCV,tmpstring);
}

void rem_data_display(int channel, char *buffer)
{
  win_attrib(att_remote,&textwin[channel]);
  win_stringout(buffer,&textwin[channel],1);
  win_attrib(att_normal,&textwin[channel]);
}

void rem_data_display_buf(int channel, char *buffer, int len)
{
  win_attrib(att_remote,&textwin[channel]);
  win_textout_len(buffer,len,&textwin[channel],1);
  win_attrib(att_normal,&textwin[channel]);
}

void rem_stat_display(int channel, char *buffer)
{
  win_attrib(att_remote,&textwin[channel]);
  win_stringout(buffer,&textwin[channel],1);
  win_charout_cntl(C_DELLINE,&textwin[channel]);
  win_charout_cntl(C_STLINE,&textwin[channel]);
  win_attrib(att_normal,&textwin[channel]);
}

void handle_display_buffer(int channel,char **hlpbuffer,int *len)
{

  char buffer[513];
  int strlen;
  int len2;
  char *ptr;

#ifdef USE_IFACE
  /* handling when iface is active */
  if (iface_active(channel)) {
    write_iface(channel,*len,*hlpbuffer);
    if (output_screen(channel))
      win_textout_len(*hlpbuffer,*len,&textwin[channel],1);
    write_file(channel,*len,RX_RCV,*hlpbuffer);
    (*hlpbuffer) += *len;
    *len = 0;
    ch_stat[channel].oldbuflen = 0;
    ch_stat[channel].lastcr = 1;
    return;
  }
#endif

  /* handling when shell/socket is active */
  if (shell_active(channel)) {
    write_pty(channel,*len,*hlpbuffer);
    if (output_screen(channel))
      win_textout_len(*hlpbuffer,*len,&textwin[channel],1);
    write_file(channel,*len,RX_RCV,*hlpbuffer);
    (*hlpbuffer) += *len;
    *len = 0;
    ch_stat[channel].oldbuflen = 0;
    ch_stat[channel].lastcr = 1;
    return;
  }
  
  /* handling when autobin receive active */
  if (act_abin_rx(channel)) {
    len2 = *len;
    write_file_abin(channel,&len2,RX_RCV,*hlpbuffer);
    (*hlpbuffer) += len2;
    *len -= len2;
    ch_stat[channel].oldbuflen = 0;
    ch_stat[channel].lastcr = 1;
    return;
  }

  /* handling when YAPP receive is active */
  if (act_yapp_rx(channel)) {
    write_file_yapp(channel,*hlpbuffer,*len);
    (*hlpbuffer) += *len;
    *len = 0;
    ch_stat[channel].oldbuflen = 0;
    ch_stat[channel].lastcr = 1;
    return;
  }

  /* handling when YAPP transmit is active */
  if (act_yapp_tx(channel)) {
    ana_response_yapp(channel,*hlpbuffer,*len);
    (*hlpbuffer) += *len;
    *len = 0;
    ch_stat[channel].oldbuflen = 0;
    ch_stat[channel].lastcr = 1;
    return;
  }
  
  /* normal data handling */
  ptr = memchr(*hlpbuffer,CR,*len);
  if (ptr != NULL) {
    /* data up to CR to screen */
    strlen = (ptr + 1) - *hlpbuffer;
    insert_cr_tx(channel); 
    win_textout_len(*hlpbuffer,strlen,&textwin[channel],1);
    scan_pw_request(channel,*hlpbuffer,strlen);

    /* only if in a valid line */ 
    if (ch_stat[channel].lastcr) {
      if ((strlen + ch_stat[channel].oldbuflen) > 256) {
        if (ch_stat[channel].oldbuflen) {
          boxcut_nocr(channel,ch_stat[channel].oldbuf,
                      ch_stat[channel].oldbuflen);
          write_file(channel,ch_stat[channel].oldbuflen,RX_RCV,
                     ch_stat[channel].oldbuf);
        }
        ch_stat[channel].oldbuflen = 0;
        boxcut(channel,*hlpbuffer,strlen);
        write_file(channel,strlen,RX_RCV,*hlpbuffer);
      }
      else {
        /* build up a complete line in buffer */
        len2 = 0;
        if (ch_stat[channel].oldbuflen) {
          memcpy(buffer,ch_stat[channel].oldbuf,ch_stat[channel].oldbuflen);
          len2 += ch_stat[channel].oldbuflen;
          ch_stat[channel].oldbuflen = 0;
        }
        memcpy(buffer+len2,*hlpbuffer,strlen);
        len2 += strlen;
        buffer[len2] = '\0';
    
        if (len2 <= 256) {
          /* now handle line */
          if (ch_stat[channel].pwwait) {
            test_sysresponse(channel,buffer,len2);
            ch_stat[channel].pwwait--;
          }
          boxcut(channel,buffer,len2);
          connect_update(channel,buffer,len2);
          scan_login_received(channel,buffer,len2);
          if (!remote_input(channel,buffer,len2))
            write_file(channel,len2,RX_RCV,buffer);
        }
      }
    }
    else {
      memcpy(buffer,*hlpbuffer,strlen);
      buffer[strlen] = '\0';

      boxcut_rest(channel,buffer,strlen);
      write_file(channel,strlen,RX_RCV,buffer);
    }
    
    /* remove line from buffer */
    (*hlpbuffer) += strlen;
    *len -= strlen;
    ch_stat[channel].lastcr = 1;
    return;
  }
  else {
    /* data to screen */
    insert_cr_tx(channel); 
    win_textout_len(*hlpbuffer,*len,&textwin[channel],1);
    scan_pw_request(channel,*hlpbuffer,*len);

    if (check_autoyapp(channel,*hlpbuffer,*len)) {
      ch_stat[channel].oldbuflen = 0;
      ch_stat[channel].lastcr = 1;
    }
    else {
      if ((ch_stat[channel].lastcr) &&
          (*len + ch_stat[channel].oldbuflen <= 256)) {
          /* add to line in buffer */
          memcpy(ch_stat[channel].oldbuf+ch_stat[channel].oldbuflen,
                 *hlpbuffer,*len);
          ch_stat[channel].oldbuflen += *len;
          ch_stat[channel].lastcr = 1; /* still in valid line */
      }
      else {
        if (ch_stat[channel].oldbuflen) {
          boxcut_nocr(channel,ch_stat[channel].oldbuf,
                      ch_stat[channel].oldbuflen);
          write_file(channel,ch_stat[channel].oldbuflen,RX_RCV,
                     ch_stat[channel].oldbuf);
        }
        ch_stat[channel].oldbuflen = 0;
        ch_stat[channel].lastcr = 0;
        boxcut_nocr(channel,*hlpbuffer,*len);
        write_file(channel,*len,RX_RCV,*hlpbuffer);
      }
    }
    (*hlpbuffer) += *len;
    *len = 0;
    return;
  }
}

void data_display_len(int channel, char *buffer)
{
  int len;
  char *hlpbuffer;
  if (output_screen(channel)) {
    if ((infobell_flag == 1) ||
        ((infobell_flag == 2) && ((act_channel != channel) ||
         (act_mode != M_CONNECT))))
      if(!((rx_file[channel].type!=-1)&&(rxbell_flag==0)))
        beep();
    newdata(channel);
  }
  len = (*buffer)+1;
  hlpbuffer = buffer+1;
  while (len > 0) {
    handle_display_buffer(channel,&hlpbuffer,&len);
  }
}

void rem_data_display_len(int channel, char *buffer)
{
  win_attrib(att_remote,&textwin[channel]);
  win_textout(buffer,&textwin[channel],1);
  win_attrib(att_normal,&textwin[channel]);
}
#endif /* DPBOXT */

void cmd_display(int flag, int channel, char *buffer, int cr)
{
  int ans_flag;
  int ans_len;
  char ans_str[256];
  int con_channel;
  
  switch (flag & 0x0000FFFF) {
#ifdef DPBOXT
  case M_MAILBOX:
    mb_pre_display(buffer,cr);
    break;
  default:
    break;
  }
#else
  case M_CMDSCRIPT:
    return;
  case M_PUSHPOP:
    return;
  case M_CONNECT:
    if (tx_file_com.type == -1) {
      con_channel = channel;
    }
    else {
      con_channel = act_channel;
    } 
    win_stringout(buffer,&prewin[con_channel],0);
    if (cr) win_charout(CR,&prewin[con_channel],0);
    break;
#ifdef HAVE_SOCKET
  case M_SOCKET:
    out_cmd_socket(channel,buffer);
    break;
#endif
  case M_REMOTE_TEMP:
  case M_REMOTE:
    strcpy(ans_str,rem_tnt_str);
    strcat(ans_str,buffer);
    strcat(ans_str,rem_newlin_str);
    ans_len = strlen(ans_str);
    ans_flag = 0;
    rem_data_display(channel,ans_str);
    queue_cmd_data(channel,X_DATA,ans_len,ans_flag,ans_str);
    break;
  case M_INTERFACE:
    strcpy(ans_str,buffer);
    strcat(ans_str,rem_newlin_str);
    ans_len = strlen(ans_str);
    ans_flag = 0;
    rem_data_display(channel,ans_str);
    queue_cmd_data(channel,X_DATA,ans_len,ans_flag,ans_str);
    break;
  case M_EXTMON:
    xm_pre_display(channel,buffer,cr);
    break;
#ifdef USE_IFACE
  case M_MAILBOX:
    mb_pre_display(buffer,cr);
    break;
#endif
  case M_IFACECMD:
#ifndef DPBOXT
    send_tntresponse(channel,flag,buffer,cr);
#endif
    break; 
  default:
    win_stringout(buffer,&cmdwin,0);
    if (cr) win_charout(CR,&cmdwin,0);
    break;
  }
#endif
}

#ifndef DPBOXT  
void moni_display2(char *buffer, int attrib)
{
	
  switch (attrib) {
    case 1: win_attrib(att_monitor,&monwin); break;
    case 2: win_attrib(att_monport1,&monwin); break;
    case 3: win_attrib(att_monport2,&monwin); break;
  }
  win_stringout(buffer,&monwin,0);
  if (attrib)
    win_attrib(att_normal,&monwin);
  win_charout(CR,&monwin,0);
  write_monfile(strlen(buffer),buffer,1);
}

int get_port(char *buffer)
{
  char *ptr;
  int port;

  ptr = strchr(buffer, ':');
  if ((ptr != NULL) && ((ptr - buffer) == 1)) {
    port = *buffer - '0';
  }
  else {
    port = 0;
  }
	
  return(port);
}

static int monitor_is_iframe(const char *header)
{
  const char *p;
  if (strstr(header,"ctl I") != NULL)
    return 1;
  for (p = header; (p = strchr(p,' ')) != NULL; p++) {
    const char *token = p + 1;
    if (token[0] != 'I' || token[1] < '0' || token[1] > '7')
      continue;
    if (token[2] < '0' || token[2] > '7')
      continue;
    token += 3;
    if (*token == '^' || *token == 'v' || *token == '+' || *token == '-')
      token++;
    if (*token == ' ' || *token == '\0')
      return 1;
  }
  return 0;
}

void moni_display(int channel __attribute__((unused)), char *buffer)
{
  int port;
  
  layer3_frame = 0;
  layer3_inp = monitor_is_iframe(buffer);
  if (layer3_flag) {
    layer3_frame = (strstr(buffer,"pid CF") != NULL);
  }
  xmon_ch = analyse_mon_header(buffer);
  port = get_port(buffer);
  moni_display2(buffer, port + 1);
}


int check_bin(char *buffer)
{
  int len;
  int i;
  int a;
  int b;

  a = 0;
  b = 0;
  len = ((unsigned char)*buffer + 1);

  for (i = 1; i <= len; i++) {
    if (((unsigned char)buffer[i] > 128))
      b++;
    if ((buffer[i] == 'e') || (buffer[i] == 'E') || (buffer[i] == 'n') || (buffer[i] == ' ')) 
      a++;
  }
	
  return (b > a);
}	


void moni_display_len(int channel __attribute__((unused)), char *buffer)
{
  char decomp_buffer[257];
  char comp_info[MAXCHAR];
  int append_cr;
  char bin_info[30];
  int is_bin;

  if (xmon_ch != -1) {
    write_mon_info(buffer,xmon_ch);
  }
  else {
    analyse_mon_body(buffer);
  }
  if (layer3_frame) {
    if (!display_layer3(buffer,layer3_inp)) return;
  }
#ifdef USE_IFACE
  check_uireq(buffer);
#endif
#ifdef BCAST
  /* special display for broadcast-frames, including decoding */
  if (analyse_bcast_body(buffer)) return;
#endif
#ifdef USE_IFACE
  if (scanmbea_valid) check_mbeacon(buffer);
#endif

  /* DH3MB: Display compressed frames */
  if(moni_decomp && (decstathuf(buffer, decomp_buffer)==0))
  {
    sprintf(comp_info, _(">>> Hufmann-compressed (%0.1f %%)"),
            ((float)(buffer[0]+1)/(float)(decomp_buffer[0]+1)*100));
    moni_display(0,comp_info);
    buffer=decomp_buffer;
  }
  
  is_bin = moni_chkbin && check_bin(buffer);
  if (is_bin) {
    sprintf(bin_info, _("<%d byte binary data...>"), (*buffer + 1));
    win_textout_len(bin_info, strlen(bin_info), &monwin, 0);
    win_charout(CR, &monwin, 0);
  }
  else
    win_textout(buffer,&monwin,0);
  
  append_cr = 0;
  if (*(buffer + *buffer + 1) != CR) {
    if (!is_bin)
      win_charout(CR,&monwin,0);
    append_cr = 1;
  }
  write_monfile((*buffer + 1),buffer+1,append_cr);
}

void moni_display_len2(char *buffer, int len)
{
  int append_cr;
  char *buffer2;

  buffer2 = (char *)malloc(len+1);
  *buffer2 = (char)(len - 1);
  memcpy(buffer2 + 1,buffer,len);
  win_textout(buffer2,&monwin,0);
  append_cr = 0;
  if (*(buffer + len - 1) != CR) {
    win_charout(CR,&monwin,0);
    append_cr = 1;
  }
  write_monfile(len,buffer,append_cr);
  free(buffer2);
}

int pre_newline(int channel, char *str)
{
  int len;
  
  len = get_line(str,&prewin[channel]);
  win_charout(CR,&prewin[channel],0);
  return(len);
}

void pre_nextline(int channel)
{
  win_charout(CR,&prewin[channel],0);
}

void pre_charout(int channel, char ch)
{
  if (prewin[channel].column < input_linelen-1) {
    win_charout_cntl(ch,&prewin[channel]);
  }
  else {
    beep();
  }
}

void pre_charout_nobnd(int channel, char ch)
{
  win_charout_cntl(ch,&prewin[channel]);
}

void pre_charout_cntl(int channel, char ch)
{
  if (prewin[channel].column < input_linelen-1) {
    ch &= 0x1f;
    ch |= 0x40;
    win_attrib(att_controlchar,&prewin[channel]);
    win_charout_cntl(ch,&prewin[channel]);
    win_attrib(att_normal,&prewin[channel]);
  }
  else {
    beep();
  }
}

int pre_lineend(int channel)
{
  return(prewin[channel].column == input_linelen-1);
}

int pre_wordwrap(int channel, char *str)
{
  return(wordwrap(str,&prewin[channel]));
}

int cmd_newline(int channel __attribute__((unused)), char *str)
{
  int len;
  char tmpstr[MAXCOLS+1];
  
  len = get_line(tmpstr,&cmdwin);
  win_charout(CR,&cmdwin,0);
  if (tmpstr[0] == ':') {
    strcpy(str,tmpstr+1);
    len--;
  }
  else
    strcpy(str,tmpstr);
  return(len);
}

void cmd_charout(int channel __attribute__((unused)), char ch)
{
  if (cmdwin.column < input_linelen-1) {
    win_charout_cntl(ch,&cmdwin);
  }
  else {
    beep();
  }
}

void cmd_charout_nobnd(int channel __attribute__((unused)), char ch)
{
  win_charout_cntl(ch,&cmdwin);
}
#endif /* DPBOXT */

int init_screen(void)
{
  int i;
  int j;

#ifdef DPBOXT
  tabexp_flag = 1;
  window_init();
  open_window(1,&statlin,1);
  act_channel = 0;
  act_mode = M_MAILBOX;
  old_mode = M_MAILBOX;
  win_attrib(att_statline,&statlin);
  statlin_update();
  return(0);
}
#else
  tabexp_flag = 1;
  infobell_flag = 0;
  cbell_flag = 1;
  rxbell_flag = 0;
  layer3_flag = 1;  
  if ((input_linelen > COLS) || (input_linelen < MAXCHAR)) input_linelen = COLS;
  for (j = 0; j < LAYOUTPARTS; j++) {
    cmd_layout[j].win = NULL;
    cmd_layout[j].first_real_line = 0;
    cmd_layout[j].win_num_lines = 0;
    cmd_layout[j].pagesize = 0;
    mon_layout[j].win = NULL;
    mon_layout[j].first_real_line = 0;
    mon_layout[j].win_num_lines = 0;
    mon_layout[j].pagesize = 0;
  }
  for (i = 0; i < tnc_channels; i++) {
    for (j = 0; j < LAYOUTPARTS; j++) {
      layout[i][j].win = NULL;
      layout[i][j].first_real_line = 0;
      layout[i][j].win_num_lines = 0;
      layout[i][j].pagesize = 0;
    }
  }
  window_init();
  open_window(1,&statlin,1);
  open_window(lines_command,&cmdwin,1);
  cmd_layout[0].win = &cmdwin;
  cmd_layout[0].first_real_line = 0;
  cmd_layout[0].win_num_lines = LINES-1;
  cmd_layout[0].pagesize = LINES-4;
  cmd_layout[1].win = &statlin;
  cmd_layout[1].first_real_line = LINES-1;
  cmd_layout[1].win_num_lines = 1;
  cmd_layout[1].pagesize = 0;
  open_window(lines_monitor,&monwin,1);
  mon_layout[0].win = &monwin;
  mon_layout[0].first_real_line = 0;
  mon_layout[0].win_num_lines = LINES-1;
  mon_layout[0].pagesize = LINES-4;
  mon_layout[1].win = &statlin;
  mon_layout[1].first_real_line = LINES-1;
  mon_layout[1].win_num_lines = 1;
  mon_layout[1].pagesize = 0;
  if ((((LINES-lines_moncon)/scr_divide-1) < 2) ||
      (scr_divide < 2) || (scr_divide > 40)) scr_divide = 3;
  for (i = 0; i < tnc_channels; i++) {
    if (i && (i < r_channels)) { /* full windowsize */
      open_window(lines_input,&prewin[i],1);
      open_window(lines_output,&textwin[i],1);
    }
    else { /* reduced windowsize */
      open_window(lines_r_input,&prewin[i],1);
      open_window(lines_r_output,&textwin[i],1);
    }
    open_window(1,&statwin[i],1);
    win_attrib(att_cstatline,&statwin[i]);
    layout[i][0].win = &prewin[i];
    layout[i][0].first_real_line = 0;
    layout[i][0].win_num_lines = (LINES-lines_moncon)/scr_divide-1;
    layout[i][0].pagesize = (LINES-lines_moncon)/scr_divide-2;
    layout[i][1].win = &statwin[i];
    layout[i][1].first_real_line = (LINES-lines_moncon)/scr_divide-1;
    layout[i][1].win_num_lines = 1;
    layout[i][1].pagesize = 0;
    layout[i][2].win = &textwin[i];
    layout[i][2].first_real_line = (LINES-lines_moncon)/scr_divide;
    layout[i][2].win_num_lines =
      (LINES-lines_moncon)-(LINES-lines_moncon)/scr_divide-1;
    layout[i][2].pagesize =
      (LINES-lines_moncon)-(LINES-lines_moncon)/scr_divide-4;
    layout[i][3].win = &statlin;
    layout[i][3].first_real_line = (LINES-lines_moncon)-1;
    layout[i][3].win_num_lines = 1;
    layout[i][3].pagesize = 0;
    if (lines_moncon) {
      layout[i][4].win = &monwin;
      layout[i][4].first_real_line = (LINES-lines_moncon);
      layout[i][4].win_num_lines = lines_moncon;
      layout[i][4].pagesize = lines_moncon - 1;
    }
  }
  init_scrollback();
  real_screen(cmd_layout);
  act_mode = M_COMMAND;
  old_mode = M_COMMAND;
  act_channel = 1;
  xmon_ch = -1;
  layer3_frame = 0;
  win_attrib(att_statline,&statlin);
  for (i = 0; i < tnc_channels;i++) {
    ch_stat[i].not_disp = 0;
  }
  statlin_update();
  if (!is_root) {
    cmd_display(M_COMMAND,1,signon,0);
    cmd_display(M_COMMAND,1,add_signon,1);
  }
  else {
    cmd_display(M_COMMAND,1,signon,1);
  }
  return(0);
}
#endif

void reinit_help(void);
void reinit_screen(void);

void resize_screen(void)
{
#ifndef DPBOXT
  int i;
#endif

  if (!resize_terminal()) return;
#ifndef DPBOXT
  for (i = 0; i < tnc_channels; i++)
    stat_display(i);
#endif
  reinit_screen();
  statlin_update();
}

void reinit_screen(void)
{
  int i;
  int j;
#ifndef DPBOXT
  int monitor_lines = lines_moncon;
  int divide = scr_divide;
  if (monitor_lines > LINES - 10) monitor_lines = LINES - 10;
#endif

#ifdef DPBOXT
  if (tnt_daemon && input_linelen > COLS) input_linelen = COLS;
  reinit_help();
  reinit_mb();
  reinit_blist();

  switch (act_mode) {
  case M_MAILBOX:  
    real_screen(mb_layout);
    break;
  case M_BOXLIST:
    real_screen(mbboxlist_layout);
    break;
  case M_HELP:  
    real_screen(help_layout);
    break;
  }
#else
  if (tnt_daemon && input_linelen > COLS) input_linelen = COLS;
  for (j = 0; j < LAYOUTPARTS; j++) {
    cmd_layout[j].win = NULL;
    cmd_layout[j].first_real_line = 0;
    cmd_layout[j].win_num_lines = 0;
    cmd_layout[j].pagesize = 0;
    mon_layout[j].win = NULL;
    mon_layout[j].first_real_line = 0;
    mon_layout[j].win_num_lines = 0;
    mon_layout[j].pagesize = 0;
  }
  for (i = 0; i < tnc_channels; i++) {
    for (j = 0; j < LAYOUTPARTS; j++) {
      layout[i][j].win = NULL;
      layout[i][j].first_real_line = 0;
      layout[i][j].win_num_lines = 0;
      layout[i][j].pagesize = 0;
    }
  }
  cmd_layout[0].win = &cmdwin;
  cmd_layout[0].first_real_line = 0;
  cmd_layout[0].win_num_lines = LINES-1;
  cmd_layout[0].pagesize = LINES-4;
  cmd_layout[1].win = &statlin;
  cmd_layout[1].first_real_line = LINES-1;
  cmd_layout[1].win_num_lines = 1;
  cmd_layout[1].pagesize = 0;

  mon_layout[0].win = &monwin;
  mon_layout[0].first_real_line = 0;
  mon_layout[0].win_num_lines = LINES-1;
  mon_layout[0].pagesize = LINES-4;
  mon_layout[1].win = &statlin;
  mon_layout[1].first_real_line = LINES-1;
  mon_layout[1].win_num_lines = 1;
  mon_layout[1].pagesize = 0;

  if (divide < 2 || divide > 40 ||
      (LINES-monitor_lines)/divide < 3) divide = 3;
  for (i = 0; i < tnc_channels; i++) {
    layout[i][0].win = &prewin[i];
    layout[i][0].first_real_line = 0;
    layout[i][0].win_num_lines = (LINES-monitor_lines)/divide-1;
    layout[i][0].pagesize = (LINES-monitor_lines)/divide-2;
    layout[i][1].win = &statwin[i];
    layout[i][1].first_real_line = (LINES-monitor_lines)/divide-1;
    layout[i][1].win_num_lines = 1;
    layout[i][1].pagesize = 0;
    layout[i][2].win = &textwin[i];
    layout[i][2].first_real_line = (LINES-monitor_lines)/divide;
    layout[i][2].win_num_lines =
      (LINES-monitor_lines)-(LINES-monitor_lines)/divide-1;
    layout[i][2].pagesize =
      (LINES-monitor_lines)-(LINES-monitor_lines)/divide-4;
    layout[i][3].win = &statlin;
    layout[i][3].first_real_line = (LINES-monitor_lines)-1;
    layout[i][3].win_num_lines = 1;
    layout[i][3].pagesize = 0;
    if (monitor_lines) {
      layout[i][4].win = &monwin;
      layout[i][4].first_real_line = (LINES-monitor_lines);
      layout[i][4].win_num_lines = monitor_lines;
      layout[i][4].pagesize = monitor_lines - 1;
    }
  }
  reinit_help();
#ifdef USE_IFACE
  reinit_mb();
#endif
  reinit_blist();
  reinit_xmon();
  reinit_mheard();  

  switch (act_mode) {
  case M_COMMAND:  
    real_screen(cmd_layout);
    break;
  case M_MONITOR:  
    real_screen(mon_layout);
    break;
  case M_CONNECT:  
    real_screen(layout[act_channel]);
    break;
  case M_EXTMON:  
    real_screen(xmon_layout[xmon_screen]);
    break;
#ifdef USE_IFACE
  case M_MAILBOX:  
    real_screen(mb_layout);
    break;
#endif
  case M_BOXLIST:
#ifdef USE_IFACE
    if (bl_mode == M_MAILBOX)  
      real_screen(mbboxlist_layout);
    else
      real_screen(boxlist_layout[act_channel]);
#else
    real_screen(boxlist_layout[act_channel]);
#endif
    break;
  case M_HEARD:  
    real_screen(heard_layout);
    break;
  case M_HELP:  
    real_screen(help_layout);
    break;
  }
#endif
}

void exit_screen(void)
{
  int i;

#ifdef DPBOXT
  close_window(&statlin);
#else  
  exit_scrollback();
  close_window(&cmdwin);
  close_window(&monwin);
  close_window(&statlin);
  for (i = 0; i < tnc_channels; i++) {
    close_window(&prewin[i]);
    close_window(&statwin[i]);
    close_window(&textwin[i]);
  }
#endif  
  window_exit();
}

/* display line on help-screen */
static void help_textout_len(char *buf, int len)
{
  win_textout_len(buf,len,&helpwin,1);
}

/* character to help-screen */
void help_charout_cntl(char ch)
{
  win_charout_cntl(ch,&helpwin);
}

/* select help screen */
void sel_help(void)
{
  if ((!help_available) || (act_mode == M_HELP)) return;
  real_screen(help_layout);
  act_mode = M_HELP;
  statlin_update();
}

#define BUFSIZE 512

/* init of help-screen */
void init_help(void)
{
  char helpfile[160];
  int helpfd;
  int end;
  int len2;
  char buf[BUFSIZE];
  int i;
  int j;
  int lines;
  char *linebuf;
  int linelen;

  
  help_available = 0;
  strcpy(helpfile,tnt_help_file);
  helpfd = open(helpfile,O_RDONLY); 
  if (helpfd == -1) {
    /* file does not exist */
    return;
  }
  
  /* counting number of LFs in file to get number of lines */
  lines = 0;
  end = 1;
  while (end) {
    len2 = read(helpfd,buf,BUFSIZE);
    for (i=0;i<len2;i++) {
      if (buf[i] == '\n') lines++;
    }
    if (len2 < BUFSIZE) end = 0;
  }
  lines += 1; /* just to be sure... */

  for (j = 0; j < LAYOUTPARTS; j++) {
    help_layout[j].win = NULL;
    help_layout[j].first_real_line = 0;
    help_layout[j].win_num_lines = 0;
    help_layout[j].pagesize = 0;
  }
  open_window(lines,&helpwin,0);
  help_layout[0].win = &helpwin;
  help_layout[0].first_real_line = 0;
  help_layout[0].win_num_lines = LINES-1;
  help_layout[0].pagesize = LINES-4;
  help_layout[1].win = &statlin;
  help_layout[1].first_real_line = LINES-1;
  help_layout[1].win_num_lines = 1;
  help_layout[1].pagesize = 0;
  
  /* seek to file begin */
  lseek(helpfd,0,SEEK_SET);
  linelen = 0;
  end = 1;
  linebuf = (char *)malloc(COLS+2);
  while (end) {
    len2 = read(helpfd,buf,BUFSIZE);
    for (i=0;i<len2;i++) {
      if (buf[i] == '\n') {
        linebuf[linelen] = '\r';
        linelen++;
        help_textout_len(linebuf,linelen);
        linelen = 0;
      }
      else {
        if (linelen < (COLS-1)) {
          linebuf[linelen] = buf[i];
          linelen++;
        }
      }
    }
    if (len2 < BUFSIZE) end = 0;
  }
  if (linelen) {
    linebuf[linelen] = '\r';
    linelen++;
    help_textout_len(linebuf,linelen);
    linelen = 0;
  }
  close(helpfd);
  free(linebuf);
  helpwin.line = 0;
  if ((helpwin.phys_line = helpwin.line + helpwin.line_offset) >=
       helpwin.num_lines)
    helpwin.phys_line -= helpwin.num_lines;
  helpwin.column = 0;
  help_available = 1;
}

void reinit_help(void)
{
  int j;
  
  if (help_available) {
    for (j = 0; j < LAYOUTPARTS; j++) {
      help_layout[j].win = NULL;
      help_layout[j].first_real_line = 0;
      help_layout[j].win_num_lines = 0;
      help_layout[j].pagesize = 0;
    }
    help_layout[0].win = &helpwin;
    help_layout[0].first_real_line = 0;
    help_layout[0].win_num_lines = LINES-1;
    help_layout[0].pagesize = LINES-4;
    help_layout[1].win = &statlin;
    help_layout[1].first_real_line = LINES-1;
    help_layout[1].win_num_lines = 1;
    help_layout[1].pagesize = 0;
  }
}

/* exit of help-screen */
void exit_help(void)
{
  if (help_available) {
    close_window(&helpwin);
  }
}

#ifndef DPBOXT
void free_display(void)
{
  free(prewin);
  free(statwin);
  free(textwin);
}

int alloc_display(void)
{
  prewin = (struct window *)malloc(tnc_channels * sizeof(struct window));
  statwin = (struct window *)malloc(tnc_channels * sizeof(struct window));
  textwin = (struct window *)malloc(tnc_channels * sizeof(struct window));
  layout = (struct real_layout (*) [LAYOUTPARTS])
    malloc(tnc_channels * sizeof(struct real_layout [LAYOUTPARTS]));
  return((prewin == NULL) ||
         (statwin == NULL) ||
         (textwin == NULL) ||
         (layout == NULL));
}
#endif

void
write_last_n_lines(int channel, int nLines , FILE *f)
  /* write last nLines lines of Winbuffer to file
   * quoting characters are inserted
   * dg9ep 8/99 */
{
  int i, startline;
  char *ptr;
  struct window *win;
  char sRes[MAXCOLS+2];
  char *pcRes;
  char *pcLastChar;

  win = &textwin[channel];
  if( win->phys_line <= nLines )
     nLines = win->phys_line;
  startline = win->phys_line-nLines;
  ptr = win->window_buffer + startline * 2 * MAXCOLS;
  while (nLines-- > 0 ) {
      sRes[0] = '\0';
      pcRes = pcLastChar = sRes;
      for (i = 0; i < MAXCOLS; i++) {
       ptr++; /* skip attributbyte */
       if( *ptr != ' ' ) /* store pointer to last nonspace-char */
          pcLastChar = pcRes;
       *pcRes++ = *ptr++;
      }
      /* remove trailing spaces */
      *(++pcLastChar) = '\0';

      if( strlen(sRes) == 0 )
        fprintf(f,"\n");
      else
        fprintf(f,"> %s\n", sRes);
  }
}
