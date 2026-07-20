tnt_bin_dir	= @tnt_bin_dir@
tnt_conf_dir	= @tnt_conf_dir@
tnt_log_dir	= @tnt_log_dir@

tntbindir	= ${tnt_bin_dir}
configdir	= ${tnt_conf_dir}
logdir		= ${tnt_log_dir}
rundir		= @includedir@
tntbin_PROGRAMS	= help logcall tnt-info
tntbin_DATA	= .help .info .logcall
run_HEADERS	= tntrun.h
help_SOURCES	= help.c tntrun.h
tnt_info_SOURCES	= info.c tntrun.h
logcall_SOURCES	= logcall.c tntrun.h

AM_CPPFLAGS	= -I../../intl
EXTRA_DIST	= READMEBIN tntrun.h.in $(tntbin_DATA)

all-local:
	ln -sf tnt-info info

clean-local:
	rm -f info

install-exec-hook:
	$(MKDIR_P) "$(DESTDIR)$(tntbindir)"
	ln -sf tnt-info $(DESTDIR)$(tntbindir)/info

uninstall-hook:
	rm -f $(DESTDIR)$(tntbindir)/info

