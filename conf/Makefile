# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4-p5 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = /var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/tnt
pkglibdir = $(libdir)/tnt
pkgincludedir = $(includedir)/tnt

top_builddir = ..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = x86_64-pc-linux-gnuoldld
build_triplet = x86_64-pc-linux-gnuoldld
host_alias = x86_64-pc-linux-gnuoldld
host_triplet = x86_64-pc-linux-gnuoldld
target_alias = x86_64-pc-linux-gnuoldld
target_triplet = x86_64-pc-linux-gnuoldld
BUILD_INCLUDED_LIBINTL = no
CATOBJEXT = .gmo
CC = gcc
DATADIRNAME = share
GENCAT = gencat
GLIBC21 = yes
GMSGFMT = /usr/bin/msgfmt
HAVE_LIB = @HAVE_LIB@
INSTOBJEXT = .mo
INTLBISON = bison
INTLLIBS = 
INTLOBJS = 
INTL_LIBTOOL_SUFFIX_PREFIX = 
LIB = @LIB@
LIBICONV = 
LIBINTL = 
LTLIB = @LTLIB@
LTLIBICONV = 
LTLIBINTL = 
MAKEINFO = makeinfo
MKINSTALLDIRS = ./mkinstalldirs
PACKAGE = tnt
POSUB = po
RANLIB = ranlib
USE_INCLUDED_LIBINTL = no
USE_NLS = yes
VERSION = 1.9.2
cwax25 = 1
cwsound = 1
cwsoundini = tnt_sound_dir
gennewuser = 0
language = en
tnt_conf_dir = /usr/local/share/tnt/conf
tnt_doc_dir = /usr/local/share/tnt/doc
tnt_editor = /usr/bin/editor
tnt_log_dir = /var/log
tnt_main_dir = /usr/local
tnt_proc_dir = /var/run
tnt_sound_dir = /usr/local/share/tnt/sounds

EXTRA_DIST = isc.cfg linux.cfg netbsd.cfg solaris.cfg \
		  tnt_setup.de.in tnt_setup.en.in tnt_setup.fr.in


setupdir = ${exec_prefix}/sbin
setup_SCRIPTS = tnt_setup.de tnt_setup.en tnt_setup.fr

tnt_work_dir = /var/spool/tnt
tnt_bin_dir = /usr/local/libexec/tnt
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES =  tnt_setup.de tnt_setup.en tnt_setup.fr
SCRIPTS =  $(setup_SCRIPTS)

DIST_COMMON =  Makefile.am Makefile.in tnt_setup.de.in tnt_setup.en.in \
tnt_setup.fr.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps conf/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status

tnt_setup.de: $(top_builddir)/config.status tnt_setup.de.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status
tnt_setup.en: $(top_builddir)/config.status tnt_setup.en.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status
tnt_setup.fr: $(top_builddir)/config.status tnt_setup.fr.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status

install-setupSCRIPTS: $(setup_SCRIPTS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(setupdir)
	@list='$(setup_SCRIPTS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(INSTALL_SCRIPT) $$p $(DESTDIR)$(setupdir)/`echo $$p|sed '$(transform)'`"; \
	    $(INSTALL_SCRIPT) $$p $(DESTDIR)$(setupdir)/`echo $$p|sed '$(transform)'`; \
	  else if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_SCRIPT) $(srcdir)/$$p $(DESTDIR)$(setupdir)/`echo $$p|sed '$(transform)'`"; \
	    $(INSTALL_SCRIPT) $(srcdir)/$$p $(DESTDIR)$(setupdir)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; fi; \
	done

uninstall-setupSCRIPTS:
	@$(NORMAL_UNINSTALL)
	list='$(setup_SCRIPTS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(setupdir)/`echo $$p|sed '$(transform)'`; \
	done
tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = conf

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am:
install-exec: install-exec-am

install-data-am: install-setupSCRIPTS
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-setupSCRIPTS
uninstall: uninstall-am
all-am: Makefile $(SCRIPTS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(setupdir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: uninstall-setupSCRIPTS install-setupSCRIPTS tags distdir info-am \
info dvi-am dvi check check-am installcheck-am installcheck \
install-exec-am install-exec install-data-am install-data install-am \
install uninstall-am uninstall all-redirect all-am all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


install:
	mkdir -p ${tnt_work_dir}/7plus         |true
	mkdir -p ${tnt_work_dir}/abin          |true
	mkdir -p ${tnt_work_dir}/bcast         |true
	mkdir -p ${tnt_work_dir}/bcast/newmail |true
	mkdir -p ${tnt_work_dir}/bcast/save    |true
	mkdir -p ${tnt_work_dir}/ctext         |true
	mkdir -p ${tnt_work_dir}/down          |true
	mkdir -p ${tnt_work_dir}/macro         |true
	mkdir -p ${tnt_work_dir}/newmail       |true
	mkdir -p ${tnt_work_dir}/remote        |true
	mkdir -p ${tnt_work_dir}/tntusers      |true
	mkdir -p ${tnt_work_dir}/up            |true
	mkdir -p ${tnt_work_dir}/yapp          |true
	@if [ ! -d ${tnt_work_dir}/bin ]; then \
		ln -s ${tnt_bin_dir} ${tnt_work_dir}/bin |true; \
	fi
	@echo "*****************************"
	@echo "*       TNT installed       *"
	@echo "*                           *"
	@echo "* Maybe you want to run     *"
	@echo -n "* tnt_setup."
	@echo -n en
	@echo " to configure *"
	@echo "* tnt.ini                   *"
	@echo "*****************************"

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
