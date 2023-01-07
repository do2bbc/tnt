tnt_bin_dir	= @tnt_bin_dir@
tnt_conf_dir	= @tnt_conf_dir@
tnt_log_dir	= @tnt_log_dir@

bindir		= ${tnt_bin_dir}
configdir	= ${tnt_conf_dir}
logdir		= ${tnt_log_dir}
rundir		= @includedir@
bin_PROGRAMS	= help info logcall
bin_DATA	= .help .info .logcall
run_HEADERS	= tntrun.h
help_SOURCES	= help.c tntrun.h
info_SOURCES	= info.c tntrun.h
logcall_SOURCES	= logcall.c tntrun.h

INCLUDES	= -I../../intl
EXTRA_DIST	= READMEBIN tntrun.h.in $(bin_DATA)

