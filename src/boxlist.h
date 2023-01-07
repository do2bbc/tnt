/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993-1997 by Mark Wahl
   For license details see documentation
   include file for boxlist utility (boxlist.h)
   created: Mark Wahl DL4YBG 94/10/03
   updated: Mark Wahl DL4YBG 97/08/16
*/


/* types of boxlistfiles */
#define BL_NORMAL 0

/* boxlist functions */
#define FUNC_READ 0
#define FUNC_LIST 1
#define FUNC_ERASE 2
#define FUNC_KILL 3
#define FUNC_TRANSFER 4
#define FUNC_LIFETIME 5
#define FUNC_NONE -1

struct boxlist_file {
  char name[MAXCHAR];
  int type;
  int fd;
  int len;
  int lines;
  int curline;
  int mode;
  char str[MAXCOLS+1];
  int strlen;
};

struct tmpname_entry {
  char *name;
  struct tmpname_entry *next;
};

