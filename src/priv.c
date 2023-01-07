/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993-1997 by Mark Wahl
   For license details see documentation
   Procedures for remote passwords (priv.c)

   created: Mark Wahl DL4YBG 94/12/21
   updated: Mark Wahl DL4YBG 97/02/01
   updated: Mark Wahl DL4YBG 97/08/01
   fixed: Matthias Hensler WS1LS 98/04/12
   updated: Johann Hanne DH3MB 98/12/28
   DXClusterergaenzungen Henning Folger, DL6DH  06.02.1999
   MD2 gefixed und MD5 eingebaut: Axel Schneider, DG2GCC  03.01.2000
*/ 

#include "tnt.h"
#include "priv.h"


extern int tnc_channels;
extern char tnt_conf_dir[];
extern char rem_newlin_str[];
extern struct channel_stat *ch_stat;

/* external function declarations */
extern void cmd_display();
extern int queue_cmd_data();
extern void rem_data_display();
extern void strip_call_log();
extern int x_random();

char tnt_pwfile[MAXCHAR];

static struct password_stat *pw_stat;
static struct calllist *calllist_root;

static struct pwmodes pwmodes[] = {
  {"DIEBOX", PW_DIEBOX},
  {"FLEXNET", PW_FLEXNET},
  {"THENET", PW_THENET},
  {"BAYCOM", PW_BAYCOM},
  {"MD2", PW_MD2},
  {"CLUSTER",PW_CLUSTER},
  {"MD5", PW_MD5},
  {"",PW_NONE}
};

static void MD2Init();
static void MD2Update();
static void MD2Final();

/* analyse one line of password file */
static int pwline_analyse(str1,str2,str3,str4,str5,num_par,calllist_ptr)
char *str1;
char *str2;
char *str3;
char *str4;
char *str5;
int num_par;
struct calllist **calllist_ptr;
{
  struct calllist *calllist_wrk;
  int found;
  int i;
  int pwmode;
  int val1;
  char tmpstr[83];
  
  if (strlen(str1) > 9) return(1); /* callsign maximum 9 characters */
  i = 0;
  found = 0;
  while ((!found) && (pwmodes[i].pwtype[0] != '\0')) {
    if (strcasecmp(str2,pwmodes[i].pwtype) == 0) {
      found = 1;
      pwmode = pwmodes[i].pwmode;
      /* check number of arguments */
      switch (pwmode) {
      case PW_DIEBOX:
        if (num_par != 3) return(1); 
        break;
      case PW_FLEXNET:
        if (num_par == 3) {
          strcpy(tmpstr,"SYS");
        }
        else if (num_par == 4) {
          strcpy(tmpstr,str4);
        }
        else {
          return(1);
        }
        break;
      case PW_THENET:
        if (num_par != 5) return(1);
        if (sscanf(str4,"%d",&val1) != 1) return(1);
        if ((val1 < 0) || (val1 > PWTN_MAXVAL)) return(1);
        strcpy(tmpstr,str5); 
        break;
      case PW_BAYCOM:
        if ((num_par != 5) && (num_par != 4)) return(1);
        if (num_par == 4)
          strcpy(tmpstr,"sys");
        else
          strcpy(tmpstr,str5);
        if (sscanf(str4,"%d",&val1) != 1) return(1);
        if ((val1 != 0) && (val1 != 2) && (val1 != 6)) return(1);
        break;
      case PW_MD2:
      case PW_MD5:
         if (num_par == 3) {
         strcpy(tmpstr,str2);  /* DG2GCC 03/01/2000 */
         }
         else if (num_par == 4) {
           strcpy(tmpstr,str4);
         }
         else {
           return(1);
         }
        break;
      case PW_CLUSTER:
        if(num_par != 4) return(1);
        strcpy(tmpstr,str4);
        break;
        
      }
    }
    i++;
  }
  if (!found) return(1);
  calllist_wrk = (struct calllist *)malloc(sizeof(struct calllist));
  for (i=0;i<strlen(str1);i++) {
    calllist_wrk->callsign[i] = toupper(str1[i]);
  }
  calllist_wrk->callsign[i] = '\0';
  calllist_wrk->pwmode = pwmode;
  strcpy(calllist_wrk->pw_file,str3);
  if ((pwmode == PW_THENET) || (pwmode == PW_BAYCOM)) {
    calllist_wrk->flags = val1;
    strcpy(calllist_wrk->priv_string,tmpstr);
  }
  else if ((pwmode == PW_FLEXNET) || (pwmode == PW_MD2) || (pwmode == PW_MD5)) {
    strcpy(calllist_wrk->priv_string,tmpstr);
  }
  
  if (pwmode == PW_CLUSTER)
  {
    strcpy(calllist_wrk->priv_string,tmpstr);
  }
  
  calllist_wrk->next = NULL;
  if (*calllist_ptr == NULL) {
    calllist_root = calllist_wrk;
    *calllist_ptr = calllist_wrk;
  }
  else {
    (*calllist_ptr)->next = calllist_wrk;
    *calllist_ptr = calllist_wrk;
  }
  return(0);
} 

/* load password file */
static void load_pwfile()
{
  int file_end;
  int file_corrupt;
  char file_str[160];
  char line[83];
  char str1[83];
  char str2[83];
  char str3[83];
  char str4[83];
  char str5[84];
  int rslt;
  FILE *pw_file_fp;
  struct calllist *calllist_cur;

  strcpy(file_str,tnt_pwfile);
  if (!(pw_file_fp = fopen(file_str,"r"))) {
    /* no file present, exit */
    return;
  }
  file_end = 0;
  file_corrupt = 0;
  calllist_cur = NULL;
  while(!file_end) {
    if (fgets(line,82,pw_file_fp) == NULL) {
      file_end = 1;
    }
    else {
      if (strlen(line) == 82) {
        file_corrupt = 1;
        file_end = 1;
      }
      else {
        if (line[0] != '#') {
          rslt = sscanf(line,"%s %s %s %s %s",str1,str2,str3,str4,str5);
          switch (rslt) {
          case EOF:
            break;
          case 3:
          case 4:
          case 5:
          case 7:
            if (pwline_analyse(str1,str2,str3,str4,str5,rslt,&calllist_cur)) {
              file_corrupt = 1;
              file_end = 1;
            }
            break;
          default:
            file_corrupt = 1;
            file_end = 1;
            break;
          }
        }
      }
    }
  }
  fclose(pw_file_fp);
  if (file_corrupt) {
    if (line == NULL) line[0] = '\0';
    cmd_display(M_COMMAND,0,
      _("WARNING: pwfile is in wrong format, wrong line:"),1);
    cmd_display(M_COMMAND,0,line,1);
    return;
  }
}

static void clear_pwfile()
{
  struct calllist *calllist_wrk;
  struct calllist *calllist_tmp;
  
  calllist_wrk = calllist_root;
  while (calllist_wrk != NULL) {
    calllist_tmp = calllist_wrk;
    calllist_wrk = calllist_tmp->next;
    free(calllist_tmp);
  }
  calllist_root = NULL;
}

/* the callsign has changed on channel, check if callsign in calllist
   and if found set pwmode */
void set_pwmode(channel)
int channel;
{
  char callsign[10];
  struct calllist *calllist_wrk;
  int found;
  
  strip_call_log(callsign,channel);
  pw_stat[channel].pwmode = PW_NONE;
  calllist_wrk = calllist_root;
  found = 0;
  while ((!found) && (calllist_wrk != NULL)) {
    if (strcmp(callsign,calllist_wrk->callsign) == 0) {
      found = 1;
      pw_stat[channel].pwmode = calllist_wrk->pwmode;
      pw_stat[channel].entry = calllist_wrk;
      pw_stat[channel].box_login[0] = '\0';
      if (pw_stat[channel].pwmode == PW_BAYCOM) { /* bei Baycom sofort nach */
        pw_stat[channel].pass_wait = 1;   /* der Frage suchen */
        pw_stat[channel].tries = 30;       /* aber max. 30 Zeilen lang */
      }
      else if (pw_stat[channel].pwmode == PW_MD2) {
        pw_stat[channel].pass_wait = 1;
        pw_stat[channel].tries = 30;
      }
      else if (pw_stat[channel].pwmode == PW_MD5) {
        pw_stat[channel].pass_wait = 1;
        pw_stat[channel].tries = 30;
      }
      else {
        pw_stat[channel].pass_wait = 0;
      }
    }
    else {
      calllist_wrk = calllist_wrk->next;
    }
  }
}

/* clear password information on disconnect */
void clear_pwmode(channel)
int channel;
{
  pw_stat[channel].pwmode = PW_NONE;
  pw_stat[channel].entry = NULL;
  pw_stat[channel].box_login[0] = '\0';
  pw_stat[channel].pass_wait = 0;
}

static char gen_rand_char()
{
  int val;
  
  val = x_random(0x7B);
  if (val <= 0x20) val += 0x40;
  if (val == ';') val++;
  if (val == ',') val++;
  return((char)val);
}

/* generate random strings */
static void gen_rand_string(str,len,pwstr)
char *str;
int len;
char *pwstr;
{
  char save_str[256];
  int i;
  int pos;
  int save_len;
  int len2;
  int pwlen;
  
  save_len = strlen(str);
  if (save_len > 0) {
    strcpy(save_str,str);
  }
  if (pwstr == NULL) {				/* DL8NEG */
    for (i=0;i<len;i++) {
      str[i] = gen_rand_char();
    }
  } else {
    pwlen = strlen(pwstr);
    for (i=0;i<len;i++) {
      str[i] = pwstr[x_random(pwlen)];
    }
  }
  str[len] = '\0';
  if (save_len > 0) {
    len2 = len - save_len;
    if (len2 > 0) {
      pos = x_random(len2);
      for (i=0;i<save_len;i++) {
        str[pos+i] = save_str[i];
      }
    }
  }
}

/* try to find password request */
void scan_pw_request(channel,buffer,len)
int channel;
char *buffer;
int len;
{
  char buffer_tmp[257];
  int flex_value;
  int tmp_value;
  int flex_result;
  int res,flag;
  char tmpstr[256];
  char *ptr;
  
  if (ch_stat[channel].conn_state != CS_CONN) return;
  if (pw_stat[channel].pwmode == PW_NONE) return;
  
  switch (pw_stat[channel].pwmode) {
      case PW_FLEXNET:
    if (!pw_stat[channel].pass_wait) return;
    strncpy(buffer_tmp,buffer,len);
    buffer_tmp[len] = '\0';
    ptr = strchr(buffer_tmp,'(');
    if (ptr == NULL) return;
    res = sscanf(ptr,"(%d) %d>",&tmp_value,&flex_value);
    if (res != 2) return;
    if (flex_value > 99999) return;
    
    flex_result = (flex_value / 10000) *
                  (pw_stat[channel].entry->pw_file[0] - '0');
    flex_value = flex_value % 10000;
    flex_result += (flex_value / 1000) *
                   (pw_stat[channel].entry->pw_file[1] - '0');
    flex_value = flex_value % 1000;
    flex_result += (flex_value / 100) *
                   (pw_stat[channel].entry->pw_file[2] - '0');
    flex_value = flex_value % 100;
    flex_result += (flex_value / 10) *
                   (pw_stat[channel].entry->pw_file[3] - '0');
    flex_value = flex_value % 10;
    flex_result += flex_value *
                   (pw_stat[channel].entry->pw_file[4] - '0');
    sprintf(tmpstr,"%d%s",flex_result,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    flag = 0;
    queue_cmd_data(channel,X_DATA,len,flag,tmpstr);
    pw_stat[channel].pass_wait = 0;
    break;
  }
}

/* try to find login time for DieBox */
/* auch andere Passwoerter sind eingegangen, DL6DH*/

void scan_login_received(channel,buffer,len)
int channel;
char *buffer;
int len;
{
  char *box_logpos;
  char buffer_tmp[257];
  char tmpstr[256];
  char tmpstr2[40];
  int res,flag;
  int tn_val[5];
  char ch;
  int i,n;
  FILE *fd;
  int error;
  char passfile[MAXCHAR];
  char *ptr;
  MD2_CTX context2;
  unsigned char digest[16];
  char buff[256];
  char rubbish[256];
  char *hlpptr;
  MD5_CTX context5;
    
  if (ch_stat[channel].conn_state != CS_CONN) return;
  if (pw_stat[channel].pwmode == PW_NONE) return;
  
  switch (pw_stat[channel].pwmode) {
  case PW_DIEBOX: 
    if (pw_stat[channel].box_login[0] != '\0') return;
    strncpy(buffer_tmp,buffer,len);
    buffer_tmp[len] = '\0';
    box_logpos = strstr(buffer_tmp,"Login: ");
    if (box_logpos != NULL) {
      i = (*(box_logpos + 15) == ' ') ? 14 : 16;
      strncpy(pw_stat[channel].box_login,box_logpos+7,i);
      pw_stat[channel].box_login[i] = '\0';
    }
    /* additional for use at DB0BOX, language "expert", no "Login:" prompt  */
    /* (DB0BOX says "> > " instead of "DIEBOX.... Login:"). 95/01/12 DL4NER */
    else {
      box_logpos = strstr(buffer_tmp,"> > ");
      if (box_logpos != NULL) {
        strncpy(pw_stat[channel].box_login,box_logpos+4,14);
        pw_stat[channel].box_login[14] = '\0';
      }
    }
    /* end additional code, DL4NER */
    break;
  case PW_THENET:
    if (!pw_stat[channel].pass_wait) return;
    strncpy(buffer_tmp,buffer,len);
    buffer_tmp[len] = '\0';
    ptr = strrchr(buffer_tmp,'>');
    if (ptr == NULL)			/* added for X1J4 nodes    */
      ptr = strrchr(buffer_tmp,'}');    /* using curley brackets   */
    if (ptr == NULL)
      ptr = buffer_tmp;
    else
      ptr++;
    res = sscanf(ptr,"%d %d %d %d %d",
                 &tn_val[0],&tn_val[1],&tn_val[2],&tn_val[3],&tn_val[4]);
    if (res != 5) return;

    if (pw_stat[channel].entry->pw_file[0] == '/') {
      strcpy(passfile,pw_stat[channel].entry->pw_file);
    }
    else {
      strcpy(passfile,tnt_conf_dir);
      strcat(passfile,pw_stat[channel].entry->pw_file);
    }
    error = 1;
    if((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
      hlpptr = fgets(buffer_tmp,256,fd);
      if (hlpptr != NULL) {
        i = strlen(buffer_tmp);
        if (i > 10) {
          if (buffer_tmp[--i] == '\n') buffer_tmp[i] = '\0';
          error = 0;
          hlpptr = fgets(rubbish,256,fd);
          if (hlpptr != NULL) {
            i = strlen(rubbish);
            if (i > 10) {
              if (rubbish[--i] == '\n') rubbish[i] = '\0';
            }
            else rubbish[0] = '\0';
          }
          else rubbish[0] = '\0';
        }
        fclose(fd);
        if (!error) {
          tmpstr[0] = '\0';
          i = 0;
          while(i < 5) {
            if (tn_val[i] > strlen(buffer_tmp))
              error = 1;
            else
              ch = buffer_tmp[tn_val[i]-1];
            strncat(tmpstr,&ch,1);
            i++;
          }
        }
      }
    }
    if (error) {
      pw_stat[channel].pass_wait = 0;
      return;
    }
    if (pw_stat[channel].entry->flags & PWTN_HIDESTRING) {
      /* hide answer in random string */
      if (pw_stat[channel].entry->flags & PWTN_HIDEPERFECT) {
        if (rubbish[0] == '\0')
          gen_rand_string(tmpstr,PWTN_STRLEN,buffer_tmp);
        else
          gen_rand_string(tmpstr,PWTN_STRLEN,rubbish);
      }
      else
        gen_rand_string(tmpstr,PWTN_STRLEN,NULL);
    }
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    flag = 0;
    queue_cmd_data(channel,X_DATA,len,flag,tmpstr);
    pw_stat[channel].tries--;
    if (pw_stat[channel].tries == 0) {
      pw_stat[channel].pass_wait = 0;
    }
    else {
      strcpy(tmpstr,pw_stat[channel].entry->priv_string);
      strcat(tmpstr,rem_newlin_str);
      len = strlen(tmpstr);
      rem_data_display(channel,tmpstr);
      flag = 0;
      queue_cmd_data(channel,X_DATA,len,flag,tmpstr);
    }
    break;
    
  case PW_CLUSTER:		/* fuer DXCluster */
  if (!pw_stat[channel].pass_wait) return;
    strncpy(buffer_tmp,buffer,len);
    buffer_tmp[len] = '\0';
    ptr = strrchr(buffer_tmp,'>');
    if (ptr == NULL)
      ptr = buffer_tmp;
    else
      ptr++;
    res = sscanf(ptr,"%d %d %d %d",
                 &tn_val[0],&tn_val[1],&tn_val[2],&tn_val[3]);
    if (res != 4) return;

    if (pw_stat[channel].entry->pw_file[0] == '/') {
      strcpy(passfile,pw_stat[channel].entry->pw_file);
    }
    else {
      strcpy(passfile,tnt_conf_dir);
      strcat(passfile,pw_stat[channel].entry->pw_file);
    }
    error = 1;
    if((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
      hlpptr = fgets(buffer_tmp,256,fd);
      if (hlpptr != NULL) {
        i = strlen(buffer_tmp);
        if (i > 10) {
          if (buffer_tmp[--i] == '\n') buffer_tmp[i] = '\0';
          error = 0;
          hlpptr = fgets(rubbish,256,fd);
          if (hlpptr != NULL) {
            i = strlen(rubbish);
            if (i > 10) {
              if (rubbish[--i] == '\n') rubbish[i] = '\0';
            }
            else rubbish[0] = '\0';
          }
          else rubbish[0] = '\0';
        }
        fclose(fd);
        if (!error) {
          tmpstr[0] = '\0';
          i = 0;
          while(i < 4) {
            if (tn_val[i] > strlen(buffer_tmp))
              error = 1;
            else
              ch = buffer_tmp[tn_val[i]-1];
            strncat(tmpstr,&ch,1);
            i++;
          }
        }
      }
    }
    if (error) {
      pw_stat[channel].pass_wait = 0;
      strcpy(tmpstr,"error");		/* irgendeine Ausgabe erzwingen */
      strcat(tmpstr,rem_newlin_str);
      len = strlen(tmpstr);
      queue_cmd_data(channel,X_DATA,len,0,tmpstr);
      return;
    } 
    
    if (pw_stat[channel].entry->flags & PWTN_HIDESTRING) {
      /* hide answer in random string */
      if (pw_stat[channel].entry->flags & PWTN_HIDEPERFECT) {
        if (rubbish[0] == '\0')
          gen_rand_string(tmpstr,PWTN_STRLEN,buffer_tmp);
        else
          gen_rand_string(tmpstr,PWTN_STRLEN,rubbish);
      }
      else
        gen_rand_string(tmpstr,PWTN_STRLEN,NULL);
    }
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    flag = 0;
    queue_cmd_data(channel,X_DATA,len,flag,tmpstr);
    pw_stat[channel].tries--;
    if (pw_stat[channel].tries == 0) {
      pw_stat[channel].pass_wait = 0;
    }
    else {
      strcpy(tmpstr,pw_stat[channel].entry->priv_string);
      strcat(tmpstr,rem_newlin_str);
      len = strlen(tmpstr);
      rem_data_display(channel,tmpstr);
      flag = 0;
      queue_cmd_data(channel,X_DATA,len,flag,tmpstr);
    }
    break;
    
  case PW_BAYCOM:
    if (!pw_stat[channel].pass_wait) return;
    pw_stat[channel].tries --;
    if (pw_stat[channel].tries == 0) pw_stat[channel].pass_wait = 0;
    strncpy(buffer_tmp,buffer,len);
    buffer_tmp[len] = '\0';
    ptr = strrchr(buffer_tmp,'>');
    if (ptr == NULL)
      return; /* DH3MB */
    else
      ptr++;
    res = sscanf(ptr,"%d %d %d %d %d",
                 &tn_val[0],&tn_val[1],&tn_val[2],&tn_val[3],&tn_val[4]);
    if (res != 5) return;

    if (pw_stat[channel].entry->pw_file[0] == '/') {
      strcpy(passfile,pw_stat[channel].entry->pw_file);
    }
    else {
      strcpy(passfile,tnt_conf_dir);
      strcat(passfile,pw_stat[channel].entry->pw_file);
    }
    error = 1;
    if((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
      hlpptr = fgets(buffer_tmp,256,fd);
      if (hlpptr != NULL) {
        i = strlen(buffer_tmp);
        if (i > 10) {
          if (buffer_tmp[--i] == '\n') buffer_tmp[i] = '\0';
          error = 0;
          hlpptr = fgets(rubbish,256,fd);
          if (hlpptr != NULL) {
            i = strlen(rubbish);
            if (i > 10) {
              if (rubbish[--i] == '\n') rubbish[i] = '\0';
            }
            else rubbish[0] = '\0';
          }
          else rubbish[0] = '\0';
        }
        fclose(fd);
        if (!error) {
          tmpstr[0] = '\0';
          i = 0;
          while(i < 5) {
            if (tn_val[i] > strlen(buffer_tmp))
              error = 1;
            else
              ch = buffer_tmp[tn_val[i]-1];
            strncat(tmpstr,&ch,1);
            i++;
          }
        }
      }
    }
    if (error) {
      pw_stat[channel].pass_wait = 0;
      return;
    }
    if (pw_stat[channel].entry->flags & PWTN_HIDESTRING) {
      /* hide answer in random string */
      if (pw_stat[channel].entry->flags & PWTN_HIDEPERFECT) {
        if (rubbish[0] == '\0')
          gen_rand_string(tmpstr,PWTN_STRLEN,buffer_tmp);
        else
          gen_rand_string(tmpstr,PWTN_STRLEN,rubbish);
      }
      else
        gen_rand_string(tmpstr,PWTN_STRLEN,NULL);
    }
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    flag = 0;
    queue_cmd_data(channel,X_DATA,len,flag,tmpstr);
    pw_stat[channel].pass_wait = 0;
    break;
  case PW_MD2:
    if (!pw_stat[channel].pass_wait) return;
    pw_stat[channel].tries --;
    if (pw_stat[channel].tries == 0) pw_stat[channel].pass_wait = 0;
    strncpy(buffer_tmp,buffer,len);
    buffer_tmp[len] = '\0';
    /* XXXBBS> [ABCDEFGHIL] */
    ptr = strchr(buffer_tmp,'[');
    if (ptr == NULL) return;
    if ((strlen(ptr) < 12) || (*(ptr+11) != ']')) return;
    strncpy(buff,ptr+1,10);
    buff[10] = '\0';
    
    if (pw_stat[channel].entry->pw_file[0] == '/') {
      strcpy(passfile,pw_stat[channel].entry->pw_file);
    }
    else {
      strcpy(passfile,tnt_conf_dir);
      strcat(passfile,pw_stat[channel].entry->pw_file);
    }
    if((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
      tmpstr[0] = '\0';
      fgets(tmpstr,160,fd);
      error = 0;
      if ((ptr = strchr(tmpstr,'\n')) != NULL) *ptr = '\0';
      if ((ptr = strchr(tmpstr,'\r')) != NULL) *ptr = '\0';
      fclose(fd);
    }
    else
      error = 1;
    if (error) {
      pw_stat[channel].pass_wait = 0;
      return;
    }
    strcat(buff,tmpstr);

    /* The MD2 engine computes the answer string */
    MD2Init (&context2);
    len = strlen(buff);
    for (i=0; i<len; i += 16) {
      n = (len - i) > 16 ? 16 : (len - i);
      MD2Update(&context2, &(buff[i]), n);
    }
    MD2Final (digest, &context2);
    
    tmpstr[0] = '\0';
    for (i=0; i<16; i++) {
      sprintf(tmpstr2,"%02x",digest[i]);
      strcat(tmpstr,tmpstr2);
    }
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    flag = 0;
    queue_cmd_data(channel,X_DATA,len,flag,tmpstr);
    pw_stat[channel].pass_wait = 0;
    break;
  case PW_MD5:
    if (!pw_stat[channel].pass_wait) return;
    pw_stat[channel].tries --;
    if (pw_stat[channel].tries == 0) pw_stat[channel].pass_wait = 0;
    strncpy(buffer_tmp,buffer,len);
    buffer_tmp[len] = '\0';
    /* XXXBBS> [ABCDEFGHIL] */
    ptr = strchr(buffer_tmp,'[');
    if (ptr == NULL) return;
    if ((strlen(ptr) < 12) || (*(ptr+11) != ']')) return;
    strncpy(buff,ptr+1,10);
    buff[10] = '\0';
    
    if (pw_stat[channel].entry->pw_file[0] == '/') {
      strcpy(passfile,pw_stat[channel].entry->pw_file);
    }
    else {
      strcpy(passfile,tnt_conf_dir);
      strcat(passfile,pw_stat[channel].entry->pw_file);
    }
    if((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
      tmpstr[0] = '\0';
      fgets(tmpstr,160,fd);
      error = 0;
      if ((ptr = strchr(tmpstr,'\n')) != NULL) *ptr = '\0';
      if ((ptr = strchr(tmpstr,'\r')) != NULL) *ptr = '\0';
      fclose(fd);
    }
    else
      error = 1;
    if (error) {
      pw_stat[channel].pass_wait = 0;
      return;
    }
    strcat(buff,tmpstr);
    
    /* The MD5 engine computes the answer string */
    MD5Init (&context5);
    len = strlen(buff);
    for (i=0; i<len; i += 16) {
      n = (len - i) > 16 ? 16 : (len - i);
      MD5Update(&context5, &(buff[i]), n);
    }

    MD5Final (digest, &context5);
    
    tmpstr[0] = '\0';
    for (i=0; i<16; i++) {
      sprintf(tmpstr2,"%02x",digest[i]);
      strcat(tmpstr,tmpstr2);
    }
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    flag = 0;
    queue_cmd_data(channel,X_DATA,len,flag,tmpstr);
    pw_stat[channel].pass_wait = 0;
    break;
  }
}

/* password generation for FlexNet */
static void priv_flexnet(channel,mode)
int channel;
int mode;
{
  int i;
  int error;
  int ch;
  char tmpstr[20];
  int len;
  
  i = 0;
  error = 0;
  /* test if password correct */
  while ((i < 5) && (!error)) {
    ch = (int)pw_stat[channel].entry->pw_file[i];
    if (!isdigit(ch)) error = 1;
    i++;
  }
  if (error) {
    cmd_display(mode,channel,_("Sorry, illegal character in password number"),1);
    return;
  }
  else {
    strcpy(tmpstr,pw_stat[channel].entry->priv_string);
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    queue_cmd_data(channel,X_DATA,len,mode,tmpstr);
    pw_stat[channel].pass_wait = 1;
    return;
  }
}

/* password generation for TheNet */
static void priv_thenet(channel,mode)
int channel;
int mode;
{
  char tmpstr[20];
  char passfile[MAXCHAR];
  FILE *fd;
  int len;
  
  if (pw_stat[channel].entry->pw_file[0] == '/') {
    strcpy(passfile,pw_stat[channel].entry->pw_file);
  }
  else {
    strcpy(passfile,tnt_conf_dir);
    strcat(passfile,pw_stat[channel].entry->pw_file);
  }
  if ((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
    fclose(fd);
    pw_stat[channel].tries = 1;
    pw_stat[channel].valid_try = 1;
    if (pw_stat[channel].entry->flags & PWTN_MORETRIES) {
      pw_stat[channel].tries = PWTN_TRIES;
      pw_stat[channel].valid_try = x_random(PWTN_TRIES) + 1;
    }
    strcpy(tmpstr,pw_stat[channel].entry->priv_string);
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    queue_cmd_data(channel,X_DATA,len,mode,tmpstr);
    pw_stat[channel].pass_wait = 1;
  }
  else {
    cmd_display(mode,channel, _("Sorry, passwordfile not found"),1);
  }
}

/* password generation for PacketCluster by Henning Folger, DL6DH */
/*----------------------------------------------------------------*/

static void priv_cluster(channel,mode)
int channel;
int mode;
{
  char tmpstr[20];
  char passfile[MAXCHAR];
  FILE *fd;
  int len;
  
  if (pw_stat[channel].entry->pw_file[0] == '/') {
    strcpy(passfile,pw_stat[channel].entry->pw_file);
  }
  else {
    strcpy(passfile,tnt_conf_dir);
    strcat(passfile,pw_stat[channel].entry->pw_file);
  }
  if ((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
    fclose(fd);
    pw_stat[channel].tries = 1;
    pw_stat[channel].valid_try = 1;
    if (pw_stat[channel].entry->flags & PWTN_MORETRIES) {
      pw_stat[channel].tries = PWTN_TRIES;
      pw_stat[channel].valid_try = x_random(PWTN_TRIES) + 1;

    }
    strcpy(tmpstr,pw_stat[channel].entry->priv_string);
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    queue_cmd_data(channel,X_DATA,len,mode,tmpstr);
    pw_stat[channel].pass_wait = 1;
  }
  else {
    cmd_display(mode,channel, _("Sorry, passwordfile not found"),1);
  }
  
}

/* password generation for Baycom by Oliver Fleischmann DL8NEG */
static void priv_baycom(channel,mode)
int channel;
int mode;
{
  char tmpstr[20];
  char passfile[MAXCHAR];
  FILE *fd;
  int len;
  
  if (pw_stat[channel].entry->pw_file[0] == '/') {
     strcpy(passfile,pw_stat[channel].entry->pw_file);
  }
  else {
    strcpy(passfile,tnt_conf_dir);
    strcat(passfile,pw_stat[channel].entry->pw_file);
  }
  if ((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
    fclose(fd);
    pw_stat[channel].pass_wait = 1;
    pw_stat[channel].tries = 30;
    strcpy(tmpstr,pw_stat[channel].entry->priv_string);
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    queue_cmd_data(channel,X_DATA,len,mode,tmpstr);
  }
  else {
    cmd_display(mode,channel, _("Sorry, passwordfile not found"),1);
  }
}
 
/* password generation for MD2 by Claudio Porfiri IW0FBB */
static void priv_md2(channel,mode)
int channel;
int mode;
{
  char tmpstr[20];
  char passfile[MAXCHAR];
  FILE *fd;
  int len;
  
  if (pw_stat[channel].entry->pw_file[0] == '/') {
     strcpy(passfile,pw_stat[channel].entry->pw_file);
  }
  else {
    strcpy(passfile,tnt_conf_dir);
    strcat(passfile,pw_stat[channel].entry->pw_file);
  }
  if ((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
    fclose(fd);
    pw_stat[channel].pass_wait = 1;
    pw_stat[channel].tries = 30;
/*  strcpy(tmpstr,"MD2"); /* Missing MD2, fixed on 10.04.98 by WS1LS */
    strcpy(tmpstr,pw_stat[channel].entry->priv_string); /* DG2GCC 03.01.2000 */
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    queue_cmd_data(channel,X_DATA,len,mode,tmpstr);
  }
  else {
    cmd_display(mode,channel, _("Sorry, passwordfile not found"),1);
  }
}

/* password generation for MD5 */
/* DG2GCC 03.01.2000 */
static void priv_md5(channel,mode)
int channel;
int mode;
{
  char tmpstr[20];
  char passfile[80];
  FILE *fd;
  int len;

  if (pw_stat[channel].entry->pw_file[0] == '/') {
     strcpy(passfile,pw_stat[channel].entry->pw_file);
  } else {
    strcpy(passfile,tnt_conf_dir);
    strcat(passfile,pw_stat[channel].entry->pw_file);
  }
  if ((fd = fopen(passfile,"r")) != NULL) { /* check if file present */
    fclose(fd);
    pw_stat[channel].pass_wait = 1;
    pw_stat[channel].tries = 30;
    strcpy(tmpstr,pw_stat[channel].entry->priv_string);
    strcat(tmpstr,rem_newlin_str);
    len = strlen(tmpstr);
    rem_data_display(channel,tmpstr);
    queue_cmd_data(channel,X_DATA,len,mode,tmpstr);
  } else {
    cmd_display(mode,channel,"Sorry, Passwortfile nicht gefunden",1);
  }
}

/* password generation for DieBox */
/**************************************************/
/* Command PRIV  (for THEBOX Remote Sysops) DK3NY */
/**************************************************/
static void priv_diebox(channel,mode)
int channel;
int mode;
{
  char priv[5];
  char privstr[17];
  char passfile[MAXCHAR];
  FILE *fd;
  int multiply, i, ofs;
  struct stat fileinfo;

  if(pw_stat[channel].box_login[0] != '\0') /* if logintime recorded */
  {
    if (pw_stat[channel].entry->pw_file[0] == '/') {
      strcpy(passfile,pw_stat[channel].entry->pw_file);
    }
    else {
      strcpy(passfile,tnt_conf_dir);
      strcat(passfile,pw_stat[channel].entry->pw_file);
    }
    if (stat(passfile,&fileinfo) != 0) {
      cmd_display(mode,channel, _("Sorry, passwordfile not found"),1);
      return;
    }
    switch (fileinfo.st_size) {
    case 1620: /* file without CR/LF */
      multiply = 27;
      break;
    case 1680: /* file with CR or LF */
      multiply = 28;
      break;
    case 1740: /* file with CR and LF */
      multiply = 29;
      break;
    default:
      cmd_display(mode,channel, _("Passwordfile has incorrect length"),1);
      return;
    }
    if ((fd = fopen(passfile,"r")) != NULL) /* open box_passfile */
    {
      /* Format of box_login string is "22.07.(19)94 16:23" */
      strcpy (privstr, pw_stat[channel].box_login);
      i = strlen (privstr) - 5;
      privstr[2] = '\0';
      privstr[i + 2] = '\0';
      ofs = atoi (privstr) + atoi (privstr + i + 3); /* calculate offset for box_passfile */
      if (ofs >= 60) ofs -= 60;
      ofs = ofs * multiply + atoi (privstr + i);
      fseek(fd,ofs,0);                /* seek to valid priv position */
      fscanf(fd,"%4s",priv);          /* get the 4 priv characters */
      fclose(fd);
      sprintf(privstr,"PRIV %s%s",priv,rem_newlin_str);
      rem_data_display(channel,privstr);
      queue_cmd_data(channel,X_DATA,strlen(privstr),mode,privstr);
    }
    else
      cmd_display(mode,channel, _("Sorry, passwordfile not found"),1);
  }
  else {
    cmd_display(mode,channel, _("Sorry, no Logintime recorded"),1);
  }
}

/* reload password file */
void cmd_loadpriv(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  clear_pwfile();  
  load_pwfile();
  cmd_display(mode,channel,OK_TEXT,1);
  return;
}

/* list password file data */
void cmd_listpriv(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  struct calllist *calllist_wrk;
  int i;
  int found;
  char pwtype[20];
  char disp_line[83];
  
  if (calllist_root == NULL) {
    cmd_display(mode,channel, _("No lines found"),1);
    return;
  }
  calllist_wrk = calllist_root;
  while (calllist_wrk != NULL) {
    i = 0;
    found = 0;
    while ((!found) && (pwmodes[i].pwtype != NULL)) {
      if (pwmodes[i].pwmode == calllist_wrk->pwmode) {
        found = 1;
        strcpy(pwtype,pwmodes[i].pwtype);
      }
      i++;
    }
    if (!found) {
      cmd_display(mode,channel, _("Calllist corrupt!"),1);
      return;
    }
    
    switch (calllist_wrk->pwmode) {
    case PW_DIEBOX:
      sprintf(disp_line,"%s %s %s",
              calllist_wrk->callsign, pwtype, calllist_wrk->pw_file);
      break;
    case PW_FLEXNET:
    case PW_MD2:
    case PW_MD5:
      sprintf(disp_line,"%s %s %s %s",
              calllist_wrk->callsign, pwtype, calllist_wrk->pw_file,
              calllist_wrk->priv_string);
      break;
    case PW_BAYCOM:
    
    case PW_CLUSTER:	/* fuer DXCluster */
      sprintf(disp_line,"%s %s %s %s",
      	      calllist_wrk->callsign, pwtype, calllist_wrk->pw_file,
      	      calllist_wrk->priv_string);
      break;
    
    case PW_THENET:
      sprintf(disp_line,"%s %s %s %d %s",
              calllist_wrk->callsign, pwtype, calllist_wrk->pw_file,
              calllist_wrk->flags,calllist_wrk->priv_string);
      break;
    }
    cmd_display(mode,channel,disp_line,1);
    calllist_wrk = calllist_wrk->next;
  }
}

/* generate password */
void cmd_priv(par1,par2,channel,len,mode,str)
int par1;
int par2;
int channel;
int len;
int mode;
char *str;
{
  if (ch_stat[channel].conn_state != CS_CONN) {
    cmd_display(mode,channel, _("Only while connected"),1);
    return;
  }
  switch (pw_stat[channel].pwmode) {
  case PW_DIEBOX:
    priv_diebox(channel,mode);
    return;
  case PW_FLEXNET:
    priv_flexnet(channel,mode);
    return;
  case PW_BAYCOM:
    priv_baycom(channel,mode);
    return;
  case PW_THENET:
    priv_thenet(channel,mode);
    return;
  case PW_MD2:
    priv_md2(channel,mode);
    return;
  case PW_MD5:
    priv_md5(channel,mode);
    return;
  case PW_CLUSTER:		/* DXCluster Passwort */
    priv_cluster(channel,mode);
    return;
    
  default:
    cmd_display(mode,channel, _("No password available"),1);
    return;
  }
}

/* init of priv data */
void init_priv()
{
  int i;
  
  for (i=0;i<tnc_channels;i++) {
    clear_pwmode(i);
  }
  calllist_root = NULL;
  load_pwfile();
}

/* exit of priv data */
void exit_priv()
{
  clear_pwfile();
}

void free_priv()
{
  free(pw_stat);
}

int alloc_priv()
{
  pw_stat = (struct password_stat *)
    malloc(tnc_channels * sizeof(struct password_stat));
  return(pw_stat == NULL);
}

/* MD2C.C - RSA Data Security, Inc., MD2 message-digest algorithm
 */

/* Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
   rights reserved.

   License to copy and use this software is granted for
   non-commercial Internet Privacy-Enhanced Mail provided that it is
   identified as the "RSA Data Security, Inc. MD2 Message Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
 */


/* Permutation of 0..255 constructed from the digits of pi. It gives a
   "random" nonlinear byte substitution operation.
 */
static unsigned char PI_SUBST[256] = {
  41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6,
  19, 98, 167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188,
  76, 130, 202, 30, 155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24,
  138, 23, 229, 18, 190, 78, 196, 214, 218, 158, 222, 73, 160, 251,
  245, 142, 187, 47, 238, 122, 169, 104, 121, 145, 21, 178, 7, 63,
  148, 194, 16, 137, 11, 34, 95, 33, 128, 127, 93, 154, 90, 144, 50,
  39, 53, 62, 204, 231, 191, 247, 151, 3, 255, 25, 48, 179, 72, 165,
  181, 209, 215, 94, 146, 42, 172, 86, 170, 198, 79, 184, 56, 210,
  150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241, 69, 157,
  112, 89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2, 27,
  96, 37, 173, 174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15,
  85, 71, 163, 35, 221, 81, 175, 58, 195, 92, 249, 206, 186, 197,
  234, 38, 44, 83, 13, 110, 133, 40, 132, 9, 211, 223, 205, 244, 65,
  129, 77, 82, 106, 220, 55, 200, 108, 193, 171, 250, 36, 225, 123,
  8, 12, 189, 177, 74, 120, 136, 149, 139, 227, 99, 232, 109, 233,
  203, 213, 254, 59, 0, 29, 57, 242, 239, 183, 14, 102, 88, 208, 228,
  166, 119, 114, 248, 235, 117, 75, 10, 49, 68, 80, 180, 143, 237,
  31, 26, 219, 153, 141, 51, 159, 17, 131, 20
};

static unsigned char *PADDING_MD2[] = {
  (unsigned char *)"",
  (unsigned char *)"\001",
  (unsigned char *)"\002\002",
  (unsigned char *)"\003\003\003",
  (unsigned char *)"\004\004\004\004",
  (unsigned char *)"\005\005\005\005\005",
  (unsigned char *)"\006\006\006\006\006\006",
  (unsigned char *)"\007\007\007\007\007\007\007",
  (unsigned char *)"\010\010\010\010\010\010\010\010",
  (unsigned char *)"\011\011\011\011\011\011\011\011\011",
  (unsigned char *)"\012\012\012\012\012\012\012\012\012\012",
  (unsigned char *)"\013\013\013\013\013\013\013\013\013\013\013",
  (unsigned char *)"\014\014\014\014\014\014\014\014\014\014\014\014",
  (unsigned char *)
    "\015\015\015\015\015\015\015\015\015\015\015\015\015",
  (unsigned char *)
    "\016\016\016\016\016\016\016\016\016\016\016\016\016\016",
  (unsigned char *)
    "\017\017\017\017\017\017\017\017\017\017\017\017\017\017\017",
  (unsigned char *)
    "\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020"
};

static void MD2_memcpy (output, input, len)
unsigned char *output;
unsigned char *input;
unsigned int len;
{
  unsigned int i;

  for (i = 0; i < len; i++)
    output[i] = input[i];
}

static void MD2_memset (output, value, len)
unsigned char *output;
int value;
unsigned int len;
{
  unsigned int i;

  for (i = 0; i < len; i++)
    ((char *)output)[i] = (char)value;
}
         
/* MD2 basic transformation. Transforms state and updates checksum
     based on block.
 */
static void MD2Transform (state, checksum, block)
unsigned char state[16];
unsigned char checksum[16];
unsigned char block[16];
{
  unsigned int i, j, t;
  unsigned char x[48];

  /* Form encryption block from state, block, state ^ block.
   */
  MD2_memcpy ((unsigned char *)x, (unsigned char *)state, 16);
  MD2_memcpy ((unsigned char *)x+16, (unsigned char *)block, 16);
  for (i = 0; i < 16; i++)
    x[i+32] = state[i] ^ block[i];

  /* Encrypt block (18 rounds).
   */
  t = 0;
  for (i = 0; i < 18; i++) {
    for (j = 0; j < 48; j++)
      t = x[j] ^= PI_SUBST[t];
    t = (t + i) & 0xff;
  }

  /* Save new state */
  MD2_memcpy ((unsigned char *)state, (unsigned char *)x, 16);

  /* Update checksum.
   */
  t = checksum[15];
  for (i = 0; i < 16; i++)
    t = checksum[i] ^= PI_SUBST[block[i] ^ t];

  /* Zeroize sensitive information.
   */
  MD2_memset ((unsigned char *)x, 0, sizeof (x));
}

/* MD2 initialization. Begins an MD2 operation, writing a new context.
 */
static void MD2Init (context2)
MD2_CTX *context2;                                        /* context */
{
  context2->count = 0;
  MD2_memset ((unsigned char *)context2->state, 0, sizeof (context2->state));
  MD2_memset ((unsigned char *)context2->checksum, 0, sizeof (context2-> checksum));
}

/* MD2 block update operation. Continues an MD2 message-digest
     operation, processing another message block, and updating the
     context.
 */
static void MD2Update (context2, input, inputLen)
MD2_CTX *context2;                                        /* context */
unsigned char *input;                                /* input block */
unsigned int inputLen;                     /* length of input block */
{
  unsigned int i, index, partLen;

  /* Update number of bytes mod 16 */
  index = context2->count;
  context2->count = (index + inputLen) & 0xf;

  partLen = 16 - index;

  /* Transform as many times as possible.
    */
  if (inputLen >= partLen) {
    MD2_memcpy ((unsigned char *)&context2->buffer[index],
            (unsigned char *)input, partLen);
    MD2Transform (context2->state, context2->checksum, context2->buffer);

    for (i = partLen; i + 15 < inputLen; i += 16)
      MD2Transform (context2->state, context2->checksum, &input[i]);

    index = 0;
  }
  else
    i = 0;

  /* Buffer remaining input */
  MD2_memcpy ((unsigned char *)&context2->buffer[index],
          (unsigned char *)&input[i], inputLen-i);
}

/* MD2 finalization. Ends an MD2 message-digest operation, writing the
     message digest and zeroizing the context.
 */
static void MD2Final (digest, context2)
unsigned char digest[16];                         /* message digest */
MD2_CTX *context2;                                        /* context */
{
  unsigned int index, padLen;

  /* Pad out to multiple of 16.
   */
  index = context2->count;
  padLen = 16 - index;
  MD2Update (context2, PADDING_MD2[padLen], padLen);

  /* Extend with checksum */
  MD2Update (context2, context2->checksum, 16);

  /* Store state in digest */
  MD2_memcpy ((unsigned char *)digest, (unsigned char *)context2->state, 16);

  /* Zeroize sensitive information.
   */
  MD2_memset ((unsigned char *)context2, 0, sizeof (*context2));
}

/* end of MD2 code */

/* DG2GCC 03.01.2000 */
/* MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/* Constants for MD5Transform routine.
 */


#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static void MD5Transform PROTO_LIST ((UINT4 [4], unsigned char [64]));
static void Encode PROTO_LIST
  ((unsigned char *, UINT4 *, unsigned int));
static void Decode PROTO_LIST
  ((UINT4 *, unsigned char *, unsigned int));
static void MD5_memcpy PROTO_LIST ((POINTER, POINTER, unsigned int));
static void MD5_memset PROTO_LIST ((POINTER, int, unsigned int));

static unsigned char PADDING_MD5[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void MD5Init (context)
MD5_CTX *context;                                        /* context */
{
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.
 */
void MD5Update (context, input, inputLen)
MD5_CTX *context;                                        /* context */
unsigned char *input;                                /* input block */
unsigned int inputLen;                     /* length of input block */
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits (funktioniert das in den naechsten 2 Zeilen?) */
  if ((context->count[0] += ((UINT4)inputLen << 3)) \
   < ((UINT4)inputLen << 3))
  context->count[1]++;
  context->count[1] += ((UINT4)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
   */
  if (inputLen >= partLen) {
    MD5_memcpy
    ((POINTER)&context->buffer[index], (POINTER)input, partLen);
    MD5Transform (context->state, context->buffer);

    for (i = partLen; i + 63 < inputLen; i += 64)
      MD5Transform (context->state, &input[i]);

    index = 0;
  }
  else
  i = 0;

  /* Buffer remaining input */
  MD5_memcpy
  ((POINTER)&context->buffer[index], (POINTER)&input[i],
  inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void MD5Final (digest, context)
unsigned char digest[16];                         /* message digest */
MD5_CTX *context;                                       /* context */
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
  */
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context, PADDING_MD5, padLen);

  /* Append length (before padding) */
  MD5Update (context, bits, 8);

  /* Store state in digest */
  Encode (digest, context->state, 16);

/* Zeroize sensitive information.
 */
  MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block. */

static void MD5Transform (state, block)
UINT4 state[4];
unsigned char block[64];
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

  /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

/* Zeroize sensitive information.
 */
  MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
static void Encode (output, input, len)
unsigned char *output;
UINT4 *input;
unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
    output[j] = (unsigned char)(input[i] & 0xff);
    output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
    output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
    output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
static void Decode (output, input, len)
UINT4 *output;
unsigned char *input;
unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
    (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.
 */

static void MD5_memcpy (output, input, len)
POINTER output;
POINTER input;
unsigned int len;
{
  unsigned int i;

  for (i = 0; i < len; i++)

  output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
 */
static void MD5_memset (output, value, len)
POINTER output;
int value;
unsigned int len;
{
  unsigned int i;

  for (i = 0; i < len; i++)
  ((char *)output)[i] = (char)value;
}

/* end of MD5 code */
