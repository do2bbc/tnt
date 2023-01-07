/* TNT - RUN-PROGRAMM (logcall)
 *
 * Version 2
 *
 * Written by Matthias Hensler, 25.8.98
 * Rewritten by Matthias Hensler, 2000/01/31
 * Copyright WSPse 1998-2000
 * eMail: wsp@gmx.de
 *
 * Free software. Redistribution and modify under the terms of GNU Public
 * License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "tntrun.h"

#define TNT_NAMES  TNT_INIT_PATH "/names.tnt"
#define TNT_ROUTES TNT_INIT_PATH "/routes.tnt"
#define TNT_LOG    TNT_MAIN_PATH "/tnt.log"

#define CUR_VER "2.00a"
#define CUR_DAT "2000/01/31"
#define MAXSIZE 512

char buffer[MAXSIZE];

char *copy_char_str(char *in)
{
  char *out;

  if(! in) return NULL;

  out = (char *) malloc(sizeof(char) * (strlen(in) + 1));
  if(! out) return NULL;

  strcpy(out, in);

  return out;
}

char *find_name(char *call)
{
  FILE *names_tnt;
  char *file, *name;
  int len;

  if(! call) return NULL;
  
  file = strchr(call, '-');
  if(file) {
    len = file-call;
  } else {
    len = strlen(call);
  }
  
  names_tnt = fopen(TNT_NAMES, "r");
  if(! names_tnt) {
    return NULL;
  }

  name = NULL;
  while(1) {
    fgets(buffer, MAXSIZE-1, names_tnt);
    if(feof(names_tnt)) break;

    file = buffer;
    if(strncasecmp(file, call, len) != 0) {
      file += 2;
      if(strncasecmp(file, call, len) != 0) {
	file = NULL;
      }
    }

    if(file) {
      file = strchr(file, ' ');
      if(file) {
	file++;
	name = strchr(file, ' ');
	if(name) *name = 0;
	name = strchr(file, '\n');
	if(name) *name = 0;
	name = strchr(file, '\r');
	if(name) *name = 0;
	name = strchr(file, ';');
	if(name) *name = 0;
	if(file) {
	  name = copy_char_str(file);
	  break;
	}
      }
    }
    name = NULL;
  }

  fclose(names_tnt);

  return name;
}

char *find_routes(char *call)
{
  FILE *routes_tnt;
  char *route, *pat;
  int len;

  if(! call) return NULL;

  pat = strchr(call, '-');
  if(pat) {
    len = pat-call;
  } else {
    len = strlen(call);
  }

  routes_tnt = fopen(TNT_ROUTES, "r");
  if(! routes_tnt) return NULL;

  route = NULL;
  while(1) {
    fgets(buffer, MAXSIZE-1, routes_tnt);
    if(feof(routes_tnt)) break;

    pat = buffer;
    if(strncasecmp(pat, call, len) == 0 || strncasecmp(pat+2, call, len) == 0) {
      pat = strrchr(buffer, '\n');
      if(pat) *pat = 0;
      pat = strrchr(buffer, '\r');
      if(pat) *pat = 0;
      if(! route) {
	route = copy_char_str(buffer);
      } else {
	route = (char *) realloc(route, sizeof(char) * (strlen(route) + strlen(buffer) + 2));
	if(! route) break;
	strcat(route, "\n");
	strcat(route, buffer);
      }
    }
  }
  
  fclose(routes_tnt);
  
  return route;
}

char *select_next_digit(char *pos)
{
  if((! pos) || (! *pos)) return NULL;

  while(1) {
    pos++;
    if((! pos) || (! *pos)) return NULL;
    if(! isdigit(*pos)) break;
  }
  if(*pos == '|') return NULL;
  while(1) {
    pos++;
    if((! pos) || (! *pos)) return NULL;
    if(*pos == '|') return NULL;
    if(isdigit(*pos)) break;
  }

  return pos;
}

time_t find_time(char *str, int *last_year)
{
  struct tm ltm;
  char *pos;
  int yh;

  pos = str;

  if(! isdigit(*pos)) return -1;
  ltm.tm_mday = atoi(pos);
  if(ltm.tm_mday < 1 || ltm.tm_mday > 31) return -1;

  if((pos = select_next_digit(pos)) == NULL) return -1;
  ltm.tm_mon = atoi(pos)-1;
  if(ltm.tm_mon < 0 || ltm.tm_mon > 11) return -1;

  if((pos = select_next_digit(pos)) == NULL) return -1;
  ltm.tm_year = atoi(pos);
  if(ltm.tm_year < 100) {
    yh = (*last_year) / 100;
    if((yh*100 + ltm.tm_year) - (*last_year) < -30) {
      yh++;
    } else if((yh*100 + ltm.tm_year) - (*last_year) > 30) {
      yh--;
    }
    ltm.tm_year += (yh*100);
  } else if(ltm.tm_year < 1900) {
    /* yes I know that this might be a problem in 3800 */
    ltm.tm_year += 1900;
  }
  *last_year = ltm.tm_year;
  ltm.tm_year -= 1900;
  if(ltm.tm_year < 0) return -1;

  if((pos = select_next_digit(pos)) == NULL) return -1;
  ltm.tm_hour = atoi(pos);
  if(ltm.tm_hour < 0 || ltm.tm_hour > 23) return -1;

  if((pos = select_next_digit(pos)) == NULL) return -1;
  ltm.tm_min = atoi(pos);
  if(ltm.tm_min < 0 || ltm.tm_min > 59) return -1;

  if((pos = select_next_digit(pos)) != NULL) {
    ltm.tm_sec = atoi(pos);
    if(ltm.tm_sec < 0 || ltm.tm_sec > 59) ltm.tm_sec = 0;
  } else {
    ltm.tm_sec = 0;
  }

  return mktime(&ltm);
}

void find_connects(char *call)
{
  FILE *log_tnt;
  char *list, *pat;
  int len;
  int connects;
  time_t start, end, total, result;
  int last_year;
  int t_days, t_hours, t_min;
 
  if(! call) return;
  
  pat = strchr(call, '-');
  if(pat) {
    len = pat-call;
  } else {
    len = strlen(call);
  } 

  log_tnt = fopen(TNT_LOG, "r");
  if(! log_tnt) return;

  list      = NULL;
  total     = 0;
  last_year = 1970;
  connects  = 0;
  while(1) {
    fgets(buffer, MAXSIZE-1, log_tnt);
    if(feof(log_tnt)) break;

    if(strncasecmp(buffer, call, len) == 0) {
      pat = strrchr(buffer, '\n');
      if(pat) *pat = 0;
      pat = strrchr(buffer, '\r');
      if(pat) *pat = 0;
      connects++;
      if(! list) {
	list = copy_char_str(buffer);
      } else {
	if(connects > 5) {
	  pat = strchr(list, '\n');
	  memmove(list, pat+1, strlen(pat));
	}
	list = (char *) realloc(list, sizeof(char) * (strlen(list) + strlen(buffer) + 2));
	if(! list) return;
	strcat(list, "\n");
	strcat(list, buffer);
      }

      start = 0;
      end   = 0;
      pat   = buffer;
      while(1) {
	pat = strchr(pat, ' ');
	if(! pat) break;
	while(*pat == ' ') pat++;
	result = find_time(pat, &last_year);
	if(result != -1) {
	  if(! start) start = result;
	  else end = result;
	}
	if(end) {
	  total += (end-start)+60;
	  break;
	}
      }
    }
  }

  fclose(log_tnt);
  if(! connects) {
    printf("Was not connected in the last time.\n");
  } else {
    printf("Last connects:\n%s\nTotal connects     : %d\n", list, connects);
    free(list);
    t_days  = (total / 86400);
    t_hours = (total % 86400) / 3600;
    t_min   = (total % 3600) / 60;
    printf("Total connecttime  : ");
    if(t_days) {
      if(t_days == 1) printf("1 days ");
      else            printf("%d days ", t_days);
    }
    if(t_hours || t_days) {
      if(t_hours == 1) printf("1 hour ");
      else             printf("%d hours ", t_hours);
    }
    if(t_min == 1) printf("1 minute\n");
    else           printf("%d minutes\n", t_min);
    total = total / connects;
    t_days  = (total / 86400);
    t_hours = (total % 86400) / 3600;
    t_min   = (total % 3600) / 60;
    printf("Average connecttime: ");
    if(t_days) {
      if(t_days == 1) printf("1 days ");
      else            printf("%d days ", t_days);
    }
    if(t_hours || t_days) {
      if(t_hours == 1) printf("1 hour ");
      else             printf("%d hours ", t_hours);
    }
    if(t_min == 1) printf("1 minute\n");
    else           printf("%d minutes\n", t_min);
  }

  return;
}
  
void print_statistics(char *call)
{
  char *info_str;
  
  if(! call) return;
  
  printf("Informations about \"%s\"\n", call);

  info_str = find_name(call);
  printf("Name:   ");
  if(! info_str) {
    printf("unknown\n");
  } else {
    printf("%s\n", info_str);
    free(info_str);
  }

  info_str = find_routes(call);
  printf("Routes: ");
  if(! info_str) {
    printf("no route found (maybe DIRECT)\n");
  } else {
    if(strrchr(info_str, '\n')) putchar('\n');
    printf("%s\n", info_str);
    free(info_str);
  }

  find_connects(call);

  printf("----------\n");
}

int main(int argc, char **argv)
{
  int i;

  printf("\n----------\n");

  if(argc == 1) {
    print_statistics(GET_CALLSSID);
  } else {
    for(i=1;i<argc;i++) {
      print_statistics(argv[i]);
    }
  }

  printf("\nStatistics by WSPse, V" CUR_VER " (" CUR_DAT ").\n"
	 "usage: %s [Callsign 1] [Callsign 2] [...]\n\n", argv[0]);

  return EXIT_SUCCESS;
}

