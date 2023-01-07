/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993-1999 by Mark Wahl
   For license details see documentation
   Main procedure (main.c)
   created: Mark Wahl DL4YBG 93/08/07
   updated: Mark Wahl DL4YBG 97/04/06
   updated: Mark Wahl DL4YBG 98/01/06
   updated: Matthias Hensler WS1LS 2000/07/10
   updated: Johann Hanne DH3MB 99/01/02
   updated: mayer hans oe1smc, 99/01/06
   updated: Peter Mack DL3FCJ 99/03/23
   updated: Boris Bruns DO2BBC 22/01/07
*/

#include "tnt.h"

#ifdef TNT_SOUND
 #ifdef USE_AX25K
  #ifdef BCAST
   #define TNT_VERSION_ADD "(AX25K,BC,Sound)"
  #else
   #define TNT_VERSION_ADD "(AX25K,Sound)"
  #endif
 #else
  #ifdef BCAST
   #define TNT_VERSION_ADD "(BC,Sound)"
  #else
   #define TNT_VERSION_ADD "(Sound)"
  #endif
 #endif
#else
 #ifdef USE_AX25K
  #ifdef BCAST
   #define TNT_VERSION_ADD "(AX25K,BC)"
  #else
   #define TNT_VERSION_ADD "(AX25K)"
  #endif
 #else
  #ifdef BCAST
   #define TNT_VERSION_ADD "(BC)"
  #else
   #define TNT_VERSION_ADD ""
  #endif
 #endif
#endif

/* external function declarations */
extern int alloc_boxlist();
extern int alloc_connect();
extern int alloc_display();
extern int alloc_file();
extern int alloc_iface();
extern int alloc_monbox();
extern int alloc_priv();
extern int alloc_serial();
extern int alloc_shell();
extern void bc_timing();
extern void bcast_exit();
extern struct sockaddr *build_sockaddr();
extern void check_xconnect_timeout();
extern void check_xmon_timeout();
extern void exit_blist();
extern void exit_file();
extern void exit_help();
extern void exit_iface();
extern void exit_keyboard();
extern void exit_log();
extern void exit_mb();
extern void exit_mheard();
extern void exit_priv();
extern void exit_proc();
extern void exit_remote();
extern void exit_screen();
extern int exit_serial();
extern void exit_shell();
extern void exit_tnt();
extern void exit_xconnect();
extern void exit_xmon();
extern void flush_frontend();
extern void free_boxlist();
extern void free_connect();
extern void free_display();
extern void free_file();
extern void free_iface();
extern void free_monbox();
extern void free_priv();
extern void free_serial();
extern void free_shell();
extern void frontend_exit();
extern void iface_fdset();
extern int iface_receive();
extern void iface_trying();
extern int init_bcast();
extern int init_blist();
extern void init_conv();
extern void init_cookie();
extern void init_crc();
extern void init_file();
extern void init_fr_buffer();
extern void init_help();
extern void init_iface();
extern void init_keyboard();
extern void init_log();
extern void init_mb();
extern void init_mheard();
extern void init_priv();
extern int init_proc();
extern void init_remote();
extern int init_screen();
extern int init_serial();
extern void init_session();
extern void init_shell();
extern void init_xconnect();
extern void init_xmon();
extern void keyboard_server();
extern void open_logfile();
extern int read_init_file();
extern void reinit_screen();
extern void resync_tnc();
extern int serial_server();
extern void set_linescols();
extern void shell_fdset();
extern int shell_receive();
extern void term_exit();
extern int term_init();
extern void uwait();
extern void yapp_timeout();

extern int serial;
#ifndef DPBOXT
extern char device[];
extern int soft_tnc;
extern unsigned int speed;
extern int speedflag;
extern char tnt_logfile[];
#endif
extern char tnt_errlog[];
#ifndef DPBOXT
extern char frontend_socket[];
extern int frontend_active;
extern int frontend_fd;
extern int frontend_sockfd;
#endif
extern int tnt_daemon;
extern int use_select;
extern int softtnc_error;
extern int ax25k_active;
#ifdef USE_AX25K
extern int ax25k_poll;
#endif
extern struct staterr staterr;

int act_channel;
int act_mode;
int act_state;
int is_root;
int terminated;
int errfd;
#ifdef DEBUGIO
int fddebugio ; 
#endif

char frontend_para[MAXCHAR];

char signon[] =
"TNT "TNT_VERSION TNT_VERSION_ADD" Packet Radio Terminal\015"
#ifdef TNT_LINUX
"LINUX"
#endif
#ifdef TNT_ISC
"ISC"
#endif
#ifdef TNT_SOLARIS
"SOLARIS"
#endif
#ifdef TNT_NETBSD
"NetBSD"
#endif
"-Version, Created: "TNT_DATE"\015"
"Copyright (C) 1993-99 by Mark Wahl, DL4YBG\015"
"Further development by Matthias Hensler, WSPse 1998-2002\015"
"Changes from version 1.2 to 1.3 by Johann Hanne, DH3MB\015"
"GnuPG/PGP extensions by Walter Koch, DG9EP\015"
"\015Maintainers: Berndt Josef Wulf (VK5ABN), Hans Mayer (OE1SMC), Matthias Hensler\015"
"Thanks to many people for testing, suggestions and ideas\015"
"forked by: Boris Bruns (DO2BBC)\015"
"Find new versions at https://github.com/do2bbc/tnt\015"
"Send bugreports to mailto:do2bbc@darc.de\015\015"

#ifdef TNT_SOLARIS
"SOLARIS version port by mayer hans OE1SMC 9/1998-2/2000 \015"
#endif
#ifdef TNT_NETBSD
"NetBSD version port by Berndt Josef Wulf, VK5ABN\015"
#endif
"For license details see documentation.\015"
"To get help: <ALT> H or <Esc> H\015"
"Compiled on " __DATE__ "\015";

char add_signon[] =
"No root permissions: SHELL command disabled\015";

char rem_ver_str[] =
"TNT/"
#ifdef TNT_LINUX
"LINUX"
#endif
#ifdef TNT_ISC
"ISC"
#endif
#ifdef TNT_SOLARIS
"SOLARIS"
#endif
" "TNT_VERSION TNT_VERSION_ADD" ("TNT_DATE", DL4YBG and others)";

void free_all()
{
  free_connect();
#ifndef DPBOXT
  free_boxlist();
  free_display();
  free_file();
  free_iface();
#endif
  free_monbox();
  free_priv();
#ifndef DPBOXT
  free_serial();
#endif
  free_shell();
#ifdef USE_AX25K
  free_ax25k();
#endif
}

int alloc_all()
{
  int i;

  i =  alloc_connect();
#ifndef DPBOXT
  i += alloc_boxlist();
  i += alloc_display();
  i += alloc_file();
  i += alloc_iface();
#endif
  i += alloc_monbox();
  i += alloc_priv();
#ifndef DPBOXT
  i += alloc_serial();
#endif
  i += alloc_shell();
#ifdef USE_AX25K
  i += alloc_ax25k();
#endif
  return (i>0);
}

static void sigterm()
{
  terminated = 1;
  signal(SIGTERM, SIG_IGN);
}

static void sighup()
{
  terminated = 1;
  signal(SIGHUP, SIG_IGN);
}

int 
main(argc,argv)
int argc;
char *argv[];
{
  int len;
  char rdbuffer[259];
  char ch;
  int unlock;
  int result;
  fd_set fdmask;
  struct timeval timevalue;
  fd_set dummy_mask;
  int served;
  int max_fd;
  int count;
  int kbd_res;
  int ser_res;
  time_t ser_resp_time;
  int shell_res;
  int empty_rounds;
  int ser_rounds;
  int resync_needed;
  int iface_res;
  int fr_lines;
  int fr_cols;
  int fr_error;
  int parm;
  char hlpstr[10];
  char fr_con[MAXCHAR];
#ifdef USE_IFACE
  int servlen;
  struct sockaddr *saddr;
  int arg;
#endif

  umask(0002); /* remove write permission for 'world' */
               /* to fix wrong socket permissions  (DG1RTF) */

  is_root = (geteuid() == 0);
  
  tnt_daemon = 0;
#ifndef DPBOXT
  frontend_active = 0;
#endif
  terminated = 0;
  
  unlock = 0; /* set a default value for sure ! DL3FCJ */
 
  /* startup national language support */
#ifdef HAVE_NLS
  setlocale(LC_MESSAGES, "");
  bindtextdomain( (char *) PACKAGE, LOCALEDIR);
  textdomain( (char *) PACKAGE);
#endif
  
  if (read_init_file(argc,argv,&unlock))
    exit(1);

  errfd = open(tnt_errlog,O_RDWR|O_CREAT|O_APPEND,0600);
  if (errfd == -1) {
    printf(_("ERROR: cannot create/open file for errorlog: %s\n"),tnt_errlog);
    exit(1);
  }
  
  dup2(errfd,STDERR_FILENO);
  close(errfd);
    
  if (!tnt_daemon) {
    if (term_init("",0,0))
      exit(1);
  }
  else
    set_linescols();
    
  if (alloc_all())
    exit(1);

  if (ax25k_active) {
#ifdef USE_AX25K
    init_xconnect();
    if (init_ax25k()) {
      free_all();
      exit(1);
    }
#endif
  }
#ifndef DPBOXT
  else {  
    if (init_serial(device,speed,speedflag,unlock)) {
      free_all();
      exit(1);
    }
  }
#endif
  
  if (!tnt_daemon) {
#ifndef DPBOXT
    if (init_proc()) {
      term_exit();
      exit_serial();
      free_all();
      exit(1);
    }
#endif
    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, sighup);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTERM, sigterm);
  }
  else {
#ifndef DPBOXT
#ifdef USE_IFACE
    saddr = (struct sockaddr *) build_sockaddr(frontend_socket,&servlen);
    if (!saddr) {
      printf(_("ERROR: invalid definition of frontend socket\n"));
      exit(1);
    }
    if ((frontend_sockfd = socket(saddr->sa_family,SOCK_STREAM,0)) < 0) {
      printf(_("ERROR: Can't open stream socket\n"));
      exit(1);
    }

    switch (saddr->sa_family) {
    case AF_UNIX:
      unlink(saddr->sa_data);
      break;     
    case AF_INET:
      arg = 1;
      setsockopt(frontend_sockfd,SOL_SOCKET,
                 SO_REUSEADDR,(char *) &arg,sizeof(arg));
      break;     
    }              
                                             
    if (bind(frontend_sockfd,saddr,servlen) < 0) {
      printf(_("ERROR: Can't bind socket\n"));
      exit(1);
    }
  
    listen(frontend_sockfd,5);

    if (!use_select)
      fcntl(frontend_sockfd,F_SETFL,O_NONBLOCK);
  
    printf(_("%s successfully started\n"),
            rem_ver_str);
    if (fork() != 0)
      exit(0);
    if (init_proc()) {
      close(frontend_sockfd);
      exit_serial();
      free_all();
      exit(1);
    }
#endif
#endif    /* DPBOXT */
    close(0);
    close(1);
    /*close(2);*/
    chdir("/");
    setsid();
    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, sighup);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTERM, sigterm);
  }

  init_file();
  init_screen();  
  init_keyboard();
  init_help();
  init_priv();
  if (!ax25k_active)
    init_xconnect();
  init_mheard();
  init_xmon();
  init_conv();
  init_remote();
  init_cookie();
  init_crc();
  init_shell();
  init_session(); /* WSP */
#ifdef USE_IFACE
  init_iface();
  init_mb();
#endif
  init_blist();
  init_log();
#ifdef DEBUGIO
  init_debugio() ;
#endif
#ifdef BCAST
  init_bcast();
#endif
 
#ifndef DPBOXT 
  if (strcmp(tnt_logfile,"") != 0) {
    open_logfile(RX_NORM,RX_RCV,1,strlen(tnt_logfile),
                 M_CMDSCRIPT,tnt_logfile);
  }
#endif
  if (!use_select) {
    ser_resp_time = time(NULL);
    ser_rounds = 0;
    empty_rounds = 0;
  }
  if (ax25k_active) {
#ifdef USE_AX25K
    act_state = S_TNT_UP;
    init_statline();
    open_upfile(&act_state);
#endif
  }
#ifndef DPBOXT
  else {
    act_state = S_INIT;
    len = 1;
    serial_server(&act_state,rdbuffer,len);
  }
#endif
  
  while (act_state) {
    if (terminated) {
      exit_tnt(0,0,0,0,M_REMOTE,NULL);
      terminated = 0;
    }
#ifdef USE_IFACE
    iface_trying();
#endif
    flush_frontend();
    check_xmon_timeout();
    check_xconnect_timeout();
    yapp_timeout();
#ifdef BCAST
    bc_timing();
#endif
#ifdef USE_AX25K
    if (ax25k_active) {
      handle_tx_queue();
      status_ax25k(0);
    }
#endif
    if (use_select) {
      FD_ZERO(&fdmask);
      if (!tnt_daemon) {
        FD_SET(0,&fdmask);
        max_fd = 1;
      }
#ifndef DPBOXT
      else {
        if (frontend_active) {
          FD_SET(frontend_fd,&fdmask);
          max_fd = frontend_fd + 1;
        }
      }
#endif
      if (ax25k_active) {
#ifdef USE_AX25K
        ax25k_fdset(&max_fd,&fdmask);
#endif
      }
      else {
        FD_SET(serial,&fdmask);
        if (serial > max_fd - 1)
          max_fd = serial + 1;
      }
      shell_fdset(&max_fd,&fdmask);
#ifdef USE_IFACE
      iface_fdset(&max_fd,&fdmask);
#endif
      if (ax25k_active) {
#ifdef USE_AX25K
        timevalue.tv_usec = ax25k_poll;
        timevalue.tv_sec = 0;
#endif
      }
      else {
        timevalue.tv_usec = 0;
        timevalue.tv_sec = RESY_TIME;
      }
      count = select(max_fd,&fdmask,
                    (fd_set *) 0,(fd_set *) 0,&timevalue);
      if (count == -1) {
        continue;
      }
      served = 0;
      resync_needed = 0;
      if (!tnt_daemon) {
        if (FD_ISSET(0,&fdmask)) {
          if (read(0,&ch,1) == 1) {
            keyboard_server(&act_state,&ch);
          } 
          served = 1;
        }
      }
      else {
#ifndef DPBOXT
        if (frontend_active) {
          if (FD_ISSET(frontend_fd,&fdmask)) {
            result = read(frontend_fd,&ch,1);
            if (result == 1) {
              if (frontend_active == 2) {
                fr_error = 1;
                if (ch != '\n') {
                  if (strlen(frontend_para) < 79) {
                    hlpstr[0] = ch;
                    hlpstr[1] = '\0';
                    strcat(frontend_para,hlpstr);
                    fr_error = 0;
                  }
                }
                else {
                  parm = sscanf(frontend_para,"%s %d %d",
                                fr_con,&fr_lines,&fr_cols);
                  if (parm == 3) {
                    if (!term_init(fr_con,fr_lines,fr_cols)) {
                      frontend_active = 1;
                      init_fr_buffer();
                      reinit_screen();
                      fr_error = 0;
                    }
                  }
                }
                if (fr_error) {
                  frontend_active = 0;
                  close(frontend_fd);
                }
              }
              else {
                keyboard_server(&act_state,&ch);
              }
            }
            served = 1;
          }
        }
#endif
      }
      if (ax25k_active) {
#ifdef USE_AX25K
        if (!ax25k_receive(&fdmask)) {
          served = 1;
        }
#endif
      }
#ifndef DPBOXT
      else {
        if (FD_ISSET(serial,&fdmask)) {
          if ((len = read(serial,rdbuffer,259)) > 0) {
            resync_needed = serial_server(&act_state,rdbuffer,len);
          }
          else {
            if ((soft_tnc) && (len == 0)) {
              act_state = S_END;
              softtnc_error = 1;
            }
          }
          served = 1;
        }
      }
#endif
      if (!shell_receive(&fdmask)) {
        served = 1;
      }
#ifdef USE_IFACE
      if (!iface_receive(&fdmask)) {
        served = 1;
      }
#endif
#ifndef DPBOXT
      if (!ax25k_active) {
        if ((!served) || (resync_needed)) {
          resync_tnc(&act_state);
        }
      }
#endif
    }
    else {
      ser_res = 1;
      resync_needed = 0;
      if (ax25k_active) {
#ifdef USE_AX25K
        ser_res = ax25k_receive(&dummy_mask);
#endif
#ifndef DPBOXT
      }
      else {
        if ((len = read(serial,rdbuffer,259))) {
          if (len > 0) {
            resync_needed = serial_server(&act_state,rdbuffer,len);
            ser_res = 0;
            ser_resp_time = time(NULL);
            ser_rounds = 0;
          }
        }
        else {
          if ((soft_tnc) && (len == 0)) {
            act_state = S_END;
            softtnc_error = 1;
          }
        }
#endif
      }
      kbd_res = 1;
      if (!tnt_daemon) {
        if (read(0,&ch,1)) {
          keyboard_server(&act_state,&ch);
          kbd_res = 0;
        }
      }
      else {
#ifndef DPBOXT
        if (frontend_active) {
          result = read(frontend_fd,&ch,1);
          if (result == 1) {
            if (frontend_active == 2) {
              fr_error = 1;
              if (ch != '\n') {
                if (strlen(frontend_para) < 79) {
                  hlpstr[0] = ch;
                  hlpstr[1] = '\0';
                  strcat(frontend_para,hlpstr);
                  fr_error = 0;
                }
              }
              else {
                parm = sscanf(frontend_para,"%s %d %d",
                              fr_con,&fr_lines,&fr_cols);
                if (parm == 3) {
                  if (!term_init(fr_con,fr_lines,fr_cols)) {
                    frontend_active = 1;
                    init_fr_buffer();
                    reinit_screen();
                    fr_error = 0;
                  }
                }
              }
              if (fr_error) {
                frontend_active = 0;
                close(frontend_fd);
              }
            }
            else {
              keyboard_server(&act_state,&ch);
            }
            kbd_res = 0;
          }
        }
#endif
      }
      shell_res = shell_receive(&dummy_mask);
#ifdef USE_IFACE
      iface_res = iface_receive(&dummy_mask);
#else
      iface_res = 1;
#endif
      if ((!ax25k_active) && (resync_needed)) {
        resync_tnc(&act_state);
        ser_resp_time = time(NULL);
        ser_rounds = 0;
        empty_rounds = 0;
      }
      else {
      /* if no serial processing and no key pressed: sleep */
        if (ser_res && kbd_res && shell_res && iface_res) {
          if (empty_rounds > 10) {
            empty_rounds = 0;
            uwait(10000); /* sleep for 10ms */
          }
          else {
            empty_rounds++;
          }
          if (!ax25k_active) {
            ser_rounds++;
            if (ser_rounds > 100*RESY_TIME) {
              if ((time(NULL) - ser_resp_time) >= RESY_TIME) {
                resync_tnc(&act_state);
                ser_resp_time = time(NULL);
                ser_rounds = 0;
                empty_rounds = 0;
              }
            }
          }
        }
      }
    }
  }

  exit_file();
#ifdef BCAST
  bcast_exit();
#endif
  exit_log();
#ifdef DEBUGIO
  exit_debugio();
#endif
  exit_blist();
#ifdef USE_IFACE
  exit_mb();
  exit_iface();
#endif
  exit_shell();
  exit_priv();
  exit_remote();
  exit_xmon();
  exit_mheard();
  exit_xconnect();
  exit_help();
  exit_keyboard();
  exit_screen();
#ifndef DPBOXT
  exit_proc();
#endif
  if (tnt_daemon) {
#ifndef DPBOXT
#ifdef USE_IFACE
    if (frontend_active) {
      frontend_exit(1);
    }
    close(frontend_sockfd);
/*
    strcpy(serv_addr.sun_path,frontend_socket);
    unlink(serv_addr.sun_path);
*/
#endif
#endif
  }
  else {
    term_exit();
  }
  if (ax25k_active) {
#ifdef USE_AX25K
    exit_ax25k();
#endif
  }
  else {
    if (exit_serial()) {
      free_all();
      exit(1);
    }
  }
  free_all();
  exit(0);
}
