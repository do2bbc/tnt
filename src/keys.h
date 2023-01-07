/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993,1994 by Mark Wahl
   For license details see documentation
   include file for keyboard codes (keys.h)
   created: Mark Wahl DL4YBG 94/02/05
   updated: Mark Wahl DL4YBG 94/12/19
   updated: mayer hans oe1smc, 6.1.1999 
   updated: Berndt Josef Wulf, 99/02/17
   updated: Berndt Josef Wulf VK5ABN, 99/03/28
   updated: Christoph Berg DF7CB, 00/04/08: removed special xterm section,
                                            merged different OSs
*/

/* keycodes */
static struct func_keys special_keys[] = {
 {"\033[11~",	1,1},		/* F1 */
 {"\033[[A",	1,1},		/* F1 linux */
/* {"\033[28~",	1,1},		 * F1 xterm ??? */
 {"\033OP",	1,1},		/* F1 xterm, ISC */
 {"\033[224z",	1,1},		/* F1 Solaris */
 {"\033[12~",	1,2},		/* F2 */
 {"\033[[B",	1,2},		/* F2 linux */
/* {"\033[33~",	1,2},		 * F2 xterm ??? */
 {"\033OQ",	1,2},		/* F2 xterm, ISC */
 {"\033[225z",	1,2},		/* F2 Solaris */
 {"\033[13~",	1,3},		/* F3 */
 {"\033[[C",	1,3},		/* F3 linux */
/* {"\033[34~",	1,3},		 * F3 xterm ??? */
 {"\033OR",	1,3},		/* F3 xterm, ISC */
 {"\033[226z",	1,3},		/* F3 Solaris */
 {"\033[14~",	1,4},		/* F4 */
 {"\033[[D",	1,4},		/* F4 linux */
/* {"\033[29~",	1,4},		 * F4 xterm ??? */
 {"\033OS",	1,4},		/* F4 xterm, ISC */
 {"\033[227z",	1,4},		/* F4 Solaris */
 {"\033[15~",	1,5},		/* F5 */
 {"\033[[E",	1,5},		/* F5 linux */
/* {"\033[32~",	1,5},		 * F5 xterm ??? */
 {"\033OT",	1,5},		/* F5 ISC */
 {"\033[228z",	1,5},		/* F5 Solaris */
 {"\033[17~",	1,6},		/* F6 */
 {"\033OU",	1,6},		/* F6 ISC */
 {"\033[229z",	1,6},		/* F6 Solaris */
 {"\033[18~",	1,7},		/* F7 */
 {"\033OV",	1,7},		/* F7 ISC */
 {"\033[230z",	1,7},		/* F7 Solaris */
 {"\033[19~",	1,8},		/* F8 */
 {"\033OW",	1,8},		/* F8 ISC */
 {"\033[231z",	1,8},		/* F8 Solaris */
 {"\033[20~",	1,9},		/* F9 */
 {"\033OX",	1,9},		/* F9 ISC */
 {"\033[232z",	1,9},		/* F9 Solaris */
#ifdef TNT_CHAMBER
 {"\033[21~",	1,10},		/* F10 */
 {"\033OY",	1,10},		/* F10 ISC */
 {"\033[233z",	1,10},		/* F10 Solaris */
#else
 {"\033[21~",	1,0},		/* F10 */
 {"\033OY",	1,0},		/* F10 ISC */
 {"\033[233z",	1,0},		/* F10 Solaris */
#endif
 {"\033[23~",	0,C_MONITOR},	/* F11 */
 {"\033OZ",	0,C_MONITOR},	/* F11 ISC */
 {"\033[192z",	0,C_MONITOR},	/* F11 Solaris */
 {"\033[24~",	0,C_COMMAND},	/* F12 */
 {"\033OA",	0,C_COMMAND},	/* F12 ISC */
 {"\033[193z",	0,C_COMMAND},	/* F12 Solaris */
/* {"\033[23~",	2,1},		 * SF1 identical to F11 */
 {"\033Op",	2,1},		/* SF1 ISC */
 {"\033[q",	2,1},		/* SF1 Solaris */
/* {"\033[24~",	2,2},		 * SF2 identical to F12 */
 {"\033Oq",	2,2},		/* SF2 ISC */
 {"\033[w",	2,2},		/* SF2 Solaris */
 {"\033[25~",	2,3},		/* SF3 */
 {"\033Or",	2,3},		/* SF3 ISC */
 {"\033[e",	2,3},		/* SF3 Solaris */
 {"\033[26~",	2,4},		/* SF4 */
 {"\033Os",	2,4},		/* SF4 ISC */
 {"\033[r",	2,4},		/* SF4 Solaris */
 {"\033[28~",	2,5},		/* SF5 */
 {"\033Ot",	2,5},		/* SF5 ISC */
 {"\033[t",	2,5},		/* SF5 Solaris */
 {"\033[29~",	2,6},		/* SF6 */
 {"\033Ou",	2,6},		/* SF6 ISC */
 {"\033[z",	2,6},		/* SF6 Solaris */
 {"\033[31~",	2,7},		/* SF7 */
 {"\033Ov",	2,7},		/* SF7 ISC */
 {"\033[u",	2,7},		/* SF7 Solaris */
 {"\033[32~",	2,8},		/* SF8 */
 {"\033Ow",	2,8},		/* SF8 ISC */
 {"\033[i",	2,8},		/* SF8 Solaris */
 {"\033[33~",	2,9},		/* SF9 */
 {"\033Ox",	2,9},		/* SF9 ISC */
 {"\033[o",	2,9},		/* SF9 Solaris */
 {"\033[34~",	2,10},		/* SF10 */
 {"\033Oy",	2,10},		/* SF10 ISC */
 {"\033[p",	2,10},		/* SF10 Solaris */
 {"\033[23$",	2,11},		/* SF11 rxvt */
 {"\033Oz",	2,11},		/* SF11 ISC */
 {"\033[24$",	2,12},		/* SF12 rxvt */
 {"\033Oa",	2,12},		/* SF12 ISC */

 {"\033[A",	0,C_CUUP},	/* CURSOR UP */
 {"\033[B",	0,C_CUDWN},	/* CURSOR DOWN */
 {"\033[C",	0,C_CURIGHT},	/* CURSOR RIGHT */
 {"\033[D",	0,C_CULEFT},	/* CURSOR LEFT */
 {"\033[1~",	0,C_CUTOP},	/* HOME */
 {"\033[7~",	0,C_CUTOP},	/* HOME */
 {"\033[H",	0,C_CUTOP},	/* HOME */
 {"\033[4~",	0,C_CUBOT},	/* END */
 {"\033[8~",	0,C_CUBOT},	/* END */
/* {"\033Ow",	0,C_CUBOT},	   END ??? */
 {"\033[5~",	0,C_WINUP},	/* PAGEUP */
 {"\033[V",	0,C_WINUP},     /* PAGEUP ISC */
 {"\033[6~",	0,C_WINDWN},	/* PAGEDWN */
 {"\033[U",	0,C_WINDWN},	/* PAGEDWN ISC */
 {"\033[2~",	0,C_INSERT},	/* INSERT */
 {"\033[@",	0,C_INSERT},	/* INSERT ISC */
 {"\033[3~",	0,C_DELCHAR},	/* DELETE */
 
 {"\033m",	0,C_MONITOR},	/* ALT-m */
 {"\033c",	0,C_COMMAND},	/* ALT-c */
 {"\033q",	0,C_CONNECT},	/* ALT-q */
 {"\033b",	0,C_MAILBOX},	/* ALT-b */
 {"\033x",	0,C_EXTMON},	/* ALT-x */
 {"\033l",	0,C_BOXLIST},	/* ALT-l */
 {"\033s",	0,C_MHEARD},	/* ALT-s */
 {"\033h",	0,C_HELP},	/* ALT-h */
 {"\033p",	0,C_PAUSE},	/* ALT-p */
 {"\033M",	0,C_MONITOR},	/* ALT-M */
 {"\033C",	0,C_COMMAND},	/* ALT-C */
 {"\033Q",	0,C_CONNECT},	/* ALT-Q */
 {"\033B",	0,C_MAILBOX},	/* ALT-B */
 {"\033X",	0,C_EXTMON},	/* ALT-X */
 {"\033L",	0,C_BOXLIST},	/* ALT-L */
 {"\033S",	0,C_MHEARD},	/* ALT-S */
 {"\033H",	0,C_HELP},	/* ALT-H */
 {"\033P",	0,C_PAUSE},	/* ALT-P */
 {"\0331",	2,1},		/* ALT-1 */
 {"\0332",	2,2},		/* ALT-2 */
 {"\0333",	2,3},		/* ALT-3 */
 {"\0334",	2,4},		/* ALT-4 */
 {"\0335",	2,5},		/* ALT-5 */
 {"\0336",	2,6},		/* ALT-6 */
 {"\0337",	2,7},		/* ALT-7 */
 {"\0338",	2,8},		/* ALT-8 */
 {"\0339",	2,9},		/* ALT-9 */
 {"\0330",	2,10},		/* ALT-0 */
 {"\033-",	2,11},		/* ALT-- */
 {"\033=",	2,12}		/* ALT-= */
};

#define NUMFUNC 119 /* number key definitions */

