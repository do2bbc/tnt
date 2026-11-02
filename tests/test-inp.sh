#!/bin/sh
set -eu
if test -f "$(dirname "$0")/../src/xmon.c"; then
  cd "$(dirname "$0")/.."
  source=src/xmon.c
  display=src/display.c
  includes=src
else
  cd "$(dirname "$0")"
  source=tnt-xmon.c
  display=tnt-display.c
  includes=.
fi
tmp=$(mktemp -d)
trap 'rm -f "$tmp/test.c" "$tmp/test"; rmdir "$tmp"' EXIT
cat > "$tmp/test.c" <<'EOF'
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "xmon.h"
#define _(s) (s)
static char output[32768];
static void moni_display2(const char *s,int attr) {
  (void)attr;
  assert(strlen(output)+strlen(s)+2 < sizeof(output));
  strcat(output,s);strcat(output,"\n");
}
static void moni_display_len2(char *s,int n) {(void)s;(void)n;}
EOF
sed -n '/^static void fill_call/,$p' "$source" >> "$tmp/test.c"
sed -n '/^static int monitor_is_iframe/,/^void moni_display/ { /^void moni_display/!p; }' "$display" >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
static void axcall(unsigned char *p) {
  int i;for(i=0;i<6;i++)p[i]=(unsigned char)("DB0ABC"[i]<<1);p[6]=0x60;
}
static void run(const unsigned char *p,size_t n,int is_i) {
  char *buffer=malloc(n+1);
  assert(n && n<=256);
  buffer[0]=(char)(n-1);memcpy(buffer+1,p,n);output[0]=0;
  assert(display_layer3(buffer,is_i)==0);
  free(buffer);
}
int main(void) {
  unsigned char packet[256]={0xff}, test[256], nodes[28]={0xff};
  size_t n=1,i;
  assert(monitor_is_iframe("R12: fm DNO284 to DNO484 I20^ pid CF"));
  assert(monitor_is_iframe("T12: fm DNO484 to DNO284 I20+ pid CF"));
  assert(monitor_is_iframe("fm DNO284 to DNO484 ctl I20 pid CF"));
  assert(!monitor_is_iframe("R12: fm DNO284 to NODES UI pid CF"));
  assert(!monitor_is_iframe("R12: fm DNO284 to DNO484 RR2v"));
  assert(!monitor_is_iframe(" I"));
  assert(!monitor_is_iframe(" I2"));
  assert(!monitor_is_iframe(" I20ABC pid CF"));
  axcall(packet+n);n+=7;packet[n++]=3;packet[n++]=0x12;packet[n++]=0x34;
  packet[n++]=5;packet[n++]=0;memcpy(packet+n,"ABC",3);n+=3;
  packet[n++]=7;packet[n++]=1;packet[n++]=44;packet[n++]=1;
  packet[n++]=2;packet[n++]=3;packet[n++]=24;
  packet[n++]=4;packet[n++]=77;packet[n++]=0xff;packet[n++]=0x80;packet[n++]=0;
  run(packet,n,1);
  assert(strstr(output,"DB0ABC hops: 3 transport time: 4660"));
  assert(strstr(output,"alias: ABC"));
  assert(strstr(output,"IP: 44.1.2.3/24"));
  assert(strstr(output,"option 77 (2 bytes)"));
  assert(!strstr(output,"malformed"));
  for(i=1;i<n;i++) {
    run(packet,i,1);
    if(i>1)assert(strstr(output,"malformed"));
  }
  memcpy(test,packet,n);test[11]=1;run(test,n,1);assert(strstr(output,"malformed"));
  memcpy(test,packet,n);test[11]=255;run(test,n,1);assert(strstr(output,"malformed"));
  memcpy(test,packet,n);test[23]=33;run(test,n,1);assert(strstr(output,"malformed"));
  n=1;
  for(i=0;i<13;i++) {axcall(test+n);n+=7;test[n++]=1;test[n++]=0;test[n++]=10;test[n++]=0;}
  test[0]=0xff;run(test,n,1);assert(!strstr(output,"malformed"));
  memcpy(nodes+1,"CLASS ",6);axcall(nodes+7);memcpy(nodes+14,"ABC   ",6);
  axcall(nodes+20);nodes[27]=200;
  run(nodes,sizeof(nodes),0);
  assert(strstr(output,"Broadcast from CLASS"));
  assert(strstr(output,"quality: 200"));
  assert(!strstr(output,"INP"));
  for(i=1;i<=6;i++) {
    memset(test,0,20);axcall(test);axcall(test+7);test[19]=(unsigned char)i;
    if(i==1) {axcall(test+21);axcall(test+28);}
    run(test,i==1?35:i==2?21:20,1);
    assert(!strstr(output,"IllOpcode"));
  }
  puts("INP entries, options, truncation, long frames, classic NODES and L4 controls: PASS");
  return 0;
}
EOF
${CC:-cc} -Wall -Wextra -Werror -fsanitize=address,undefined -g -I"$includes" "$tmp/test.c" -o "$tmp/test"
"$tmp/test"
