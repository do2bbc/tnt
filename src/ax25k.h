/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993-1997 by Mark Wahl
   For license details see documentation
   include file for AX25-Kernel-Interface (ax25k.h)
   created: Mark Wahl DL4YBG 97/02/16
   updated: Mark Wahl DL4YBG 97/02/24
*/

/* definitions of messages for timestamping */
#define ST_EVER 0
#define ST_MONI 1
#define ST_STAT 2

/* definitions of monitor frames to display */
#define MONI_UNPR 1
#define MONI_INFO 2
#define MONI_SUPV 4
#define MONI_CONN 8

#ifdef USE_AX25K
int init_ax25k(void);
void exit_ax25k(void);
int alloc_ax25k(void);
void free_ax25k(void);
void init_statline(void);
void status_ax25k(int always);
void ax25k_fdset(int *max_fd, fd_set *fdmask);
int ax25k_receive(fd_set *fdmask);
void write_ax25k(int channel, char *buffer, int len);
void command_ax25k(int channel, char *data, int len);
#endif
