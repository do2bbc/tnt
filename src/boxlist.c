/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993-1996 by Mark Wahl
   For license details see documentation
   Procedures for boxlist utility (boxlist.c)
   created: Mark Wahl DL4YBG 94/10/03
   updated: Mark Wahl DL4YBG 96/05/27
   updated: Matthias Hensler WS1LS 99/09/08
*/

#include "tnt.h"
#include "boxlist.h"

/* external function declarations */
extern void beep();
extern void check_bid();
extern void close_file();
extern int close_window();
extern void cmd_actiface();
extern void cmd_display();
extern void dat_input();
extern int get_line_noconv();
extern int iface_active();
extern void insert_cr_rx();
extern void mb_charout();
extern void mb_input();
extern void mb_nextline();
extern void open_logfile();
extern int open_window();
extern void pre_charout();
extern void pre_nextline();
extern void real_screen();
extern void real_screen_hold();
extern void rem_data_display_buf();
extern void rem_mb_display_buf();
extern void sel_mailbox();
extern void statlin_update();
extern void win_attrib();
extern void win_charout_cntl();
extern void win_textout_len();
extern void write_iface();

extern int act_mode;
extern int act_channel;
extern int tnc_channels;
extern char download_dir[];
extern char tnt_blcmdfile[];
extern char box_socket[];
extern int LINES,COLS;
extern struct window statlin;
extern int txecho_flag;
extern char att_monitor;
extern char att_normal;
extern char att_special;

#if defined(DPBOXT) || defined(USE_IFACE)
extern struct rx_file mb_file;
extern struct real_layout mb_layout[LAYOUTPARTS];
#endif

#ifndef DPBOXT
extern struct rx_file *rx_file;
extern struct real_layout (*layout)[LAYOUTPARTS];
#endif

#if defined(DPBOXT) || defined(USE_IFACE)
struct boxlist_file mbboxlist_file;
static struct window mbboxlistwin;
struct real_layout mbboxlist_layout[LAYOUTPARTS];
int bl_mode;
extern struct rx_file mb_file;

/* data for save of boxlist windows */
#define BL_SAVEWIN 10
static struct window blistsavewin[BL_SAVEWIN];
static char blistsavewin_board[BL_SAVEWIN][9];
static int next_blist_save;
#endif

int blist_add_plus;
#ifndef DPBOXT
struct boxlist_file *boxlist_file;
static struct window *boxlistwin;
struct real_layout (*boxlist_layout)[LAYOUTPARTS];
#ifdef USE_IFACE
extern int box_active_flag;
extern int box_busy_flag;
#endif
#endif

/* structures for boxlist commands */
struct blcmdstr {
  char command[80];
  struct blcmdstr *next;
};

#define BLAPPL_MAILBOX 1
#define BLAPPL_CONNECT 2
#define BLAPPL_BOTH (BLAPPL_MAILBOX | BLAPPL_CONNECT)

struct blcmdlist {
  char key;
  int appl_flag;
  int boxline_type;
  int allowed;
  struct blcmdstr *blcmd;
  struct blcmdlist *next;
};

static struct blcmdlist *blcmdlist_root;

/* strings for scanning for board */
static char *board_string[] = {
  "Info-File: ",
  "User-File: ",
  ""
};

/* result mode for 'boxline_analysis' */
struct boxline_result {
  int type;
  int msg_nr;
  char call[7];
  char board[9];
  int number;
  char date[9];
  char time[6];
  char bid[13];
  char mbx[7];
  int bytes;
  int lt;
  char title[MAXCOLS+1];
  int start_col;
  int num_col;
};

/* results of 'type' in 'struct boxline_result' */
#define BOXLINE_ALL -1
#define BOXLINE_UNKNOWN 0
#define BOXLINE_CHECK 1
#define BOXLINE_RUNC 2
#define BOXLINE_LIST 3
#define BOXLINE_DIR 4

static char global_board[9];

static struct tmpname_entry *tmpname_root;

static void blist_analyse_line(int channel,char key,int flag);

void cmd_xblist(int par1,int par2,int channel,int len,int mode, char *str);

/* load boxlist commands file */
static void load_blcmdfile()
{
  int file_end;
  int file_corrupt;
  char file_str[160];
  char line[83];
  FILE *blcmd_file_fp;
  char *ptr;
  struct blcmdlist *blcmdlist_wrk;
  struct blcmdlist *blcmdlist_cur;
  struct blcmdstr *blcmdstr_wrk;
  struct blcmdstr *blcmdstr_cur;
  int key;
  int appl_mode;
  int boxline_type;
  int append;
  int allowed;

  strcpy(file_str,tnt_blcmdfile);
  if (!(blcmd_file_fp = fopen(file_str,"r"))) {
    /* no file present, exit */
    return;
  }
  file_end = 0;
  file_corrupt = 0;   
  blcmdlist_cur = NULL;
  while ((!file_end) && (!file_corrupt)) {  
    if (fgets(line,82,blcmd_file_fp) == NULL) {
      file_end = 1;   
    }
    else {
      if (strlen(line) == 82) {
        file_corrupt = 1;
      }
      else {
        if (line[0] != '#') {
          ptr = strchr(line,'\n');
          if (ptr != NULL) *ptr = '\0';
          if (strlen(line) < 6) {
            file_corrupt = 1;
            continue;
          }
          if (((line[2] != '#') && (line[2] != '^')) || 
              ((line [4] != '#') && (line[4] != '!'))) {
            file_corrupt = 1;
            continue;
          }
          appl_mode = -1;
          append = 0;
          switch (toupper(line[0])) {
          case 'M':
            appl_mode = BLAPPL_MAILBOX;
            break;
          case 'C':
            appl_mode = BLAPPL_CONNECT;
            break;
          case '*':
            appl_mode = BLAPPL_BOTH;
            break;
          case 'X':
            append = 1;
            break;
          }
          if ((appl_mode == -1) && (append == 0)) {
            file_corrupt = 1;
            continue;
          }
          if ((append) && (blcmdlist_cur == NULL)){
            file_corrupt = 1;
            continue;
          }
          boxline_type = BOXLINE_UNKNOWN;
          switch (toupper(line[1])) {
          case '*':
            boxline_type = BOXLINE_ALL;
            break;
          case 'C':
            boxline_type = BOXLINE_CHECK;
            break;
          case 'R':
            boxline_type = BOXLINE_RUNC;
            break;
          case 'L':
            boxline_type = BOXLINE_LIST;
            break;
          case 'D':
            boxline_type = BOXLINE_DIR;
            break;
          }
          if (boxline_type == BOXLINE_UNKNOWN){
            file_corrupt = 1;
            continue;
          }
          key = toupper(line[3]);
          if (line[2] == '^') {
            key &= 0x1f;
          }
          if ((append) && (blcmdlist_cur->key != key)){
            file_corrupt = 1;
            continue;
          }
          allowed = (line[4] == '!');

          if (!append) {
            blcmdlist_wrk =
              (struct blcmdlist *)malloc(sizeof(struct blcmdlist));
            blcmdlist_wrk->key = key;
            blcmdlist_wrk->appl_flag = appl_mode;
            blcmdlist_wrk->boxline_type = boxline_type;
            blcmdlist_wrk->allowed = allowed;

            blcmdstr_wrk =
              (struct blcmdstr *)malloc(sizeof(struct blcmdstr));
            strcpy(blcmdstr_wrk->command,&line[5]);
            blcmdstr_wrk->next = NULL;
            blcmdlist_wrk->blcmd = blcmdstr_wrk;
            
            blcmdlist_wrk->next = NULL;
            if (blcmdlist_root == NULL) {
              blcmdlist_root = blcmdlist_wrk;
              blcmdlist_cur = blcmdlist_wrk;
            }
            else {
              blcmdlist_cur->next = blcmdlist_wrk;
              blcmdlist_cur = blcmdlist_wrk;
            }
          }
          else {
            blcmdstr_wrk =
              (struct blcmdstr *)malloc(sizeof(struct blcmdstr));
            strcpy(blcmdstr_wrk->command,&line[5]);
            blcmdstr_wrk->next = NULL;
            blcmdstr_cur = blcmdlist_wrk->blcmd;
            while (blcmdstr_cur->next != NULL) {
              blcmdstr_cur = blcmdstr_cur->next;
            }
            blcmdstr_cur->next = blcmdstr_wrk;
          }
        }
      }
    }
  }
  fclose(blcmd_file_fp);
  if (file_corrupt) { 
    if (line == NULL) line[0] = '\0';
    cmd_display(M_COMMAND,0,
      _("WARNING: blcmdfile is in wrong format, wrong line:"),1);
    cmd_display(M_COMMAND,0,line,1);
    return;
  }
}

static void clear_blcmdfile()
{
  struct blcmdlist *blcmdlist_wrk;
  struct blcmdlist *blcmdlist_tmp;
  struct blcmdstr *blcmdstr_wrk;
  struct blcmdstr *blcmdstr_tmp;

  blcmdlist_wrk = blcmdlist_root;
  while (blcmdlist_wrk != NULL) {
    blcmdlist_tmp = blcmdlist_wrk;
    blcmdlist_wrk = blcmdlist_tmp->next;
    blcmdstr_wrk = blcmdlist_tmp->blcmd;
    while (blcmdstr_wrk != NULL) {
      blcmdstr_tmp = blcmdstr_wrk;
      blcmdstr_wrk = blcmdstr_tmp->next;
      free(blcmdstr_tmp);
    }
    blcmdlist_tmp->blcmd = NULL;
    free(blcmdlist_tmp);
  }
  blcmdlist_root = NULL;
}

/* reload boxlist command file */
void cmd_ldblcmd(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  clear_blcmdfile();
  load_blcmdfile();
  cmd_display(mode,channel,OK_TEXT,1);
  return;
}

/* list boxlist command data */
void cmd_lsblcmd(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  char disp_line[83]; 
  int found;
  struct blcmdlist *blcmdlist_wrk;
  struct blcmdstr *blcmdstr_wrk;
  char appl_char;
  char boxline_char;
  char key;
  char keyflag;
  char allowed;

  found = 0;
  blcmdlist_wrk = blcmdlist_root;
  while (blcmdlist_wrk != NULL) {
    found = 1;
    appl_char = '?';
    switch (blcmdlist_wrk->appl_flag) {
    case BLAPPL_BOTH:
      appl_char = '*';
      break;
    case BLAPPL_MAILBOX:
      appl_char = 'M';
      break;
    case BLAPPL_CONNECT:
      appl_char = 'C';
      break;
    }
    boxline_char = '?';
    switch (blcmdlist_wrk->boxline_type) {
    case BOXLINE_ALL:
      boxline_char = '*';
      break;
    case BOXLINE_CHECK:
      boxline_char = 'C';
      break;
    case BOXLINE_RUNC:
      boxline_char = 'R';
      break;
    case BOXLINE_LIST:
      boxline_char = 'L';
      break;
    case BOXLINE_DIR:
      boxline_char = 'D';
      break;
    }
    keyflag = '#';
    key = blcmdlist_wrk->key;
    if (key < 0x20) {
      key |= 0x40;
      keyflag = '^';
    }
    allowed = '#';
    if (blcmdlist_wrk->allowed) allowed = '!';
    sprintf(disp_line,"%c%c%c%c%c%s",appl_char,boxline_char,keyflag,key,
            allowed,blcmdlist_wrk->blcmd->command);
    cmd_display(mode,channel,disp_line,1);
    blcmdstr_wrk = blcmdlist_wrk->blcmd->next;
    while (blcmdstr_wrk != NULL) {
      sprintf(disp_line,"%c%c%c%c%c%s",'X',boxline_char,keyflag,key,
              allowed,blcmdstr_wrk->command);
      cmd_display(mode,channel,disp_line,1);
      blcmdstr_wrk = blcmdstr_wrk->next;
    }
    blcmdlist_wrk = blcmdlist_wrk->next;
  }
  if (!found) {
    cmd_display(mode,channel, _("No lines found"),1);
    return;
  }
}

static int extract_board_number(char *file, char *board, int *number)
{

  int i;
  int res;
  int line_ok;
  char *str;

  line_ok = 1;
  str = strdup(file);  
  for (i=0;i<strlen(str);i++) {
    if (str[i] == '.') str[i] = ' ';
  }
  res = sscanf(str,"%s %d",board,number);
  if (res != 2) {
    if (strlen(str) > 8) {
      strncpy(board,str,8);
      board[8] = '\0';
      res = sscanf(str+8,"%d",number);
      if (res != 1) line_ok = 0;
    }
    else line_ok = 0;
  }
  else {
    if (strlen(board) > 8) line_ok = 0;
  }
  free(str);
  return(line_ok);
}

static int boxline_analysis(char *str, struct boxline_result *boxline_ptr,
                            int column)
{
  char call[MAXCOLS+1];
  char file[MAXCOLS+1];
  char date[MAXCOLS+1];
  char time[MAXCOLS+1];
  char bid[MAXCOLS+1];
  char mbx[MAXCOLS+1];
  char title[MAXCOLS+1];
  char rubrik[MAXCOLS+1];
  int check_nr;
  int bytes;
  int lt;
  int number;
  int i;
  int res;
  int line_ok;
  struct dir_entry {
    char str[MAXCOLS+1];
    char board[MAXCOLS+1];
    int start_col;
    int end_col;
  } dir[6];
  int pos;
  int first;

/* DIEBOX checklist without or with BID

    7 DL4BCU > TERMINE...16 24.09.94 DL      2214   5 2m Mobilfuchsjagd I05 08.
   85 DH3FBI > KENWOOD..423 055514DB0GV  DL   851   5 LF & VLF Empfang mit TS-5
   
*/  

  res = sscanf(str,"%d %s > %s %s %s %d %d %s",
               &check_nr, call, file, date, mbx, &bytes, &lt, title);
  line_ok = (res == 8);
  if (!line_ok) {
    /* special scan if mbx is empty */
    res = sscanf(str,"%d %s > %s %s %d %d %s",
               &check_nr, call, file, date, &bytes, &lt, title);
    if (res == 7) {
      mbx[0] = '\0';
      line_ok = 1;
    }
  }
  if (line_ok) {
    line_ok = extract_board_number(file,rubrik,&number);
  }
  if (line_ok) {
    /* check if bid or date */
    if (!((date[2] == '.') && (date[5] == '.'))) {
      strcpy(bid,date);
      date[0] = '\0';
    }
    else {
      bid[0] = '\0';
    }

    boxline_ptr->type = BOXLINE_CHECK;
    boxline_ptr->msg_nr = check_nr;
    strncpy(boxline_ptr->call,call,6);
    boxline_ptr->call[6] = '\0';
    strncpy(boxline_ptr->board,rubrik,8);
    boxline_ptr->board[8] = '\0';
    boxline_ptr->number = number;
    strncpy(boxline_ptr->date,date,8);
    boxline_ptr->date[8] = '\0';
    boxline_ptr->time[0] = '\0';
    strncpy(boxline_ptr->bid,bid,12);
    boxline_ptr->bid[12] = '\0';
    strncpy(boxline_ptr->mbx,mbx,8);
    boxline_ptr->mbx[8] = '\0';
    boxline_ptr->bytes = bytes;
    boxline_ptr->lt = lt;
    strncpy(boxline_ptr->mbx,title,MAXCOLS);
    boxline_ptr->title[MAXCOLS] = '\0';
    boxline_ptr->start_col = 0;
    boxline_ptr->num_col = COLS - 1;
    return(1);
  }


/* RUN C mit option D=CRD$@L

DG0XC  DIGI......17 28.04.95 2845DB0BALWE DL      1 DB0BRO-1 wieder ok.

*/

  res = sscanf(str,"%s %s %s %s %s %d %s",
               call, file, date, bid, mbx, &bytes, title);
  line_ok = (res == 7);             
  if (!line_ok) {
    /* special scan if mbx is empty */
    res = sscanf(str,"%s %s %s %s %d %s",
                 call, file, date, bid, &bytes, title);
    if (res == 6) {
      mbx[0] = '\0';
      line_ok = 1;
    }
  }
  if (line_ok) {
    /* if call contains only numbers, it is a DIEBOX-list */
    line_ok = 0;
    for (i=0;i<strlen(call);i++) {
      if (isalpha(call[i])) line_ok = 1;
    }
  }
  if (line_ok) {
    line_ok = extract_board_number(file,rubrik,&number);
  }

  if (line_ok) {
    boxline_ptr->type = BOXLINE_RUNC;
    boxline_ptr->msg_nr = -1;
    strncpy(boxline_ptr->call,call,6);
    boxline_ptr->call[6] = '\0';
    strncpy(boxline_ptr->board,rubrik,8);
    boxline_ptr->board[8] = '\0';
    boxline_ptr->number = number;
    strncpy(boxline_ptr->date,date,8);
    boxline_ptr->date[8] = '\0';
    boxline_ptr->time[0] = '\0';
    strncpy(boxline_ptr->bid,bid,12);
    boxline_ptr->bid[12] = '\0';
    strncpy(boxline_ptr->mbx,mbx,8);
    boxline_ptr->mbx[8] = '\0';
    boxline_ptr->bytes = bytes;
    boxline_ptr->lt = -1;
    strncpy(boxline_ptr->mbx,title,MAXCOLS);
    boxline_ptr->title[MAXCOLS] = '\0';
    boxline_ptr->start_col = 0;
    boxline_ptr->num_col = COLS - 1;
    return(1);
  }

/* DIEBOX list 

 263 DL1ZAX 02.11.94 18:03   6763  DL-RUNDSPRUCH NR. 39/94

*/  
  res = sscanf(str,"%d %s %s %s %d %s",
               &number, call, date, time, &bytes, title);
  line_ok = (res == 6);
  if (line_ok) {

    boxline_ptr->type = BOXLINE_LIST;
    boxline_ptr->msg_nr = -1;
    strncpy(boxline_ptr->call,call,6);
    boxline_ptr->call[6] = '\0';
    if (global_board[0] == '\0') 
      boxline_ptr->board[0] = '\0';
    else
      strcpy(boxline_ptr->board,global_board);
    boxline_ptr->number = number;
    strncpy(boxline_ptr->date,date,8);
    boxline_ptr->date[8] = '\0';
    strncpy(boxline_ptr->time,time,5);
    boxline_ptr->time[5] = '\0';
    boxline_ptr->bid[0] = '\0';
    boxline_ptr->mbx[0] = '\0';
    boxline_ptr->bytes = bytes;
    boxline_ptr->lt = -1;
    strncpy(boxline_ptr->mbx,title,MAXCOLS);
    boxline_ptr->title[MAXCOLS] = '\0';
    boxline_ptr->start_col = 0;
    boxline_ptr->num_col = COLS - 1;
    return(1);
  }

/* DIEBOX und DPBOX dir

BAYBOX....30 BAYCOM....60 BBS.......60 BILDER....30 BLN-DIGI.180 C64......180
KENWOOD.....0 KEPLER......3 KW..........0 LINKTRX.....0 LINUX.......0

*/
  res = sscanf(str,"%s %s %s %s %s %s",
               dir[0].str,dir[1].str,dir[2].str,dir[3].str,
               dir[4].str,dir[5].str);
  if ((res >= 1) && (res <= 6)) {
    i = 0;
    line_ok = 1;
    while (line_ok && (i < res)) {
      line_ok = extract_board_number(dir[i].str,dir[i].board,&number);
      i++;
    }
  }
  if (line_ok) {

    pos = 0;
    i = 0;
    first = 1;
    while ((pos < res) && (i < strlen(str))) {
      if ((str[i] != ' ') && (first)) {
        dir[pos].start_col = i;
        first = 0;
      }
      if ((str[i] == ' ') && (!first)) {
        dir[pos].end_col = i;
        pos++;
        first = 1;
      }
      i++;
    }
    if (!first) {
      dir[pos].end_col = i - 1;
    }

    pos = 0;
    i = 0;
    while ((pos < res) && (i < strlen(str)) && (i < column)) {
      if (str[i] == ' ') pos++;
      i++;
    }
    if (pos == res) line_ok = 0;
  }
  if (line_ok) {
    boxline_ptr->type = BOXLINE_DIR;
    boxline_ptr->msg_nr = -1;
    boxline_ptr->call[0] = '\0';
    strcpy(boxline_ptr->board,dir[pos].board);
    boxline_ptr->number = -1;
    boxline_ptr->date[0] = '\0';
    boxline_ptr->time[0] = '\0';
    boxline_ptr->bid[0] = '\0';
    boxline_ptr->mbx[0] = '\0';
    boxline_ptr->bytes = -1;
    boxline_ptr->lt = -1;
    boxline_ptr->title[0] = '\0';
    boxline_ptr->start_col = dir[pos].start_col;
    boxline_ptr->num_col = dir[pos].end_col - dir[pos].start_col;
    return(1);
  }

/* nothing known found */

  return(0);
}

/* a key was pressed in boxlist screen */
void blist_keypressed(int channel,char ch)
{
  char key;
  int flag;

  key = toupper(ch);
  flag = BLAPPL_CONNECT;
  if (bl_mode == M_MAILBOX) {
    flag = BLAPPL_MAILBOX;
  }
  blist_analyse_line(channel,key,flag);
}

/* display line on boxlist-screen */
static void blist_textout_len(buf,len,mode,channel)
char *buf;
int len;
int mode;
int channel;
{
#if defined(USE_IFACE) && (! defined(DPBOXT))
  if (mode == M_MAILBOX) {
#endif
#if defined(DPBOXT) || defined(USE_IFACE)
    win_textout_len(buf,len,&mbboxlistwin,1);
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
  else {
#endif
#ifndef DPBOXT
    win_textout_len(buf,len,&boxlistwin[channel],1);
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
#endif
}

/* character to boxlist screen */
void blist_charout_cntl(channel,ch)
int channel;
char ch;
{
#if defined(USE_IFACE) && (! defined(DPBOXT)) 
  if (bl_mode == M_MAILBOX) {
#endif
#if defined(DPBOXT) || defined(USE_IFACE)
    win_charout_cntl(ch,&mbboxlistwin);
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT)) 
  }
  else {
#endif
#ifndef DPBOXT
    win_charout_cntl(ch,&boxlistwin[channel]);
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT)) 
  }
#endif
}

/* mark selected line in boxlist screen */
static void blist_mark_line(channel,str,len)
int channel;
char *str;
int len;
{
#if defined(USE_IFACE) && (! defined(DPBOXT))
  if (bl_mode == M_MAILBOX) {
#endif
#if defined(DPBOXT) || defined(USE_IFACE)
    win_attrib(att_monitor,&mbboxlistwin);
    win_textout_len(str,len,&mbboxlistwin,1);
    win_attrib(att_normal,&mbboxlistwin);
    win_charout_cntl(C_STLINE,&mbboxlistwin);
    win_charout_cntl(C_CUUP,&mbboxlistwin);
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
  else {
#endif
#ifndef DPBOXT
    win_attrib(att_monitor,&boxlistwin[channel]);
    win_textout_len(str,len,&boxlistwin[channel],1);
    win_attrib(att_normal,&boxlistwin[channel]);
    win_charout_cntl(C_STLINE,&boxlistwin[channel]);
    win_charout_cntl(C_CUUP,&boxlistwin[channel]);
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
#endif
}

#if defined(USE_IFACE) && (! defined(DPBOXT))
/* mark selected line as present in box */
static void blist_have_line(channel,str,len)
int channel;
char *str;
int len;
{
  win_attrib(att_special,&boxlistwin[channel]);
  win_textout_len(str,len,&boxlistwin[channel],1);
  win_attrib(att_normal,&boxlistwin[channel]);
  win_charout_cntl(C_STLINE,&boxlistwin[channel]);
  win_charout_cntl(C_CUUP,&boxlistwin[channel]);
}

/* check if line is present in box */
static int check_bid_in_line(channel)
int channel;
{
  char str[MAXCOLS+1];
  int len;
  struct boxline_result bl_result;

  win_charout_cntl(C_EOLINE,&boxlistwin[channel]);
  len = get_line_noconv(str,&boxlistwin[channel]);
  str[len] = '\0';
  strcpy(boxlist_file[channel].str,str);
  boxlist_file[channel].strlen = len;
  win_charout_cntl(C_STLINE,&boxlistwin[channel]);

  if (boxline_analysis(str,&bl_result,0)) {
    /* check if bulletin-id found */
    if (bl_result.bid[0] != '\0') {
      check_bid(channel,bl_result.bid);
      return(1);
    }
  }
  return(0);
}

void next_bid(result,channel)
int result;
int channel;
{
  struct boxlist_file *blfile;

  blfile = &boxlist_file[channel];
  if (result) {
    blist_have_line(channel,blfile->str,blfile->strlen);
  }
  blfile->curline++;
  win_charout_cntl(C_CUDWN,&boxlistwin[channel]);
  while (blfile->curline < blfile->lines) {
    if (check_bid_in_line(channel))
      return;
    blfile->curline++;
    win_charout_cntl(C_CUDWN,&boxlistwin[channel]);
  }
  win_charout_cntl(C_CUTOP,&boxlistwin[channel]);
  cmd_display(blfile->mode,channel,OK_TEXT,1);
  blfile->curline = -1;
}
#endif

/* select boxlist screen */
void sel_boxlist()
{
  if (act_mode == M_BOXLIST) {
#if defined(USE_IFACE) && (! defined(DPBOXT))
    if (bl_mode == M_MAILBOX) {
#endif
#if defined(DPBOXT) || defined(USE_IFACE)
      real_screen_hold(mb_layout,0);
      act_mode = M_MAILBOX;
      statlin_update();
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
    }
    else {
#endif
#ifndef DPBOXT
      real_screen_hold(layout[act_channel],0);
      act_mode = M_CONNECT;
      statlin_update();
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
    }
#endif
  }
  else {
#ifndef DPBOXT
    if (act_mode == M_CONNECT) {
      if (boxlist_file[act_channel].type == -1) return;
#ifdef USE_IFACE
      if (boxlist_file[act_channel].curline != -1) return;
#endif
      real_screen_hold(boxlist_layout[act_channel],2);
#ifdef USE_IFACE
      bl_mode = M_CONNECT;
#endif
    }
#ifdef USE_IFACE  
    else if (act_mode == M_MAILBOX) {
      if (mbboxlist_file.type == -1) return;
      real_screen_hold(mbboxlist_layout,2);
      bl_mode = M_MAILBOX;
    }
#endif
    else {
      return;
    }
#endif /* DPBOXT */
    act_mode = M_BOXLIST;
    statlin_update();
  }
}

/* send generated line */
static void blist_send_line(channel,str,len,end)
int channel;
char *str;
int len;
int end;
{
  int i;
#ifdef DPBOXT
  if (end) mb_input(str,len);
  for (i=0;i<len;i++) {
    if (str[i] == '\r') {
      mb_nextline();
    }
    else {
      mb_charout(str[i]);
    }
  }
  if (end && txecho_flag) {
    rem_mb_display_buf(str,len);
  }
#else  
#ifdef USE_IFACE
  if (bl_mode == M_MAILBOX) {
    if (end) mb_input(str,len);
    for (i=0;i<len;i++) {
      if (str[i] == '\r') {
        mb_nextline();
      }
      else {
        mb_charout(str[i]);
      }
    }
    if (end && txecho_flag) {
      rem_mb_display_buf(str,len);
    }
  }
  else {
    if (end) dat_input(act_channel,str,len);
    else sel_boxlist();
    for (i=0;i<len;i++) {
      if (str[i] == '\r') {
        pre_nextline(channel);
      }
      else {
        pre_charout(channel,str[i]);
      }
    }
    if (end && txecho_flag) {
      insert_cr_rx(channel);
      rem_data_display_buf(channel,str,len);
    }
  }
#else
  if (end) dat_input(act_channel,str,len);
  else sel_boxlist();
  for (i=0;i<len;i++) {
    if (str[i] == '\r') {
      pre_nextline(channel);
    }
    else {
      pre_charout(channel,str[i]);
    }
  }
  if (end && txecho_flag) {
    insert_cr_rx(channel);
    rem_data_display_buf(channel,str,len);
  }
#endif
#endif
}

/* analyse boxlist line */
static void blist_analyse_line(int channel, char func, int flag)
{
  char str[MAXCOLS+1];
  char call[MAXCOLS+1];
  char file[MAXCOLS+1];
  char *s;
  int len;
  int check_nr;
  int i;
  int res;
  int number;
  int end;

#ifdef DPBOXT
  win_charout_cntl(C_EOLINE,&mbboxlistwin);
  len = get_line_noconv(str,&mbboxlistwin);
  str[len] = '\0';
  win_charout_cntl(C_STLINE,&mbboxlistwin);
#else
#ifdef USE_IFACE
  if (bl_mode == M_MAILBOX) {
    win_charout_cntl(C_EOLINE,&mbboxlistwin);
    len = get_line_noconv(str,&mbboxlistwin);
    str[len] = '\0';
    win_charout_cntl(C_STLINE,&mbboxlistwin);
  }
  else {
    win_charout_cntl(C_EOLINE,&boxlistwin[channel]);
    len = get_line_noconv(str,&boxlistwin[channel]);
    str[len] = '\0';
    win_charout_cntl(C_STLINE,&boxlistwin[channel]);
  }
#else  
  win_charout_cntl(C_EOLINE,&boxlistwin[channel]);
  len = get_line_noconv(str,&boxlistwin[channel]);
  str[len] = '\0';
  win_charout_cntl(C_STLINE,&boxlistwin[channel]);
#endif
#endif

/* DIEBOX checklist 

    7 DL4BCU > TERMINE...16 24.09.94 DL      2214   5 2m Mobilfuchsjagd I05 08.

*/  

  res = sscanf (str,"%d %s > %s", &check_nr, call, file);
  if (strlen (file) == 12) {
    for (i = 8; i < 12; i++) if (file[i] == '.') file[i] = ' ';
    sscanf (file + 8, "%d", &number);
    file[8] = '\0';
    if ((s = strchr (file, '.')) != NULL) *s = '\0'; }
  else {        /* for Baybox dir/list */
    if (file[2] == '.' && file[5] == '.' && isdigit (file[3])) res = 0;
    else number = check_nr; }
  if (res) {
    blist_mark_line (channel, str, len);
    end = 1;
    switch (func) {
    case FUNC_READ:
      if (blist_add_plus)
        sprintf(str, "R %s %d +\r", file, number);
      else
        sprintf(str, "R %s %d\r", file, number);
      break;
    case FUNC_LIST:
      sprintf(str, "L %s %d\r", file, number);
      break;
    case FUNC_ERASE:
      sprintf(str, "E %s %d\r", file, number);
      break;
    case FUNC_KILL:
      sprintf(str, "K %s %d\r", file, number);
      break;
    case FUNC_TRANSFER:
      sprintf(str, "TR %s %d ", file, number);
      end = 0;
      break;
    case FUNC_LIFETIME:
      sprintf(str, "LT %s %d ", file, number);
      end = 0; 
      break;
    }
    blist_send_line (channel, str, strlen (str), end);
    return;
  }

/* DIEBOX list 

 263 DL1ZAX 02.11.94 18:03   6763  DL-RUNDSPRUCH NR. 39/94

*/  

  res = sscanf (str, "%d %s", &number, call);
  if (res == 2) {
    blist_mark_line(channel,str,len);
    end = 1;
    switch (func) {
    case FUNC_READ:
      if (blist_add_plus)
        sprintf(str,"R %d +\r",number);
      else
        sprintf(str,"R %d\r",number);
      break;
    case FUNC_LIST:
      sprintf(str,"L %d\r",number);
      break;
    case FUNC_ERASE:
      sprintf(str,"E %d\r",number);
      break;
    case FUNC_KILL:
      sprintf(str,"K %d\r",number);
      break;
    case FUNC_TRANSFER:
      sprintf(str,"TR %d ",number);
      end = 0;
      break;
    case FUNC_LIFETIME:
      sprintf(str,"LT %d \r",number);
      end = 0;
      break;
    }
    blist_send_line(channel,str,strlen(str),end);
    return;
  }
  
/* all other not yet implemented */

  beep();
  return;
}


/* store filename in list for delete */
static void add_tempfilename(char *tmpname)
{
  struct tmpname_entry *newentry;
  struct tmpname_entry *listentry;

  /* do entry in tempfile list */
  newentry = (struct tmpname_entry *)malloc(sizeof(struct tmpname_entry));
  newentry->name = tmpname;
  newentry->next = NULL;
  if (tmpname_root == NULL) {
    tmpname_root = newentry;
  }
  else {
    listentry = tmpname_root;
    while (listentry->next != NULL) {
      listentry = listentry->next;
    }
    listentry->next = newentry;    
  }
}

/* open a temporary file for boxlist */
void cmd_logblist(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  char *tmpname;
  struct tmpname_entry *newentry;
  struct tmpname_entry *listentry;
  
  tmpname = strdup("/tmp/tntbliXXXXXX");
  close(mkstemp(tmpname));
  /* do entry in tempfile list */
  newentry = (struct tmpname_entry *)malloc(sizeof(struct tmpname_entry));
  newentry->name = tmpname;
  newentry->next = NULL;
  if (tmpname_root == NULL) {
    tmpname_root = newentry;
  }
  else {
    listentry = tmpname_root;
    while (listentry->next != NULL) {
      listentry = listentry->next;
    }
    listentry->next = newentry;    
  }
  open_logfile(RX_NORM,RX_RCV,channel,strlen(tmpname),mode,tmpname);
}

static void scan_for_board(char *buffer,int len)
{
  int i;
  int bslen;
  int restlen;
  char *ptr1;
  char *ptr2;
  
  i = 0;
  while (board_string[i][0] != '\0') {
    bslen = strlen(board_string[i]);
    if (bslen < len) {
      if (memcmp(buffer,board_string[i],bslen) == 0) {
        ptr2 = global_board;
        ptr1 = &buffer[bslen];
        restlen = len - bslen;
        if (restlen > 8) restlen = 8;
        while (restlen) {
          switch (*ptr1) {
          case ' ':
          case '\r':
            *ptr2 = '\0';
            return;
            break;
          default:
            *ptr2 = *ptr1;
            ptr1++;
            ptr2++;
            break;
          }
          restlen--;
        }
        *ptr2 = '\0';
        return;
      }
    }
    i++;
  }
}

#define BUFSIZE 512

/* start a boxlist screen with specified file */
/* par1 = 1: no check whether boxlist already open (be careful) */
void cmd_blist(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  int file_len;
  int end;
  int len2;
  char buf[BUFSIZE];
  int i;
  int lines;
  int column;
  struct boxlist_file *blfile;
  char *linebuf;
  int linelen;
  int linenr;

#if defined(USE_IFACE) && (! defined(DPBOXT))  
  if (mode == M_MAILBOX) {
#endif
#if defined(DPBOXT) || defined(USE_IFACE)
    blfile = &mbboxlist_file;
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))  
  }
  else {
#endif
#ifndef DPBOXT
    blfile = &boxlist_file[channel];
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))  
  }
#endif

  if (par1 != 1) {
    if (blfile->type != -1) {
      cmd_display(mode,channel, _("Boxlist already active"),1);
      return;
    }
  }
#if defined(USE_IFACE) || defined(DPBOXT)  
  else {
    if (blfile->type != -1) {
      if (next_blist_save < BL_SAVEWIN) {
        memcpy(&blistsavewin[next_blist_save],&mbboxlistwin,
               sizeof(struct window));
        blistsavewin[next_blist_save].real = -1;
        blistsavewin[next_blist_save].holdflag = 1;
        strcpy(blistsavewin_board[next_blist_save],global_board);
        next_blist_save++;
      }
      else {
        cmd_display(mode,channel, _("No save buffer left"),1);
        return;
      }
    }
  }
#endif

  if (len == 0) {
    close_file(1,0,channel,0,mode,NULL);
#if (defined(USE_IFACE)) || (defined(DPBOXT))
    if ((mode == M_MAILBOX) && (mb_file.name[0] != '\0')) {
      strcpy(blfile->name,mb_file.name);
    }
#endif
#if (defined(USE_IFACE)) && (! defined(DPBOXT))
    else if ((mode != M_MAILBOX) && (rx_file[channel].name[0] != '\0')) {
      strcpy(blfile->name,rx_file[channel].name);
    }
#endif
#if (! defined(USE_IFACE)) && (! defined(DPBOXT))
    if (rx_file[channel].name[0] != '\0') {
      strcpy(blfile->name,rx_file[channel].name);
    }
#endif
    else {
      cmd_display(mode,channel, _("No boxlist file stored"),1);
      return;
    }
  }
  else {
    if(strchr(str,'/') != NULL) {
      strcpy(blfile->name,str);
    }
    else {
      strcpy(blfile->name,download_dir);
      strcat(blfile->name,str);
    }
  }
  blfile->fd = open(blfile->name,O_RDONLY); 
  if (blfile->fd == -1) {
    /* file does not exist */
    cmd_display(mode,channel, _("File not existing"),1);
    return;
  }
  
  blfile->type = BL_NORMAL;
  /* counting number of LFs in file to get number of lines */
  file_len = 0;
  lines = 0;
  column = 0;
  end = 1;
  while (end) {
    len2 = read(blfile->fd,buf,BUFSIZE);
    file_len += len2;
    for (i=0;i<len2;i++) {
      if (buf[i] == '\n') {
        lines++;
        column = 0;
      }
      else {
        column++;
        if (column == COLS) {
          lines++;
          column = 0;
        }
      }
    }
    if (len2 < BUFSIZE) end = 0;
  }
  blfile->len = file_len;
  lines += 2; /* just to be sure... */
  blfile->lines = lines;

#if defined(USE_IFACE) && (! defined(DPBOXT))
  if (mode == M_MAILBOX) {
#endif
#if defined(DPBOXT) || defined(USE_IFACE)
    open_window(lines,&mbboxlistwin,1);
    mbboxlist_layout[0].win = &mbboxlistwin;
    mbboxlist_layout[0].first_real_line = 0;
    mbboxlist_layout[0].win_num_lines = LINES-1;
    mbboxlist_layout[0].pagesize = LINES-4;
    mbboxlist_layout[1].win = &statlin;
    mbboxlist_layout[1].first_real_line = LINES-1;
    mbboxlist_layout[1].win_num_lines = 1;
    mbboxlist_layout[1].pagesize = 0;
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
  else {
#endif
#ifndef DPBOXT
    open_window(lines,&boxlistwin[channel],1);
    boxlist_layout[channel][0].win = &boxlistwin[channel];
    boxlist_layout[channel][0].first_real_line = 0;
    boxlist_layout[channel][0].win_num_lines = LINES-1;
    boxlist_layout[channel][0].pagesize = LINES-4;
    boxlist_layout[channel][1].win = &statlin;
    boxlist_layout[channel][1].first_real_line = LINES-1;
    boxlist_layout[channel][1].win_num_lines = 1;
    boxlist_layout[channel][1].pagesize = 0;
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
#endif
  
  /* seek to file begin */
  lseek(blfile->fd,0,SEEK_SET);
  linelen = 0;
  linenr = 0;
  end = 1;
  global_board[0] = '\0';
  linebuf = (char *)malloc(COLS+2);
  while (end) {
    len2 = read(blfile->fd,buf,BUFSIZE);
    for (i=0;i<len2;i++) {
      if (buf[i] == '\n') {
        if ((linenr < 8) && (global_board[0] == '\0')) {
          scan_for_board(linebuf,linelen);
        }
        linebuf[linelen] = '\r';
        linelen++;
        blist_textout_len(linebuf,linelen,mode,channel);
        linenr++;
        linelen = 0;
      }
      else {
        if (linelen < COLS) {
          linebuf[linelen] = buf[i];
          linelen++;
        }
        else {
          blist_textout_len(linebuf,linelen,mode,channel);
          linelen = 0;
          linenr++;
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
    blist_textout_len(linebuf,linelen,mode,channel);
    linelen = 0;
  }
  free(linebuf);
  close(blfile->fd);
#ifdef DPBOXT
  cmd_display(mode,channel,OK_TEXT,1);
#else
#ifdef USE_IFACE  
  if (mode == M_MAILBOX) {
    cmd_display(mode,channel,OK_TEXT,1);
  }
  else { 
    if ((box_active_flag) && (!box_busy_flag)) {
      /* mark all mails in box */
      cmd_display(mode,channel, _("Scanning DPBox"),1);
      blfile->curline = 0;
      blfile->mode = mode;
      win_charout_cntl(C_CUTOP,&boxlistwin[channel]);
      while (blfile->curline < blfile->lines) {
        if (check_bid_in_line(channel))
          return;
        blfile->curline++;
        win_charout_cntl(C_CUDWN,&boxlistwin[channel]);
      }
    }
    cmd_display(mode,channel,OK_TEXT,1);
    blfile->curline = -1;
  }
#else
  cmd_display(mode,channel,OK_TEXT,1);
#endif
#endif
}


#if defined(USE_IFACE) || defined(DPBOXT)
/* restore last saved boxlist contents */
void cmd_blrestore(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  if (next_blist_save) {
    if (mbboxlist_file.type != -1) {
      close_window(&mbboxlistwin);
    }
    memcpy(&mbboxlistwin,&blistsavewin[next_blist_save-1],
           sizeof(struct window));
    strcpy(global_board,blistsavewin_board[next_blist_save-1]);
    next_blist_save--;
    mbboxlist_file.type = BL_NORMAL;
    cmd_display(mode,channel,OK_TEXT,1);
    if (mbboxlist_file.type == -1) return;
    if (act_mode == M_BOXLIST) {
      real_screen(mbboxlist_layout);
    }
    else {
      real_screen_hold(mbboxlist_layout,2);
    }
    bl_mode = M_MAILBOX;
    act_mode = M_BOXLIST;
    statlin_update();
    return;
  }
  else {
    if (act_mode == M_BOXLIST) {
      sel_mailbox();
      cmd_xblist(0,0,0,0,M_MAILBOX,NULL);
      return;
    }
    else {
      cmd_display(mode,channel, _("No save buffer found"),1);
      return;
    }
  }
}

/* show number of save buffers */
void cmd_showblsv(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  char buffer[80];
  
  sprintf(buffer, _("Number of used save buffers: %d"),next_blist_save);
  cmd_display(mode,channel,buffer,1);
}

/* flush all save buffers */
void cmd_blflush(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  while (next_blist_save) {
    close_window(&blistsavewin[next_blist_save-1]);
    next_blist_save--;
  }
  cmd_display(mode,channel,OK_TEXT,1);
}
#endif

/* finish boxlist */
void cmd_xblist(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  struct boxlist_file *blfile;

#if defined(USE_IFACE) && (! defined(DPBOXT))
  if (mode == M_MAILBOX) {
#endif
#if defined(DPBOXT) || defined(USE_IFACE)
    blfile = &mbboxlist_file;
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
  else {
#endif
#ifndef DPBOXT
    blfile = &boxlist_file[channel];
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
#endif
  
  if (blfile->type == -1) {
    cmd_display(mode,channel,"No boxlist active",1);
    return;
  }

#if defined(USE_IFACE) && (! defined(DPBOXT))
  if (mode == M_MAILBOX) {
#endif
#if defined(DPBOXT) || defined(USE_IFACE)
    close_window(&mbboxlistwin);
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
  else {
#endif
#ifndef DPBOXT
    close_window(&boxlistwin[channel]);
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  }
#endif

  blfile->type = -1;  
  cmd_display(mode,channel,OK_TEXT,1);
}

/* init of boxlist */
void init_blist()
{
  int i;
  int j;

  blcmdlist_root = NULL;
  load_blcmdfile();

  tmpname_root = NULL;
  
#if defined(DPBOXT) || defined(USE_IFACE)
  mbboxlist_file.type = -1;
  for (j = 0; j < 5; j++) {
    mbboxlist_layout[j].win = NULL;
    mbboxlist_layout[j].first_real_line = 0;
    mbboxlist_layout[j].win_num_lines = 0;
    mbboxlist_layout[j].pagesize = 0;
  }
  next_blist_save = 0;
#endif
#ifndef DPBOXT
  for (i=0;i<tnc_channels;i++) {
    boxlist_file[i].type = -1;
    for (j = 0; j < 5; j++) {
      boxlist_layout[i][j].win = NULL;
      boxlist_layout[i][j].first_real_line = 0;
      boxlist_layout[i][j].win_num_lines = 0;
      boxlist_layout[i][j].pagesize = 0;
    }
  }
#endif
#ifdef DPBOXT
  bl_mode = M_MAILBOX;
#endif
#if defined(USE_IFACE) && (! defined(DPBOXT))
  bl_mode = M_CONNECT;
#endif
}

void reinit_blist()
{
  int i;
  int j;

#if defined(DPBOXT) || defined(USE_IFACE)
  if (mbboxlist_file.type != -1) {
    for (j = 0; j < LAYOUTPARTS; j++) {
      mbboxlist_layout[j].win = NULL;
      mbboxlist_layout[j].first_real_line = 0;
      mbboxlist_layout[j].win_num_lines = 0;
      mbboxlist_layout[j].pagesize = 0;
    }
    mbboxlist_layout[0].win = &mbboxlistwin;
    mbboxlist_layout[0].first_real_line = 0;
    mbboxlist_layout[0].win_num_lines = LINES-1;
    mbboxlist_layout[0].pagesize = LINES-4;
    mbboxlist_layout[1].win = &statlin;
    mbboxlist_layout[1].first_real_line = LINES-1;
    mbboxlist_layout[1].win_num_lines = 1;
    mbboxlist_layout[1].pagesize = 0;
  }
#endif
#ifndef DPBOXT
  for (i=0;i<tnc_channels;i++) {
    if (boxlist_file[i].type != -1) {
      for (j = 0; j < LAYOUTPARTS; j++) {
        boxlist_layout[i][j].win = NULL;
        boxlist_layout[i][j].first_real_line = 0;
        boxlist_layout[i][j].win_num_lines = 0;
        boxlist_layout[i][j].pagesize = 0;
      }
      boxlist_layout[i][0].win = &boxlistwin[i];
      boxlist_layout[i][0].first_real_line = 0;
      boxlist_layout[i][0].win_num_lines = LINES-1;
      boxlist_layout[i][0].pagesize = LINES-4;
      boxlist_layout[i][1].win = &statlin;
      boxlist_layout[i][1].first_real_line = LINES-1;
      boxlist_layout[i][1].win_num_lines = 1;
      boxlist_layout[i][1].pagesize = 0;
    }
  }
#endif  
}

/* exit of boxlist */
void exit_blist()
{
  int i;
  struct tmpname_entry *listentry;
  struct tmpname_entry *oldentry;

  clear_blcmdfile();
    
  if (tmpname_root != NULL) {
    listentry = tmpname_root;
    do {
      oldentry = listentry;
      listentry = oldentry->next;
      unlink(oldentry->name);
      free(oldentry->name);
      free(oldentry);
    } while (listentry != NULL);
  }
#if defined(DPBOXT) || defined(USE_IFACE)
  if (mbboxlist_file.type != -1) {
    close_window(&mbboxlistwin);
    mbboxlist_file.type = -1;
  }
  cmd_blflush(0,0,0,0,M_CMDSCRIPT,NULL);
#endif
#ifndef DPBOXT
  for (i=0;i<tnc_channels;i++) {
    if (boxlist_file[i].type != -1) {
      close_window(&boxlistwin[i]);
      boxlist_file[i].type = -1;
    }
  }
#endif  
}

#ifndef DPBOXT
void free_boxlist()
{
  free(boxlist_file);
  free(boxlistwin);
  free(boxlist_layout);
}

int alloc_boxlist()
{
  boxlist_file = (struct boxlist_file *)
    malloc(tnc_channels * sizeof(struct boxlist_file));
  boxlistwin = (struct window *)
    malloc(tnc_channels * sizeof(struct window));
  boxlist_layout = (struct real_layout (*) [LAYOUTPARTS])
    malloc(tnc_channels * sizeof(struct real_layout [LAYOUTPARTS]));
  return((boxlist_file == NULL) ||
         (boxlistwin == NULL) ||
         (boxlist_layout == NULL));
}
#endif

#if defined(USE_IFACE) || defined(DPBOXT)
void cmd_xbox(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  int newchannel;
  char command[256];

  if (len == 0) {
    cmd_display(mode,channel, _("Error: no command given"),1);
    return;
  }
  strcpy(command,str);
  strcat(command,"\r");
#ifdef DPBOXT
  newchannel = 1;
#else
  newchannel = tnc_channels;
#endif
  cmd_actiface(par1,-1,newchannel,strlen(box_socket),M_MAILBOX,box_socket);
  if (iface_active(newchannel))
    write_iface(newchannel,strlen(command),command);
}

void single_display_buf(char *buf,int len)
{
  char *filename;
  char *ptr;
  
  filename = (char *)malloc(len+1);
  if (filename == NULL) return;
  memcpy(filename,buf,len);
  filename[len] = '\0';
  if ((ptr = strchr(filename,'\n')) != NULL) *ptr = '\0';
  if ((ptr = strchr(filename,'\r')) != NULL) *ptr = '\0';
  
  add_tempfilename(filename);
  
  cmd_blist(1,0,0,strlen(filename),M_MAILBOX,filename);

  if (mbboxlist_file.type == -1) return;
  if (act_mode == M_BOXLIST) {
    real_screen(mbboxlist_layout);
  }
  else {
    real_screen_hold(mbboxlist_layout,2);
  }
  bl_mode = M_MAILBOX;
  act_mode = M_BOXLIST;
  statlin_update();
}
#endif
