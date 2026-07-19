/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993-1997 by Mark Wahl
   For license details see documentation
   Procedures for logbook (log.c)
   created: Mark Wahl DL4YBG 94/03/12
   updated: Mark Wahl DL4YBG 97/01/26
   updated: mayer hans oe1smc, 6.1.1999
   updated: Matthias Hensler WS1LS, 23.5.1999

   $Id: log.c,v 1.2 2001/11/28 22:37:51 cvs-tnt Exp $
*/


#include "tnt.h"
#include "connect.h"
#include "ifacedef.h"
#include <time.h>

/* external function declarations */
extern void cmd_display();

extern int tnc_channels;
extern struct channel_stat *ch_stat;
extern struct sqrg_info qrg_info[] ; 

char tnt_logbookfile[MAXCHAR];
int logbook_active;
int logbook_flag;

static char log_error_text[] =
     "Warning: Can't write to Logbook, function now disabled!";
     
static char loghead[] =
     "CALL      QRG        Starttime   |    Endtime     |       "
     "   Callsign       \n";
     
static char logdelim[] =
     "----------------------------------------"
     "---------------------------------------\n";


void disable_logging()
{
  logbook_active = 0;
  cmd_display(M_COMMAND,0,log_error_text,1);
}

#ifdef DEBUGIO
extern int fddebugio ; 
extern char debugiof[] ; 

void init_debugio()
{
  if (( fddebugio = open ( debugiof, O_RDWR|O_CREAT , PMODE )) == -1 ) 
    {
      close ( fddebugio ) ; 
    } ;
  return ;
}

void exit_debugio()
{ 
      close ( fddebugio ) ;
} 
#endif 

void init_log()
{
  int fd;
  char tmpname[160];

  logbook_flag = 1;
  logbook_active = 1;  
  strcpy(tmpname,tnt_logbookfile);
  /* test if file already existing */
  if ((fd = open(tmpname,O_RDONLY)) != -1) {
    close(fd);
    return;
  }  
  /* file, not existing, create it and write header */
  if ((fd = open(tmpname,O_RDWR|O_CREAT|O_EXCL,PMODE)) == -1) {
    /* can't create file */
    disable_logging();
    return;
  }
  if (write(fd,loghead,strlen(loghead)) < (ssize_t)strlen(loghead)) {
    disable_logging();
    close(fd);
    return;
  }
  if (write(fd,logdelim,strlen(logdelim)) < (ssize_t)strlen(logdelim)) {
    disable_logging();
    close(fd);
    return;
  }
  close(fd);
}


#define LOG_LINELEN 120

void write_log(channel)
int channel;
{
  char logstr[LOG_LINELEN];
  char tmpstr[LOG_LINELEN];
  struct tm *cvtime;
  int fd;
  int avail;
  char tmpname[2*LOG_LINELEN];
  
  if (!logbook_flag) return;
  if (!logbook_active) return;
  strcpy(tmpname,tnt_logbookfile);
  if ((fd = open(tmpname,O_RDWR|O_APPEND)) == -1) {
    /* can't open file */
    disable_logging();
    return;
  }
  /* generate the logline */
  snprintf(logstr, sizeof(logstr), "%-10.10s%-7.7s ",
           ch_stat[channel].mycall, qrg_info[(int)0].qrg);
  cvtime = localtime(&ch_stat[channel].start_time);
  strftime(tmpstr,sizeof(tmpstr),"%d.%m.%y %R | ",cvtime);
  strncat(logstr, tmpstr, sizeof(logstr) - strlen(logstr) - 1);
  cvtime = localtime(&ch_stat[channel].end_time);
  strftime(tmpstr,sizeof(tmpstr),"%d.%m.%y %R | ",cvtime);
  strncat(logstr, tmpstr, sizeof(logstr) - strlen(logstr) - 1);
  if (ch_stat[channel].log_call[0] != '\0') {
    strncat(logstr, ch_stat[channel].log_call, sizeof(logstr) - strlen(logstr) - 1);
    strncat(logstr, ", Uplink: ", sizeof(logstr) - strlen(logstr) - 1);
  }
  avail = LOG_LINELEN - (int)strlen(logstr) - 2;
  if (avail < 0) avail = 0;
  snprintf(tmpstr, sizeof(tmpstr), "%.*s\n", avail, ch_stat[channel].call);
  strncat(logstr, tmpstr, sizeof(logstr) - strlen(logstr) - 1);
  /* logline available in logstr */
  if (write(fd,logstr,strlen(logstr)) < (ssize_t)strlen(logstr)) {
    disable_logging();
    close(fd);
    return;
  }
  close(fd);
}


void exit_log()
{
  int i;
  
  for (i = 1;i < tnc_channels;i++) {
    if (ch_stat[i].state >= 4) {
      ch_stat[i].end_time = time(NULL);
      write_log(i);
    }
  }
}
