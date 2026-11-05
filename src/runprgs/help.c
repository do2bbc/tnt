/* TNT - RUN-PROGRAMM (hilfe)
 *
 * Written by Matthias Hensler, 25.8.98
 * Copyright WSPse 1998
 * eMail: wsp@gmx.de
 *
 * Free software. Redistribution and modify under the terms of GNU Public
 * License.
 */

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <tntrun.h>

#define CUR_VER "1.0c"
#define CUR_DAT "31.1.00"

int main (void)
{
  struct stat st;
  struct dirent *de;
  DIR *run_dir;
  int tmp;
  char tmp_str[200];
  FILE *help_file;

  run_dir = opendir( TNT_RUN_PATH ); /* defined in tntrun.h */
  if(run_dir == NULL) {
    perror("diropen");
    return 1;
  }

  printf("List of executivable programs\n"
         "Start with \"//run <program>\".\n\n");

  while(1) {
    errno = 0;
    de = readdir(run_dir);
    if(de == NULL) {
      if(errno != 0) {
        perror("readdir");
        closedir(run_dir);
        return 1;
      }
      break;
    }
    if(de->d_name[0] != '.') {
      strcpy(tmp_str, TNT_RUN_PATH);
      strcat(tmp_str, "/");
      strcat(tmp_str, de->d_name);
      tmp = stat(tmp_str, &st);
      if(tmp != 0) {
        perror(tmp_str);
        closedir(run_dir);
        return 1;
      }
      if(tmp == 0) {
        tmp = S_ISREG(st.st_mode);
        if(tmp != 0) {
          strcpy(tmp_str, TNT_RUN_PATH);
          strcat(tmp_str, "/.");
          strcat(tmp_str, de->d_name);
          help_file = fopen(tmp_str, "r");
          if(help_file == NULL) {
            printf("<%s> (no Help available)\n\n", de->d_name);
          } else {
            printf("<%s>\n", de->d_name);
	    while(fgets(tmp_str, sizeof(tmp_str), help_file) != NULL) {
	      fputs(tmp_str, stdout);
	    }
            if(ferror(help_file)) {
              perror("reading help file");
              fclose(help_file);
              closedir(run_dir);
              return 1;
            }
            fclose(help_file);
            printf("\n");
          }
        }
      }
    }
  }
  closedir(run_dir);
  printf("\nby WSPse, V" CUR_VER "/" CUR_DAT ".\n");
  return 0;
}
