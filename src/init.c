/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993-1997 by Mark Wahl
   For license details see documentation
   Procedures for initialization (init.c)
   created: Mark Wahl DL4YBG 94/01/11
   updated: Mark Wahl DL4YBG 97/03/09
   updated: Mark Wahl DL4YBG 97/09/06
   updated: Matthias Hensler WS1LS 99/09/09
   updated: Johann Hanne DH3MB 98/12/28
   updated: Peter Mack DL3FCJ 99/03/19
   updated: Berndt Josef Wulf VK5ABN 99/04/16
*/

#include "tnt.h"
#include "init.h"

/* external function declarations */
extern void cmd_display();

#ifdef DPBOXT
extern int scr_divide;
extern char tnt_help_file[];
extern char tnt_work_dir[];
extern char tnt_conf_dir[];
extern char tnt_doc_dir[];
extern char tnt_log_dir[];
extern char tnt_proc_dir[];
#ifdef TNT_SOUND
extern char tnt_sound_dir[];
#endif
extern int auto_newline;
extern int supp_hicntl;
extern int auto_newline_save;
extern int supp_hicntl_save;
extern char box_socket[];
extern int tnt_daemon;
int use_select;

static char box_socket2[MAXCHAR];
char tnt_initfile[MAXCHAR];
char tnt_realinitfile[256];

/* variables filled from init-file */
char attc_normal;
char attc_statline;
char attc_monitor;
char attc_cstatline;
char attc_controlchar;
char attc_remote;
char attc_special;
char attc_monport1;
char attc_monport2;
char attm_normal;
char attm_statline;
char attm_monitor;
char attm_cstatline;
char attm_controlchar;
char attm_remote;
char attm_special;
char attm_monport1;
char attm_monport2;
int input_linelen;
int lines_mbinput;
int lines_mboutput;
int mbscr_divide;
int color;
int color_save;
int termcap;
int termcap_save;
char tnt_termcapfile[MAXCHAR];
char tnt_blcmdfile[80];
char download_dir[MAXCHAR];
char func_key_file[MAXCHAR];
char macrotext_dir[MAXCHAR];
extern int insertmode;

#else

extern int scr_divide;
extern char rem_info_file[];
extern char rem_help_file[];
extern char news_file_name[];
extern char name_file_name[];
extern char tnt_help_file[];
extern char tnt_work_dir[];
extern char tnt_doc_dir[];
extern char tnt_conf_dir[];
extern char tnt_log_dir[];
extern char tnt_proc_dir[];
#ifdef TNT_SOUND
extern char tnt_sound_dir[];
#endif
extern char remote_dir[];
extern char ctext_dir[];
extern char abin_dir[];
extern char tnt_logbookfile[];
extern int pty_timeout;
extern int auto_newline;
extern int supp_hicntl;
extern int auto_newline_save;
extern int supp_hicntl_save;
#ifdef GEN_NEW_USER
extern int unix_new_user;
extern char unix_user_dir[];
extern int unix_first_uid;
extern int unix_user_gid;
extern int unix_user_max;
#endif
#ifdef USE_IFACE
extern char box_socket[];
extern int tnt_box_ssid;
extern char tnt_box_call[];
#endif
extern char tnt_pwfile[];
extern char tnt_sysfile[];
extern char tnt_noremfile[];
extern char tnt_flchkfile[];
extern char tnt_notownfile[];
extern char tnt_autostartfile[];
extern char tnt_extremotefile[];
extern int tnt_daemon;
int use_select;

char tnt_initfile[MAXCHAR];
char tnt_realinitfile[256];
char tnt_logfile[MAXCHAR];

/* variables filled from init-file */
char device[MAXCHAR];
int soft_tnc;
unsigned int speed;
int speedflag;
int tnc_channels;
int r_channels;
int file_paclen;
int tnt_comp;
int moni_decomp;
int moni_chkbin;
int ibm_umlaut_flag;
char tnt_upfile[MAXCHAR];
char tnt_downfile[MAXCHAR];
char attc_normal;
char attc_statline;
char attc_monitor;
char attc_cstatline;
char attc_controlchar;
char attc_remote;
char attc_special;
char attc_monport1;
char attc_monport2;
char attm_normal;
char attm_statline;
char attm_monitor;
char attm_cstatline;
char attm_controlchar;
char attm_remote;
char attm_special;
char attm_monport1;
char attm_monport2;
int lines_command;
int lines_monitor;
int lines_input;
int lines_output;
int lines_moncon;
int lines_r_input;
int lines_r_output;
int input_linelen;
#ifdef USE_IFACE
int lines_mbinput;
int lines_mboutput;
int mbscr_divide;
#endif
extern int lines_xmon;
extern int lines_xmon_pre;
extern int xmon_scr_divide;
extern int num_heardentries;
int color;
int color_save;
int termcap;
int termcap_save;
char tnt_termcapfile[MAXCHAR];
char tnt_blcmdfile[80];
char proc_file[MAXCHAR];
char tnt_cookiefile[MAXCHAR];
char tnt_lockfile[MAXCHAR];
char tnt_ctextfile[MAXCHAR];
char remote_user[MAXCHAR];
char resy_log_file[MAXCHAR];
#ifdef DEBUGIO
char debugiof[MAXCHAR];
#endif
char tnt_telltext_file[MAXCHAR];
#ifdef TNT_SOUND
char tnt_sound_file[MAXCHAR];
#endif
char tnt_session_log[MAXCHAR];
char tnt_qtextfile[MAXCHAR];
#ifdef BCAST
char bcast_log_file[MAXCHAR];
#endif
char upload_dir[MAXCHAR];
char download_dir[MAXCHAR];
char tnt_7plus_dir[MAXCHAR];
char yapp_dir[MAXCHAR];
char tnt_bin_dir[MAXCHAR];
#ifdef HAVE_SOCKET
char sock_passfile[MAXCHAR];
#endif
char func_key_file[MAXCHAR];
char macrotext_dir[MAXCHAR];
#ifdef USE_IFACE
char newmaildir[MAXCHAR];
char autobox_dir[MAXCHAR];
char tnt_boxender[MAXCHAR];
char f6fbb_box[MAXCHAR];
#endif
char route_file_name[MAXCHAR];
char frontend_socket[MAXCHAR];
extern int altstat;
extern int disc_on_start;
#ifdef BCAST
extern char tnt_bctempdir[];
extern char tnt_bcsavedir[];
extern char tnt_bcnewmaildir[];
#endif
#ifdef HAVE_SOCKET
extern int fixed_wait;
extern int amount_wait;
#endif
extern int insertmode;
#ifdef USE_AX25K
extern char ax25k_port[];
extern int fullmoni_flag;
extern char moni_socket[];
#endif
extern char tnt_editor[];
#endif /* DPBOXT */
char tnt_errlog[MAXCHAR];
int ax25k_active;

static int analyse_value(str1,str2)
char *str1;
char *str2;
{
  int tmp;
  
#ifndef DPBOXT
  if (strcmp(str1,"use_select") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    use_select = tmp;
    return(0);
  }
  else if (strcmp(str1,"device") == 0) {
    strcpy(device,str2);
    return(0);
  }
  else if (strcmp(str1,"soft_tnc") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    soft_tnc = tmp;
    return(0);
  }
#ifdef HAVE_SOCKET
  else if (strcmp(str1,"fixed_wait") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    fixed_wait = tmp;
    return(0);
  }
   else if (strcmp(str1,"amount_wait") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    amount_wait = tmp;
    return(0);
  }
#endif
  else if (strcmp(str1,"speed") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    speedflag = 0;
    switch (tmp) {
    case 4800:
      speed = B4800;
      return(0);
    case 9600:
      speed = B9600;
      return(0);
    case 19200:
      speed = B19200;
      return(0);
    case 38400:
      speed = B38400;
      return(0);
#ifdef USE_HIBAUD
    case 57600:
      speed = B38400;
      speedflag = ASYNC_SPD_HI;
      return(0);
    case 115200:
      speed = B38400;
      speedflag = ASYNC_SPD_VHI;
      return(0);
#endif
    default:
      return(1);
    }
  }
  else if (strcmp(str1,"tnc_channels") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    tnc_channels = tmp + 1; /* include unproto channel */
    return(0);
  }
  else if (strcmp(str1,"r_channels") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    r_channels = tmp;
    return(0);
  }
  else if (strcmp(str1,"file_paclen") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    file_paclen = tmp;
    if ((file_paclen > 256) || (file_paclen < 20)) return(1);
    if (tnt_comp && (file_paclen == 256)) return(1);
    return(0);
  }
  else if (strcmp(str1,"tnt_comp") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    tnt_comp = tmp;
    if (tnt_comp && (file_paclen == 256)) return(1);
    return(0);
  }
  else if (strcmp(str1,"moni_decomp") == 0) { /* DH3MB */
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    moni_decomp = tmp;
    return(0);
  }
  else if (strcmp(str1,"moni_chkbin") == 0) { /* DL3FCJ */
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    moni_chkbin = tmp;
    return(0);
  }
  else if (strcmp(str1,"ibm_umlaut") == 0) { /* DH3MB */
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    ibm_umlaut_flag = tmp;
    return(0);
  }
  else if (strcmp(str1,"attc_normal") == 0) {
#else
  if (strcmp(str1,"attc_normal") == 0) {
#endif
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attc_normal = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attc_statline") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attc_statline = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attc_monitor") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attc_monitor = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attc_cstatline") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attc_cstatline = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attc_controlchar") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attc_controlchar = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attc_remote") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attc_remote = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attc_special") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attc_special = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attc_monport1") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attc_monport1 = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attc_monport2") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attc_monport2 = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attm_normal") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attm_normal = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attm_statline") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attm_statline = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attm_monitor") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attm_monitor = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attm_cstatline") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attm_cstatline = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attm_controlchar") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attm_controlchar = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attm_remote") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attm_remote = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attm_special") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attm_special = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attm_monport1") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attm_monport1 = (char) tmp;
    return(0);
  }
  else if (strcmp(str1,"attm_monport2") == 0) {
    if (sscanf(str2,"%x",&tmp) != 1) return(1);
    attm_monport2 = (char) tmp;
    return(0);
  }
#ifndef DPBOXT
  else if (strcmp(str1,"lines_command") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_command = tmp;
    return(0);
  }
  else if (strcmp(str1,"lines_monitor") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_monitor = tmp;
    return(0);
  }
  else if (strcmp(str1,"lines_input") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_input = tmp;
    return(0);
  }
  else if (strcmp(str1,"lines_output") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_output = tmp;
    return(0);
  }
  else if (strcmp(str1,"lines_moncon") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_moncon = tmp;
    return(0);
  }
  else if (strcmp(str1,"lines_r_input") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_r_input = tmp;
    return(0);
  }
  else if (strcmp(str1,"lines_r_output") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_r_output = tmp;
    return(0);
  }
  else if (strcmp(str1,"scr_divide") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    scr_divide = tmp;
    return(0);
  }
  else if (strcmp(str1,"lines_xmon") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_xmon = tmp;
    return(0);
  }
  else if (strcmp(str1,"lines_xmon_pre") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_xmon_pre = tmp;
    return(0);
  }
  else if (strcmp(str1,"xmon_scr_divide") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    xmon_scr_divide = tmp;
    return(0);
  }
  else if (strcmp(str1,"num_heardentries") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    num_heardentries = tmp;
    return(0);
  }
#endif
#ifdef USE_IFACE
  else if (strcmp(str1,"lines_mbinput") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_mbinput = tmp;
    return(0);
  }
  else if (strcmp(str1,"lines_mboutput") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    lines_mboutput = tmp;
    return(0);
  }
  else if (strcmp(str1,"mbscr_divide") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    mbscr_divide = tmp;
    return(0);
  }
#endif
  else if (strcmp(str1,"input_linelen") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    input_linelen = tmp;
    return(0);
  }
  else if (strcmp(str1,"insertmode") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    insertmode = tmp;
    return(0);
  }
#ifndef DPBOXT
  else if (strcmp(str1,"tnt_upfile") == 0) {
    strcpy(tnt_upfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_downfile") == 0) {
    strcpy(tnt_downfile,str2);
    return(0);
  }
  else if (strcmp(str1,"proc_file") == 0) {
    strcpy(proc_file,str2);
    return(0);
  }
  else if (strcmp(str1,"rem_info_file") == 0) {
    strcpy(rem_info_file,str2);
    return(0);
  }
  else if (strcmp(str1,"rem_help_file") == 0) {
    strcpy(rem_help_file,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_cookiefile") == 0) {
    strcpy(tnt_cookiefile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_lockfile") == 0) {
    strcpy(tnt_lockfile,str2);
    return(0);
  }
#endif
  else if (strcmp(str1,"tnt_help_file") == 0) {
    strcpy(tnt_help_file,str2);
    return(0);
  }
  else if (strcmp(str1,"termcap") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    termcap = tmp;
    return(0);
  }
  else if (strcmp(str1,"color") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    color = tmp;
    return(0);
  }
  else if (strcmp(str1,"tnt_termcapfile") == 0) {
    strcpy(tnt_termcapfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_blcmdfile") == 0) {
    strcpy(tnt_blcmdfile,str2);
    return(0);
  }
#ifndef DPBOXT
  else if (strcmp(str1,"remote_user") == 0) {
    strcpy(remote_user,str2);
    return(0);
  }
  else if (strcmp(str1,"news_file_name") == 0) {
    strcpy(news_file_name,str2);
    return(0);
  }
  else if (strcmp(str1,"name_file_name") == 0) {
    strcpy(name_file_name,str2);
    return(0);
  }
  else if (strcmp(str1,"route_file_name") == 0) {
    strcpy(route_file_name,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_ctextfile") == 0) {
    strcpy(tnt_ctextfile,str2);
    return(0);
  }
#endif
  else if (strcmp(str1,"tnt_work_dir") == 0) {
    strcpy(tnt_work_dir,str2);
    tmp = strlen(tnt_work_dir);
    if (tnt_work_dir[tmp-1] != '/') {
      tnt_work_dir[tmp] = '/';
      tnt_work_dir[tmp+1] = '\0';
    }
    return(0);
  }
  else if (strcmp(str1,"tnt_conf_dir") == 0) {
    strcpy(tnt_conf_dir,str2);
    tmp = strlen(tnt_conf_dir);
    if (tnt_conf_dir[tmp-1] != '/') {
      tnt_conf_dir[tmp] = '/';
      tnt_conf_dir[tmp+1] = '\0';
    }
    return(0);
  }
  else if (strcmp(str1,"tnt_doc_dir") == 0) {
    strcpy(tnt_doc_dir,str2);
    tmp = strlen(tnt_doc_dir);
    if (tnt_doc_dir[tmp-1] != '/') {
      tnt_doc_dir[tmp] = '/';
      tnt_doc_dir[tmp+1] = '\0';
    }
    return(0);
  }
  else if (strcmp(str1,"tnt_log_dir") == 0) {
    strcpy(tnt_log_dir,str2);
    tmp = strlen(tnt_log_dir);
    if (tnt_log_dir[tmp-1] != '/') {
      tnt_log_dir[tmp] = '/';
      tnt_log_dir[tmp+1] = '\0';
    }
    return(0);
  }
  else if (strcmp(str1,"tnt_proc_dir") == 0) {
    strcpy(tnt_proc_dir,str2);
    tmp = strlen(tnt_proc_dir);
    if (tnt_proc_dir[tmp-1] != '/') {
      tnt_proc_dir[tmp] = '/';
      tnt_proc_dir[tmp+1] = '\0';
    }
    return(0);
  }
#ifdef TNT_SOUND
  else if (strcmp(str1,"tnt_sound_dir") == 0) {
    strcpy(tnt_sound_dir,str2);
    tmp = strlen(tnt_sound_dir);
    if (tnt_sound_dir[tmp-1] != '/') {
      tnt_sound_dir[tmp] = '/';
      tnt_sound_dir[tmp+1] = '\0';
    }
    return(0);
  }
#endif
#ifndef DPBOXT
  else if (strcmp(str1,"remote_dir") == 0) {
    strcpy(remote_dir,str2);
    return(0);
  }
  else if (strcmp(str1,"ctext_dir") == 0) {
    strcpy(ctext_dir,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_tellfile") == 0) {
    strcpy(tnt_telltext_file,str2);
    return(0);
  }
#ifdef TNT_SOUND
  else if (strcmp(str1,"tnt_soundfile") == 0) {
    strcpy(tnt_sound_file,str2);
    return(0);
  }
#endif
  else if (strcmp(str1,"tnt_sessionlog") ==0) {
    strcpy(tnt_session_log,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_qtextfile") ==0) {
    strcpy(tnt_qtextfile,str2);
    return(0);
  }
  else if (strcmp(str1,"abin_dir") == 0) {
    strcpy(abin_dir,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_logbookfile") == 0) {
    strcpy(tnt_logbookfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_pwfile") == 0) {
    strcpy(tnt_pwfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_sysfile") == 0) {
    strcpy(tnt_sysfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_noremfile") == 0) {
    strcpy(tnt_noremfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_flchkfile") == 0) {
    strcpy(tnt_flchkfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_notownfile") == 0) {
    strcpy(tnt_notownfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_autostartfile") == 0) {
    strcpy(tnt_autostartfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_extremotefile") == 0) {
    strcpy(tnt_extremotefile,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_editor") == 0) {
    if (strcmp(str2,"0") == 0)
      strcpy(tnt_editor,"\0");
    else
      strcpy(tnt_editor,str2);
    return(0);
  }
#endif
  else if (strcmp(str1,"blist_add_plus") == 0) {
    /* just ignore */
    return(0);
  }
#ifndef DPBOXT
  else if (strcmp(str1,"pty_timeout") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    pty_timeout = tmp;
    return(0);
  }
#endif
  else if (strcmp(str1,"auto_newline") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    auto_newline = tmp;
    return(0);
  }
  else if (strcmp(str1,"supp_hicntl") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    supp_hicntl = tmp;
    return(0);
  }
#ifndef DPBOXT
#ifdef DEBUGIO
  else if (strcmp(str1,"debugiof") == 0) {
    strcpy(debugiof,str2);
    return(0);
  }
#endif
  else if (strcmp(str1,"resy_log_file") == 0) {
    strcpy(resy_log_file,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_errlog") == 0) {
    strcpy(tnt_errlog,str2);
    return(0);
  }
#ifdef BCAST
  else if (strcmp(str1,"bcast_log_file") == 0) {
    strcpy(bcast_log_file,str2);
    return(0);
  }
#endif
  else if (strcmp(str1,"tnt_bin_dir") == 0) {
    strcpy(tnt_bin_dir,str2);
    return(0);
  }
  else if (strcmp(str1,"upload_dir") == 0) {
    strcpy(upload_dir,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_7plus_dir") == 0) {
    strcpy(tnt_7plus_dir,str2);
    return(0);
  }
  else if (strcmp(str1,"yapp_dir") == 0) {
    strcpy(yapp_dir,str2);
    return(0);
  }
  else if (strcmp(str1,"altstat") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    altstat = tmp;
    return(0);
  }
  else if (strcmp(str1,"disc_on_start") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    disc_on_start = tmp;
    return(0);
  }
#endif
  else if (strcmp(str1,"download_dir") == 0) {
    strcpy(download_dir,str2);
    return(0);
  }
#ifndef DPBOXT
#ifdef HAVE_SOCKET  
  else if (strcmp(str1,"sock_passfile") == 0) {
    strcpy(sock_passfile,str2);
    return(0);
  }
#endif
#ifdef GEN_NEW_USER
  else if (strcmp(str1,"unix_new_user") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    unix_new_user = tmp;
    return(0);
  }
  else if (strcmp(str1,"unix_user_dir") == 0) {
    strcpy(unix_user_dir,str2);
    return(0);
  }
  else if (strcmp(str1,"unix_first_uid") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    unix_first_uid = tmp;
    return(0);
  }
  else if (strcmp(str1,"unix_user_gid") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    unix_user_gid = tmp;
    return(0);
  }
  else if (strcmp(str1,"unix_user_max") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    unix_user_max = tmp;
    return(0);
  }
#endif
#endif /* DPBOXT */
  else if (strcmp(str1,"func_key_file") == 0) {
    strcpy(func_key_file,str2);
    return(0);
  }
  else if (strcmp(str1,"macrotext_dir") == 0) {
    strcpy(macrotext_dir,str2);
    return(0);
  }
#ifdef USE_IFACE
  else if (strcmp(str1,"box_socket") == 0) {
    strcpy(box_socket,str2);
    return(0);
  }
#ifndef DPBOXT
  else if (strcmp(str1,"node_socket") == 0) {
    /* just ignore */
    return(0);
  }
  else if (strcmp(str1,"frontend_socket") == 0) {
    strcpy(frontend_socket,str2);
    return(0);
  }
  else if (strcmp(str1,"newmaildir") == 0) {
    strcpy(newmaildir,str2);
    return(0);
  }
  else if (strcmp(str1,"autobox_dir") == 0) {
    strcpy(autobox_dir,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_boxender") == 0) {
    strcpy(tnt_boxender,str2);
    return(0);
  }
  else if (strcmp(str1,"f6fbb_box") == 0) {
    strcpy(f6fbb_box,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_box_ssid") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    tnt_box_ssid = tmp;
    return(0);
  }
  else if (strcmp(str1,"tnt_box_call") == 0) {
    if (strlen(str2) > 9) return(1);
    strcpy(tnt_box_call,str2);
    for (tmp=0;tmp<strlen(tnt_box_call);tmp++)
      tnt_box_call[tmp] = toupper(tnt_box_call[tmp]);
    return(0);
  }
  else if (strcmp(str1,"tnt_node_ssid") == 0) {
    /* just ignore */
    return(0);
  }
  else if (strcmp(str1,"tnt_node_call") == 0) {
    /* just ignore */
    return(0);
  }
#ifdef BCAST
  else if (strcmp(str1,"tnt_bctempdir") == 0) {
    strcpy(tnt_bctempdir,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_bcsavedir") == 0) {
    strcpy(tnt_bcsavedir,str2);
    return(0);
  }
  else if (strcmp(str1,"tnt_bcnewmaildir") == 0) {
    strcpy(tnt_bcnewmaildir,str2);
    return(0);
  }
#endif /* BCAST */
#ifdef USE_AX25K
  else if (strcmp(str1,"ax25k_port") == 0) {
    strcpy(ax25k_port,str2);
    ax25k_active = 1;
    return(0);
  }
  else if (strcmp(str1,"fullmoni_flag") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    fullmoni_flag = tmp;
    return(0);
  }
  else if (strcmp(str1,"moni_socket") == 0) { /* DH3MB */
    strcpy(moni_socket,str2);
    return(0);
  }
#endif
#endif /* !DPBOXT */
#endif
  else {
    return(1);
  }
}

#ifndef DPBOXT
int init_proc()
{
  FILE *fp;
  char file[160];
  pid_t pid;

  strcpy(file,proc_file);
  fp = fopen(file,"w+");
  if (fp == NULL) {
    printf(_("ERROR: Can't create process file\n\n"));
    return(1);
  }
  pid = getpid();
  fprintf(fp,"%d",pid);
  fclose(fp);
  return(0);
}

void exit_proc()
{
  char file[160];

  strcpy(file,proc_file);
  unlink(file);
}
#endif

static void add_dir(char *dir, char *str)
{
  char temp[MAXCHAR];
  
  if (str[0] == '\0')
    return;
  if (str[0] != '/') {
    strcpy(temp,dir);
    strcat(temp,str);
    strcpy(str,temp);
  }
}

static int check_dir(char *str,char *name)
{
  DIR *ptr;
  
  ptr = opendir(str);
  if (ptr == NULL) {
    printf(_("ERROR: %s is defined as %s, but directory is not existing\n"),
           name,str);
    return(1);
  }
  closedir(ptr);
  return(0);
}

static int add_dir_slash(char *dir, char *str,char *name)
{
  char temp[MAXCHAR];
  int len;
  
  if (str[0] == '\0') {
    strcpy(str,dir);
    return(check_dir(str,name));
  }
  
  if (str[0] != '/') {
    strcpy(temp,dir);
    strcat(temp,str);
    strcpy(str,temp);
  }
  len = strlen(str);
  if (str[len-1] != '/') {
    str[len] = '/';
    str[len+1] = '\0';
  }
  return(check_dir(str,name));
}

static int update_filenames()
{
  int res;
  
  res = 0;
  add_dir(tnt_conf_dir,tnt_termcapfile);
  add_dir(tnt_blcmdfile, tnt_work_dir);
#ifndef DPBOXT
  add_dir(tnt_conf_dir,tnt_upfile);
  add_dir(tnt_conf_dir,tnt_downfile);
#ifdef TNT_SOLARIS
  add_dir(tnt_work_dir,proc_file);
#else
  add_dir(tnt_proc_dir,proc_file);
#endif
  add_dir(tnt_conf_dir,rem_info_file);
  add_dir(tnt_doc_dir,rem_help_file);
  add_dir("",tnt_cookiefile); /*XXXX*/
  add_dir("",tnt_lockfile);   /*XXXX*/
  add_dir(tnt_conf_dir,tnt_telltext_file);
#ifdef TNT_SOUND
  add_dir(tnt_conf_dir,tnt_sound_file);
#endif
#endif
  add_dir(tnt_doc_dir,tnt_help_file);
#ifndef DPBOXT
  add_dir(tnt_conf_dir,news_file_name);
  add_dir(tnt_conf_dir,name_file_name);
  add_dir(tnt_conf_dir,route_file_name);
  add_dir(tnt_conf_dir,tnt_ctextfile);
  add_dir(tnt_conf_dir,tnt_qtextfile);
  res += add_dir_slash(tnt_work_dir,remote_dir,"remote_dir");
  res += add_dir_slash(tnt_work_dir,ctext_dir,"ctext_dir");
  res += add_dir_slash(tnt_work_dir,abin_dir,"abin_dir");
  add_dir(tnt_log_dir,tnt_logbookfile);
  add_dir(tnt_conf_dir,tnt_pwfile);
  add_dir(tnt_conf_dir,tnt_sysfile);
  add_dir(tnt_conf_dir,tnt_noremfile);
  add_dir(tnt_conf_dir,tnt_flchkfile);
  add_dir(tnt_conf_dir,tnt_notownfile);
  add_dir(tnt_conf_dir,tnt_autostartfile);
  add_dir(tnt_conf_dir,tnt_extremotefile);
  add_dir(tnt_log_dir,resy_log_file);
#ifdef DEBUGIO
  add_dir(tnt_work_dir,debugiof);
#endif
  add_dir(tnt_log_dir,tnt_errlog);
#ifdef BCAST
  add_dir(tnt_log_dir,bcast_log_file);
#endif
  res += add_dir_slash(tnt_work_dir,tnt_bin_dir,"tnt_bin_dir");
  res += add_dir_slash(tnt_work_dir,upload_dir,"upload_dir");
  res += add_dir_slash(tnt_work_dir,tnt_7plus_dir,"tnt_7plus_dir");
  res += add_dir_slash(tnt_work_dir,yapp_dir,"yapp_dir");
#endif
  res += add_dir_slash(tnt_work_dir,download_dir,"download_dir");
#ifndef DPBOXT
  add_dir(tnt_conf_dir,sock_passfile);
#ifdef GEN_NEW_USER
  res += add_dir_slash(tnt_work_dir,unix_user_dir,"unix_user_dir");
#endif /* GEN_NEW_USER */
#endif /* ! DPBOXT */
  add_dir(tnt_conf_dir,func_key_file);
  res += add_dir_slash(tnt_work_dir,macrotext_dir,"macrotext_dir");
/*  add_dir(tnt_proc_dir,box_socket); */
#ifndef DPBOXT
/*  add_dir(tnt_proc_dir,node_socket); */
/*  add_dir(tnt_proc_dir,frontend_socket); */
  res += add_dir_slash(tnt_work_dir,newmaildir,"newmaildir");
  add_dir(tnt_conf_dir,autobox_dir);
  add_dir(tnt_conf_dir,tnt_boxender);
  add_dir(tnt_conf_dir,f6fbb_box);
#ifdef BCAST
  res += add_dir_slash("",tnt_bctempdir,"tnt_bctempdir"); /*XXXX*/
  res += add_dir_slash(tnt_work_dir,tnt_bcsavedir,"tnt_bcsavedir");
  res += add_dir_slash(tnt_work_dir,tnt_bcnewmaildir,"tnt_bcnewmaildir");
#endif
#endif
  return(res > 0);
}

void list_filenames(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  cmd_display(mode,channel,"List of all configured filenames:",1);
  cmd_display(mode,channel,tnt_work_dir,1);
  cmd_display(mode,channel,tnt_conf_dir,1);
  cmd_display(mode,channel,tnt_doc_dir,1);
  cmd_display(mode,channel,tnt_doc_dir,1);
  cmd_display(mode,channel,tnt_log_dir,1);
  cmd_display(mode,channel,tnt_proc_dir,1);
  cmd_display(mode,channel,tnt_termcapfile,1);
#ifndef DPBOXT
  cmd_display(mode,channel,tnt_upfile,1);
  cmd_display(mode,channel,tnt_downfile,1);
  cmd_display(mode,channel,proc_file,1);
  cmd_display(mode,channel,rem_info_file,1);
  cmd_display(mode,channel,rem_help_file,1);
  cmd_display(mode,channel,tnt_cookiefile,1);
  cmd_display(mode,channel,tnt_lockfile,1);
  cmd_display(mode,channel,tnt_telltext_file,1);
#ifdef TNT_SOUND
  cmd_display(mode,channel,tnt_sound_file,1);
#endif
  cmd_display(mode,channel,tnt_session_log,1);
#endif
  cmd_display(mode,channel,tnt_help_file,1);
  cmd_display(mode,channel,tnt_blcmdfile,1);
#ifndef DPBOXT
  cmd_display(mode,channel,news_file_name,1);
  cmd_display(mode,channel,name_file_name,1);
  cmd_display(mode,channel,route_file_name,1);
  cmd_display(mode,channel,tnt_ctextfile,1);
  cmd_display(mode,channel,tnt_qtextfile,1);
  cmd_display(mode,channel,remote_dir,1);
  cmd_display(mode,channel,ctext_dir,1);
  cmd_display(mode,channel,abin_dir,1);
  cmd_display(mode,channel,tnt_logbookfile,1);
  cmd_display(mode,channel,tnt_pwfile,1);
  cmd_display(mode,channel,tnt_sysfile,1);
  cmd_display(mode,channel,tnt_noremfile,1);
  cmd_display(mode,channel,tnt_flchkfile,1);
  cmd_display(mode,channel,tnt_notownfile,1);
  cmd_display(mode,channel,tnt_autostartfile,1);
  cmd_display(mode,channel,tnt_extremotefile,1);
  cmd_display(mode,channel,resy_log_file,1);
#ifdef DEBUGIO
  cmd_display(mode,channel,debugiof,1);
#endif
  cmd_display(mode,channel,tnt_errlog,1);
#ifdef BCAST
  cmd_display(mode,channel,bcast_log_file,1);
#endif
  cmd_display(mode,channel,tnt_bin_dir,1);
  cmd_display(mode,channel,upload_dir,1);
  cmd_display(mode,channel,tnt_7plus_dir,1);
  cmd_display(mode,channel,yapp_dir,1);
#endif
  cmd_display(mode,channel,download_dir,1);
#ifndef DPBOXT
  cmd_display(mode,channel,sock_passfile,1);
#ifdef GEN_NEW_USER
  cmd_display(mode,channel,unix_user_dir,1);
#endif /* GEN_NEW_USER */
#endif /* ! DPBOXT */
  cmd_display(mode,channel,func_key_file,1);
  cmd_display(mode,channel,macrotext_dir,1);
  cmd_display(mode,channel,box_socket,1);
#ifndef DPBOXT
  cmd_display(mode,channel,frontend_socket,1);
  cmd_display(mode,channel,newmaildir,1);
  cmd_display(mode,channel,autobox_dir,1);
  cmd_display(mode,channel,tnt_boxender,1);
  cmd_display(mode,channel,f6fbb_box,1);
#ifdef BCAST
  cmd_display(mode,channel,tnt_bctempdir,1);
  cmd_display(mode,channel,tnt_bcsavedir,1);
  cmd_display(mode,channel,tnt_bcnewmaildir,1);
#endif
  cmd_display(mode,channel,tnt_logfile,1);
#endif
}

int read_init_file(argc,argv,unlock)
int argc;
char *argv[];
int *unlock;
{
  FILE *init_file_fp;
  int file_end;
  int file_corrupt;
  char line[82];
  char str1[82];
  char str2[82];
  char tmp_str[MAXCHAR];
  int rslt;
  /* int warning; */
  int wrong_usage;
  char *str_ptr;
  int scanned;
  int explized_ini;

  use_select = DEF_USE_SELECT;
#ifndef DPBOXT
  ax25k_active = 0;
#endif
#ifdef USE_AX25K
  ax25k_port[0] = '\0';
  fullmoni_flag = 0;
#endif
#ifndef DPBOXT
  strcpy(device,DEF_DEVICE);
  soft_tnc = 0;
#ifdef HAVE_SOCKET
  fixed_wait = DEF_FIXED_WAIT;
  amount_wait = DEF_AMOUNT_WAIT;
#endif
  speed = DEF_SPEED;
  speedflag = DEF_SPEEDFLAG;
  tnc_channels = DEF_TNC_CHANNELS + 1; /* include unproto channel */
  r_channels = DEF_R_CHANNELS;
  file_paclen = DEF_FILE_PACLEN;
  tnt_comp = DEF_TNT_COMP;
  moni_decomp = DEF_MONI_DECOMP;
  moni_chkbin = DEF_MONI_CHKBIN;
  ibm_umlaut_flag = DEF_TNT_IBM_UMLAUT;
#endif
  attc_normal = DEF_ATTC_NORMAL;
  attc_statline = DEF_ATTC_STATLINE;
  attc_monitor = DEF_ATTC_MONITOR;
  attc_cstatline = DEF_ATTC_CSTATLINE;
  attc_controlchar = DEF_ATTC_CONTROLCHAR;
  attc_remote = DEF_ATTC_REMOTE;
  attc_special = DEF_ATTC_SPECIAL;
  attc_monport1 = DEF_ATTC_MONPORT1;
  attc_monport2 = DEF_ATTC_MONPORT2;
  attm_normal = DEF_ATTM_NORMAL;
  attm_statline = DEF_ATTM_STATLINE;
  attm_monitor = DEF_ATTM_MONITOR;
  attm_cstatline = DEF_ATTM_CSTATLINE;
  attm_controlchar = DEF_ATTM_CONTROLCHAR;
  attm_remote = DEF_ATTM_REMOTE;
  attm_special = DEF_ATTM_SPECIAL;
  attm_monport1 = DEF_ATTM_MONPORT1;
  attm_monport2 = DEF_ATTM_MONPORT2;
#ifndef DPBOXT
  lines_command = DEF_LINES_COMMAND;
  lines_monitor = DEF_LINES_MONITOR;
  lines_input = DEF_LINES_INPUT;
  lines_output = DEF_LINES_OUTPUT;
  lines_moncon = DEF_LINES_MONCON;
  lines_r_input = DEF_LINES_R_INPUT;
  lines_r_output = DEF_LINES_R_OUTPUT;
  scr_divide = DEF_SCR_DIVIDE;
  strcpy(tnt_upfile,UP_FILE);
  strcpy(tnt_downfile,DWN_FILE);
  strcpy(proc_file,DEF_PROC_FILE);
  strcpy(rem_info_file,DEF_INFO_FILE);
  strcpy(rem_help_file,DEF_HELP_FILE);
  strcpy(tnt_telltext_file,DEF_TELLTEXT_FILE);
#ifdef TNT_SOUND
  strcpy(tnt_sound_file,DEF_SOUND_FILE);
#endif
  strcpy(tnt_session_log,DEF_SESSION_LOG);
#endif
  strcpy(tnt_help_file,DEF_TNT_HELP_FILE);
#ifndef DPBOXT
  strcpy(tnt_cookiefile,COOKIE_FILE);
  strcpy(tnt_lockfile,DEF_LOCK_FILE);
#endif
  color = DEF_COLOR;
  termcap = DEF_TERMCAP;
  strcpy(tnt_termcapfile,DEF_TNT_TERMCAPFILE);
  strcpy(tnt_blcmdfile,DEF_TNT_BLCMDFILE);
#ifndef DPBOXT
  strcpy(remote_user,DEF_REMOTE_USER);
  strcpy(news_file_name,DEF_NEWS_FILE_NAME);
  strcpy(name_file_name,DEF_NAME_FILE_NAME);
  strcpy(route_file_name,DEF_ROUTE_FILE_NAME);
  strcpy(tnt_ctextfile,DEF_TNT_CTEXTFILE);
  strcpy(tnt_qtextfile,DEF_TNT_QTEXTFILE);
#endif
  strcpy(tnt_work_dir,DEF_TNT_WORK_DIR);
  strcpy(tnt_conf_dir,DEF_TNT_CONF_DIR);
  strcpy(tnt_doc_dir,DEF_TNT_DOC_DIR);
  strcpy(tnt_doc_dir,DEF_TNT_HELP_DIR);
  strcpy(tnt_log_dir,DEF_TNT_LOG_DIR);
  strcpy(tnt_proc_dir,DEF_TNT_PROC_DIR);
  strcpy(tnt_editor,DEFAULT_EDITOR);
#ifndef DPBOXT
  strcpy(remote_dir,DEF_REMOTE_DIR);
  strcpy(ctext_dir,DEF_CTEXT_DIR);
  strcpy(abin_dir,DEF_ABIN_DIR);
  strcpy(tnt_logbookfile,DEF_TNT_LOGBOOKFILE);
  strcpy(tnt_pwfile,DEF_TNT_PWFILE);
  strcpy(tnt_sysfile,DEF_TNT_SYSFILE);
  strcpy(tnt_noremfile,DEF_TNT_NOREMFILE);
  strcpy(tnt_flchkfile,DEF_TNT_FLCHKFILE);
  strcpy(tnt_notownfile,DEF_TNT_NOTOWNFILE);
  strcpy(tnt_autostartfile,DEF_TNT_AUTOSTARTFILE);
  strcpy(tnt_extremotefile,DEF_TNT_EXTREMOTEFILE);
  pty_timeout = DEF_PTY_TIMEOUT;
#ifdef BCAST
  strcpy(tnt_bctempdir,DEF_TNT_BCTEMPDIR);
  strcpy(tnt_bcsavedir,DEF_TNT_BCSAVEDIR);
  strcpy(tnt_bcnewmaildir,DEF_TNT_BCNEWMAILDIR);
#endif
#endif
  auto_newline = DEF_AUTO_NEWLINE;
  supp_hicntl = DEF_SUPP_HICNTL;
#ifndef DPBOXT
  strcpy(resy_log_file,DEF_RESY_LOG_FILE);
  strcpy(tnt_errlog,DEF_TNT_ERRLOG);
#ifdef BCAST
  strcpy(bcast_log_file,DEF_BCAST_LOG_FILE);
#endif
  strcpy(tnt_bin_dir,DEF_BIN_DIR);
  strcpy(upload_dir,DEF_UPLOAD_DIR);
  strcpy(tnt_7plus_dir,DEF_TNT_7PLUS_DIR);
  strcpy(yapp_dir,DEF_YAPP_DIR);
  altstat = DEF_ALTSTAT;
  disc_on_start = DEF_DISC_ON_START;
#endif
  strcpy(download_dir,DEF_DOWNLOAD_DIR);
#ifndef DPBOXT
#ifdef HAVE_SOCKET
  strcpy(sock_passfile,DEF_SOCKPASS_FILE);
#endif
#ifdef GEN_NEW_USER
  unix_new_user = DEF_UNIX_NEW_USER;
  strcpy(unix_user_dir,DEF_UNIX_USER_DIR);
  unix_first_uid = DEF_UNIX_FIRST_UID;
  unix_user_gid = DEF_UNIX_USER_GID;
#endif
#endif /* DPBOXT */
  strcpy(func_key_file,DEF_FUNC_KEY_FILE);
  strcpy(macrotext_dir,DEF_MACROTEXT_DIR);
#ifdef USE_IFACE
  strcpy(box_socket,DEF_BOX_SOCKET);
#ifndef DPBOXT
  strcpy(newmaildir,DEF_NEWMAILDIR);
  strcpy(autobox_dir,DEF_AUTOBOX_DIR);
  strcpy(tnt_boxender,DEF_TNT_BOXENDER);
  strcpy(f6fbb_box,DEF_F6FBB_BOX);
  lines_mbinput = DEF_LINES_MBINPUT;
  lines_mboutput = DEF_LINES_MBOUTPUT;
  tnt_box_ssid = DEF_TNT_BOX_SSID;
  strcpy(tnt_box_call,DEF_TNT_BOX_CALL);
  strcpy(frontend_socket,DEF_FRONTEND_SOCKET);
#endif
  mbscr_divide = DEF_MBSCR_DIVIDE;
#endif
  input_linelen = DEF_INPUT_LINELEN;
  insertmode = DEF_INSERTMODE;
#ifndef DPBOXT
  lines_xmon = DEF_LINES_XMON;
  lines_xmon_pre = DEF_LINES_XMON_PRE;
  xmon_scr_divide = DEF_XMON_SCR_DIVIDE;
  num_heardentries = DEF_NUM_HEARDENTRIES;
#endif
  
  tnt_initfile[0] = '\0';
#ifdef DPBOXT
  box_socket2[0] = '\0';
#else
  strcpy(tnt_logfile,"");
#endif
  wrong_usage = 0;
  scanned = 1;
  explized_ini=0;
  while ((scanned < argc) && (!wrong_usage)) {
    if (strcmp(argv[scanned],"-i") == 0) {
      scanned++;
      if (scanned < argc) {
        strcpy(tnt_initfile,argv[scanned]);
        explized_ini=1;
      }
      else wrong_usage = 1;
    }
#ifdef DPBOXT
    else if (strcmp(argv[scanned],"-s") == 0) {
      scanned++;
      if (scanned < argc) {
        strcpy(box_socket2,argv[scanned]);
      }
      else wrong_usage = 1;
    }
#else
    else if (strcmp(argv[scanned],"-l") == 0) {
      scanned++;
      if (scanned < argc) {
        strcpy(tnt_logfile,argv[scanned]);
      }
      else wrong_usage = 1;
    }
    else if (strcmp(argv[scanned],"-u") == 0) {
      *unlock = 1;
    }
    else if (strcmp(argv[scanned],"-d") == 0) {
      tnt_daemon = 1;
    }
#endif
    else {
      wrong_usage = 1;
    }
    scanned++;
  }
  if (wrong_usage) {
#ifdef DPBOXT
    printf(_("Usage : dpboxt [-i <init-file>] [-s <box-socket>]\n"));
#else
    printf(_("Usage : tnt [-i <init-file>] [-l <log-file>] [-u] [-d]\n"));
#endif
    return(1);
  }
  
  /* warning = 0; */

/* WSPse: Try to find a tnt configuration file (22.2.1999) */
  if (explized_ini == 1) {   /* Explizites File angegeben */
    if (!(init_file_fp = fopen(tnt_initfile,"r"))) {
      printf(_("ERROR: explizit configuration \"%s\" not found.\n\n"),
              tnt_initfile);
      return(1);
    }
    strcpy(tnt_realinitfile,tnt_initfile);
  } else {                        /* Try to find inifile in another location */
    str_ptr = getenv("HOME");
    if(str_ptr != NULL) {         /* Try to find in Home */
      strcpy(tmp_str, str_ptr);
      strcat(tmp_str, "/.tnt/");
      strcat(tmp_str, INIT_FILE);
      init_file_fp = fopen(tmp_str,"r");
      strcpy(tnt_realinitfile, tmp_str);
      if(init_file_fp == NULL) {
        strcpy(tmp_str, getenv("HOME"));
        strcat(tmp_str, "/.tnt/");
        strcat(tmp_str, INIT_FILE2);
        init_file_fp = fopen(tmp_str,"r");
        strcpy(tnt_realinitfile, tmp_str);
      }
    }

    if(init_file_fp == NULL) {    /* Try to find in default Dir */
      strcpy(tmp_str, INIT_FILE);
      init_file_fp = fopen(tmp_str,"r");
      strcpy(tnt_realinitfile, tmp_str);
    }
    if(init_file_fp == NULL) {
      strcpy(tmp_str, INIT_FILE2);
      init_file_fp = fopen(tmp_str,"r");
      strcpy(tnt_realinitfile, tmp_str);
    }

    if(init_file_fp == NULL) {    /* Try to find in Installpath (hardcoded) */
      strcpy(tmp_str, TNT_INIT_PATH);
      strcat(tmp_str, "/");
      strcat(tmp_str, INIT_FILE);
      init_file_fp = fopen(tmp_str,"r");
      strcpy(tnt_realinitfile, tmp_str);
    }
    if(init_file_fp == NULL) {
      strcpy(tmp_str, TNT_INIT_PATH);
      strcat(tmp_str, "/");
      strcat(tmp_str, INIT_FILE2);
      init_file_fp = fopen(tmp_str,"r");
      strcpy(tnt_realinitfile, tmp_str);
    }

    if(init_file_fp == NULL) {    /* Try to find in /etc */
      strcpy(tmp_str, "/etc/");
      strcat(tmp_str, INIT_FILE);
      init_file_fp = fopen(tmp_str,"r");
      strcpy(tnt_realinitfile, tmp_str);
    }

    if(init_file_fp == NULL) {    /* give up */
      printf(_("ERROR: no configuration file found\n"));
      return(1);
    }
  }
  file_end = 0;
  file_corrupt = 0;
  while (!file_end) {
    if (fgets(line,82,init_file_fp) == NULL) {
      file_end = 1;
    }
    else {
      if (strlen(line) == 82) {
        file_end = 1;
        file_corrupt = 1;
      }
      else {
        if (line[0] != '#') { /* ignore comment-lines */
          rslt = sscanf(line,"%s %s",str1,str2);
          switch (rslt) {
          case EOF: /* ignore blank lines */
            break;
          case 2:
            if (analyse_value(str1,str2)) {
              file_end = 1;
              file_corrupt = 1;
            }
            break;
          default:
            file_end = 1;
            file_corrupt = 1;
            break;
          }
        }
      }
    }
  }
  fclose(init_file_fp);
  if (file_corrupt) {
    if (line == NULL) line[0] = '\0';
    printf(_("ERROR: %s is in wrong format, wrong line:\n%s\n"
           "       a lot of stuff has changed in TNT 1.9, maybe you are\n"
           "       using an old configuration file, please take a look at\n"
           "       the example files\n"),
           tnt_realinitfile,line);
    return(1);
  }
  color_save = color;
  termcap_save = termcap;
  supp_hicntl_save = supp_hicntl;
  auto_newline_save = auto_newline;
  if (check_dir(tnt_work_dir,"tnt_work_dir")) return(1);
  if (check_dir(tnt_conf_dir,"tnt_conf_dir")) return(1);
  if (check_dir(tnt_doc_dir,"tnt_doc_dir")) return(1);
  if (check_dir(tnt_doc_dir,"tnt_doc_dir")) return(1);
  if (check_dir(tnt_log_dir,"tnt_log_dir")) return(1);
  if (check_dir(tnt_proc_dir,"tnt_proc_dir")) return(1);
#ifdef TNT_SOUND
  if (check_dir(tnt_sound_dir,"tnt_sound_dir")) return(1);
#endif
  if (update_filenames()) return(1);
#ifdef DPBOXT
  if (box_socket2[0] != '\0') {
    strcpy(box_socket,box_socket2);
    add_dir(tnt_proc_dir,box_socket);
  }
#endif
  return(0);
}
