/* tnt: Hostmode Terminal for TNC
   Copyright (C) 1993-1997 by Mark Wahl
   For license details see documentation
   include file for remote passwords (priv.h)
   created: Mark Wahl DL4YBG 94/12/21
   updated: Mark Wahl DL4YBG 97/01/22
*/

/* DXClusterergaenzungen Henning Folger, DL6DH, 06.02.1999 */

#define PW_NONE 0
#define PW_DIEBOX 1
#define PW_FLEXNET 2
#define PW_THENET 3
#define PW_BAYCOM 4
#define PW_MD2 5
#define PW_CLUSTER 6
#define PW_MD5 7

/* Flags for THENET-Password */
#define PWTN_MORETRIES 1
#define PWTN_HIDESTRING 2
#define PWTN_HIDEPERFECT 4
#define PWTN_MAXVAL PWTN_MORETRIES+PWTN_HIDESTRING+PWTN_HIDEPERFECT
/* values for THENET-Password */
#define PWTN_TRIES 3
#define PWTN_STRLEN 72

struct pwmodes {
  char pwtype[20];
  int pwmode;
};

struct password_stat {
  int pwmode;
  struct calllist *entry;
  char box_login[15]; /* login time for DieBox */
  int pass_wait; /* waiting for password-generation string */
  int tries;
  int valid_try;
};

struct calllist {
  char callsign[10];
  int pwmode;
  char pw_file[MAXCHAR];
  int flags;
  char priv_string[40];
  struct calllist *next;
};

/* MD2.H - header file for MD2C.C
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

typedef struct {
  unsigned char state[16];                                 /* state */
  unsigned char checksum[16];                           /* checksum */
  unsigned int count;                 /* number of bytes, modulo 16 */
  unsigned char buffer[16];                         /* input buffer */
} MD2_CTX;

/* DG2GCC 03.01.2000 */
/* MD5.H - header file for MD5C.C
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

/* PROTOTYPES should be set to one if and only if the compiler supports
  function argument prototyping.
  The following makes PROTOTYPES default to 0 if it has not already
  been defined with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

void MD5Init PROTO_LIST ((MD5_CTX *));
void MD5Update PROTO_LIST
  ((MD5_CTX *, unsigned char *, unsigned int));
void MD5Final PROTO_LIST ((unsigned char [16], MD5_CTX *));

