/*
   macro.c
   (C) Joerg Trieschmann (DD8FR @ DB0EAM) 1994
   
   updated: Mark Wahl DL4YBG 96/05/26
   updated: Matthias Hensler WS1LS 99/09/08

   $Id: macro.c,v 1.3 2001/11/28 22:37:51 cvs-tnt Exp $
*/

/* -------------------------------------------------- */

#include "tnt.h"
#include "macro.h"
#include <time.h>

char news_file_name[MAXCHAR];
char name_file_name[MAXCHAR];

#define MAXDTCHAR	30		/* max size of datetime buffer */
#define COOKIE_TMP	"/tmp/tntoriXXXXXX"

#undef DEBUG

/* external function declarations */
extern int gen_cookie();

extern time_t sysopactiv;
extern time_t tnt_startup;
extern time_t session_sek;
extern char tnt_info_message[];
extern char rem_ver_str[];                         /* from main.c */

u_char macro_gettime(char *);
void macro_getdate(char *);
int  macro_getname(char *,char *);
int  find_line_for_call(char *,char *);

int getbyte(int fd)
{
  char cs[1];
  if(read(fd,cs,1) <= 0) return EOF;
  return (int)cs[0];
}

int putbyte(int c,int fd)
{
  char cs[1];
  cs[0] = (char)c;
  return write(fd,cs,1);
}

void putstring(char *str,int fd)
{
  int i;
  for(i = 0;str[i] != 0;i++) putbyte((int)(str[i]),fd);
}

/*
   replace_macros() reads file 'in' and replaces all '%'-macros by the text they
   stand for. The new file is written to 'out'.
*/

int replace_macros(int in,int out,char *othercall,char *mycall,int channel,
                   time_t start_time)
{
  int c,c1;
  char number[4];
  char *tmp_name = strdup(COOKIE_TMP);
  int tmp_cookie;
  int news_file;
  char datetimestr[MAXDTCHAR];
  char namestr[28];
  char line[128];
  char tmpstr[160];
  time_t diffmin;
  int hour;
  int day;

  while((c = getbyte(in)) != EOF) {

#ifdef DEBUG
    fprintf(stderr,"%c",c);
#endif

    if(c == '%') {
      if((c1 = getbyte(in)) != EOF) {

#ifdef DEBUG
	fprintf(stderr,"%c",c1);
#endif

	switch(c1) {
	case 'V':
	case 'v':
	  putstring(rem_ver_str,out);
	  break;
	case '%':   /* '%' itself */
	  putbyte(c,out);
	  break;
	case 'A':	/* Sysopactivity */
	case 'a':
		diffmin = (time(NULL) - sysopactiv ) / 60;
		hour = (diffmin / 60);
                diffmin %= 60;
		if(hour>0) sprintf(namestr,"%dh %ldm",hour,diffmin);
        else {
		  sprintf(namestr,"%ld Minutes",diffmin);
          if(diffmin!=1) strcat(namestr,"n"); }
		putstring(namestr,out);
		break;
        case 'l': /* Connectlaenge */
        case 'L':
          diffmin = ((time(NULL) - start_time ) / 60);
          hour = (diffmin / 60);
          diffmin %= 60;
          sprintf(namestr,"%2.2u:%2.2lu",hour,diffmin);
          putstring(namestr,out);
          break;
	case 'S':	/* Laufzeit dieser Version */
	case 's':
		diffmin = ((time(NULL) - tnt_startup ) + session_sek) / 60;
                day = (diffmin / 1440);
                hour = (diffmin / 60) % 60;
                diffmin %= 60;
		if(day>0) sprintf(namestr,"%dd %dh %ldm",day,hour,diffmin);
		else sprintf(namestr,"%dh %ldm",hour,diffmin);
		putstring(namestr,out);
		break;
	case 'M':	/* Message ausgeben */
	case 'm':
		putstring(tnt_info_message,out);
		break;
	case 'C':   /* call of other station */
	case 'c':
	  putstring(othercall,out);
	  break;
	case 'N':   /* name of other station */
	case 'n':
	  if(macro_getname(othercall,namestr) == TNT_OK)
	    putstring(namestr,out);
	  break;
	case 'Y':   /* call of this station */
	case 'y':
	  putstring(mycall,out);
	  break;
	case 'K':   /* channel number */
	case 'k':
	  sprintf(number,"%d",channel);
	  putstring(number,out);
	  break;
	case 'T':   /* time */
	case 't':
	  macro_gettime(datetimestr);
	  putstring(datetimestr,out);
	  break;
	case 'D':   /* date */
	case 'd':
	  macro_getdate(datetimestr);
	  putstring(datetimestr,out);
	  break;
	case 'B':   /* bell */
	case 'b':
	  putbyte(0x07,out);
	  break;
	case 'I':
	case 'i':
	  strcpy(tmpstr,news_file_name);
	  if((news_file = open(tmpstr,O_RDONLY))) {
	    while((c1 = getbyte(news_file)) != EOF) putbyte(c1,out);
	    close(news_file);
	  }
	  break;
	case 'Z':   /* timezone */
	case 'z':
	  /* a call to localtime() initializes tzname[] */
	    putstring(tzname[(int) macro_gettime(datetimestr)],out);
	  break;
	case '_':   /* CR/LF */
	  putstring("\r\n",out);
	  break;
	case 'O':   /* print origin (cookie) */
	case 'o':
	  close(mkstemp(tmp_name));
	  if(!gen_cookie(tmp_name,0)) {
	    if((tmp_cookie = open(tmp_name,O_RDONLY))) {
	      while((c1 = getbyte(tmp_cookie)) != EOF) putbyte(c1,out);
	      close(tmp_cookie);
	      unlink(tmp_name);
	    }
	  } else putstring(_("sorry, no cookie"),out);
	  break;
	case '?':   /* user is not in name-file ? -> tell it */
	  if(find_line_for_call(othercall,line) == LINE_NOTFOUND)
	    putstring(_("Pse enter your name with //name <your_name>"),out);
	  break;
	default:
	  putbyte(c,out);
	  putbyte(c1,out);
	}
      } else { putbyte(c,out); free(tmp_name); return TNT_OK; }
    } else putbyte(c,out);
  }

  free(tmp_name);
  return TNT_OK;
}

/*
  macro_gettime() passes a pointer to the string containing the time in a
  human readable form. It will return a value of 0x01 for Daylight Saving
  Time or 0x00 otherwise.

  The link /usr/lib/zoneinfo/localtime, the location may differ between
  operating systems, has to point to the correct file representing your 
  timezone. On GNU systems, the timezone can also be specified by means of 
  the TZ environment variable. 
*/

u_char macro_gettime(char *datetimestr)
{
  struct tm *timestr;
  time_t timeval;

  time(&timeval);
  timestr = localtime(&timeval);

  strftime(datetimestr,MAXDTCHAR,"%T", timestr); 

  if ( timestr->tm_isdst != 0 )
    return 0x1;

  return 0x0;
}

void macro_getdate(char *datetimestr)
{
  struct tm *timestr;
  time_t timeval;

  time(&timeval);
  timestr = localtime(&timeval);

  strftime(datetimestr,MAXDTCHAR,"%y/%m/%d",timestr); 
}

int macro_getname(char *call,char *name)
{
  int i;
  char line[128];
  char *cptr,*cptr1;

#ifdef DEBUG
  fprintf(stderr,"in macro_getname()\n");
#endif

  if(find_line_for_call(call,line) == LINE_FOUND) {
    cptr = line;
    while((*cptr != ' ') && (*cptr != '\t') && (*cptr != 0)) cptr++;
    while((*cptr == ' ') || (*cptr == '\t')) cptr++;
    i = 0;
    cptr1 = cptr;
    while((*cptr1 != ';') && (*cptr1 != 0) && (i < 27)) {
      cptr1++; i++;
    }
    *cptr1=0;
    if(i) {
      strncpy(name,cptr,i+1);
      name[i+1] = '\0';
    }
    else  strcpy(name,call);
    return TNT_OK;
  }

  strcpy(name,call);
  return TNT_OK;
}

int find_line_for_call(char *call,char *line)
{
  char callline[MAXCHAR];
  char remline[MAXCHAR] = "";
  int c = 0;
  int i;
  char mcall[10];
  long namelen;
  int fd;
  char namesfile[160];

  /* this can't be a callsign */
  if(strlen(call) > 9) return LINE_NOTFOUND;
  
  strcpy(namesfile,name_file_name);

  fd = open(namesfile,O_RDONLY);
  if(fd<0) return LINE_NOTFOUND;

  strcpy(mcall,call);

  i = 0;
  while(mcall[i] != 0) { mcall[i] = toupper(mcall[i]); i++; }

#ifdef DEBUG
  fprintf(stderr,"find_line_for_call(): searching for call %s\n",call);
#endif

  while(c != EOF) {

    /* get a line from file */
    c = getbyte(fd);
    i = 0;
    while((i < MAXCHAR) && (c != '\n') && (c != EOF)) {
      callline[i++] = c;
      c = getbyte(fd);
    }
    callline[i] = 0;

#ifdef DEBUG
    fprintf(stderr,"find_line_for_call(): got line : %s\n",callline);
#endif

    if(i > 2) {
      if(callline[1] == '>') {
#ifdef HAVE_INDEX      
	if(!index(mcall,'-')) {
#else
	if(!strchr(mcall,'-')) {
#endif	
	                       /* call has no ssid */
	  if(!strncmp(&(callline[2]),mcall,strlen(mcall))) {
	    if(callline[2+strlen(mcall)] != '-') {
	      strcpy(line,callline);               /* found exact match */
	      close(fd);
	      return LINE_FOUND;
	    } else
	      /* if we'll do not find a better matching call, we'll take this */
	      if(!strlen(remline)) strcpy(remline,callline);
	  }
	} else {
	                        /* call has ssid */
	  if(!strncmp(&(callline[2]),mcall,strlen(call))) {
	    strcpy(line,callline);                 /* found exact match */
	    close(fd);
	    return LINE_FOUND;
	  } else {
	    namelen = 0;
	    while(mcall[namelen] != '-') namelen++;
	    if(!strncmp(&(callline[2]),mcall,namelen)) strcpy(remline,callline);
	  }
	}
      }
    }
  }

  if(strlen(remline)) {
    strcpy(line,remline);
    close(fd);
    return LINE_FOUND;
  }

  close(fd);
  return LINE_NOTFOUND;
}

int delete_line_for_call(char *call)
{
  char callline[81];
  int c = 0;
  int i;
  char mcall[10];
  long namelen;
  int fd;
  int fd2;
  char namesfile[160];
  char newnamesfile[160];
  int found;

  /* this can't be a callsign */
  if(strlen(call) > 9) return(1);
 
  strcpy(namesfile,name_file_name);

  strcpy(newnamesfile,namesfile);
  strcat(newnamesfile,"~");
  
  fd = open(namesfile,O_RDONLY);
  if(fd<0) return(1);
  
  fd2 = open(newnamesfile,O_RDWR|O_CREAT|O_TRUNC,PMODE);
  if (fd2 < 0) {
    close(fd);
    return(1);
  }

  strcpy(mcall,call);

  i = 0;
  while(mcall[i] != 0) { mcall[i] = toupper(mcall[i]); i++; }

#ifdef DEBUG
  fprintf(stderr,"delete_line_for_call(): searching for call %s\n",call);
#endif

  while(c != EOF) {

    /* get a line from file */
    c = getbyte(fd);
    i = 0;
    while((i < MAXCHAR) && (c != '\n') && (c != EOF)) {
      callline[i++] = c;
      c = getbyte(fd);
    }
    callline[i] = 0;

    found = 0;
    
    if(i > 2) {
      if(callline[1] == '>') {
#ifdef HAVE_INDEX      
	if(!index(mcall,'-')) {
#else
	if(!strchr(mcall,'-')) {
#endif	
	                       /* call has no ssid */
	  if(!strncmp(&(callline[2]),mcall,strlen(mcall))) {
	    found = 1;                /* found match */
	  }
	} else {
	                        /* call has ssid */
	  if(!strncmp(&(callline[2]),mcall,strlen(call))) {
	    found = 1;                 /* found exact match */
	  } else {
	    namelen = 0;
	    while(mcall[namelen] != '-') namelen++;
	    if(!strncmp(&(callline[2]),mcall,namelen)) found = 1;
	  }
	}
      }
    }
    if ((!found) && (c != EOF)) {
      strcat(callline,"\n");
      if (write(fd2,callline,strlen(callline)) < strlen(callline)) {
        close(fd);
        close(fd2);
        unlink(newnamesfile);
        return(1);
      }
    }
  }
  close(fd);
  close(fd2);
  unlink(namesfile);
  rename(newnamesfile,namesfile);
  return(0);
}

int add_line_for_call(char *call,char *name)
{
  char namesfile[160];
  int fd;
  char tempstr[250];
  
  /* this can't be a callsign */
  if(strlen(call) > 9) return(1);
 
  strcpy(namesfile,name_file_name);

  fd = open(namesfile,O_RDWR|O_APPEND|O_CREAT,PMODE);
  if (fd < 0) return(1);

  sprintf(tempstr,"T>%s %.50s\n",call,name);
  if (write(fd,tempstr,strlen(tempstr)) < strlen(tempstr)) {
    close(fd);
    return(1);
  }
  
  close(fd);
  return(0);
}
