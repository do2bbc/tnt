#!/bin/sh
set -eu
source_dir=${top_srcdir:-$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)}
build_dir=${top_builddir:-$source_dir}
tmp=$(mktemp -d)
trap 'rm -f "$tmp/test.c" "$tmp/test"; rmdir "$tmp"' EXIT HUP INT TERM
cat > "$tmp/test.c" <<'EOF'
#include <assert.h>
#include "tnt.h"
#include "connect.h"

static struct channel_stat channels[2];
static struct channel_stat *ch_stat = channels;
static struct xconn_stat crossconnects[2];
static struct xconn_stat *xc_stat = crossconnects;
static int altstat, status_updates, overview_updates, name_updates;
static char overview_call[31], status_call[31];
static char connect_str[] = "onnected to ";
static char cmpstr[][20] = {"failure"};
#define NUM_CMPSTR 1

static void update_name(int channel)
{
  assert(channel == 1);
  ++name_updates;
}

static void stat_display(int channel)
{
  assert(channel == 1);
  strcpy(status_call, ch_stat[channel].disp_call);
  ++status_updates;
}

void statlin_update(void)
{
  strcpy(overview_call, ch_stat[1].disp_call);
  ++overview_updates;
}
EOF
awk '
  /^static int test_connected\(/ { found = 1 }
  found { print }
  found && /^}/ { exit }
  END { if (!found) exit 1 }
' "$source_dir/src/connect.c" >> "$tmp/test.c"
cat >> "$tmp/test.c" <<'EOF'
int main(void)
{
  char connected[] = "*** connected to DNO284\r";
  char reconnected[] = "*** reconnected to DNO484\r";
  char ordinary[] = "Next received line\r";
  char failure[] = "failure\r";
  char incomplete[] = "*** reconnected to \r";
  for (altstat = 0; altstat <= 1; ++altstat) {
    status_updates = overview_updates = name_updates = 0;
    strcpy(ch_stat[1].disp_call, "DNO486");
    strcpy(ch_stat[1].log_call, "DNO486");
    assert(test_connected(1, connected, sizeof(connected) - 1) == T_CON);
    assert(strcmp(status_call, "DNO284") == 0);
    assert(strcmp(overview_call, "DNO284") == 0);
    assert(strcmp(ch_stat[1].log_call, "DNO284") == 0);
    assert(status_updates == 1 && overview_updates == 1);
    assert(name_updates == altstat);

    /* No further received line or keyboard event may be needed. */
    assert(test_connected(1, reconnected, sizeof(reconnected) - 1) == T_RECON);
    assert(strcmp(status_call, "DNO484") == 0);
    assert(strcmp(overview_call, "DNO484") == 0);
    assert(strcmp(ch_stat[1].log_call, "DNO284") == 0);
    assert(status_updates == 2 && overview_updates == 2);
    assert(name_updates == 2 * altstat);

    test_connected(1, ordinary, sizeof(ordinary) - 1);
    test_connected(1, incomplete, sizeof(incomplete) - 1);
    xc_stat[1].active = 1;
    assert(test_connected(1, failure, sizeof(failure) - 1) == T_RECON);
    xc_stat[1].active = 0;
    assert(status_updates == 2 && overview_updates == 2);
  }
  puts("Connect/reconnect callsigns immediately refresh both status displays: PASS");
  return 0;
}
EOF
${CC:-cc} -g -O2 -Wall -Wextra -Wstrict-prototypes \
  -Wold-style-definition -Werror -D_FORTIFY_SOURCE=3 \
  -fsanitize=address,undefined -I"$build_dir" -I"$source_dir/src" \
  "$tmp/test.c" -o "$tmp/test"
"$tmp/test"
