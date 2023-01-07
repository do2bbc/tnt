# Makefile.in generated automatically by automake 1.4-p5 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/bash

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
infodir = ${prefix}/share/info
mandir = ${prefix}/share/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/tnt
pkglibdir = $(libdir)/tnt
pkgincludedir = $(includedir)/tnt

top_builddir = ..

ACLOCAL = ${SHELL} '/home/boris/tnt/missing' aclocal-1.16
AUTOCONF = ${SHELL} '/home/boris/tnt/missing' autoconf
AUTOMAKE = ${SHELL} '/home/boris/tnt/missing' automake-1.16
AUTOHEADER = ${SHELL} '/home/boris/tnt/missing' autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = 
build_triplet = x86_64-pc-linux-gnu
host_alias = 
host_triplet = x86_64-pc-linux-gnu
target_alias = 
target_triplet = x86_64-pc-linux-gnu
BUILD_INCLUDED_LIBINTL = yes
CATOBJEXT = .gmo
CC = gcc
DATADIRNAME = @DATADIRNAME@
GENCAT = @GENCAT@
GLIBC21 = @GLIBC21@
GMSGFMT = /usr/bin/msgfmt
HAVE_LIB = @HAVE_LIB@
INSTOBJEXT = @INSTOBJEXT@
INTLBISON = @INTLBISON@
INTLLIBS = 
INTLOBJS = @INTLOBJS@
INTL_LIBTOOL_SUFFIX_PREFIX = @INTL_LIBTOOL_SUFFIX_PREFIX@
LIB = @LIB@
LIBICONV = @LIBICONV@
LIBINTL = 
LTLIB = @LTLIB@
LTLIBICONV = @LTLIBICONV@
LTLIBINTL = 
MAKEINFO = ${SHELL} '/home/boris/tnt/missing' makeinfo
MKINSTALLDIRS = @MKINSTALLDIRS@
PACKAGE = tnt
POSUB = po
RANLIB = @RANLIB@
USE_INCLUDED_LIBINTL = no
USE_NLS = yes
VERSION = 1.9.2
cwax25 = 0
cwsound = 1
cwsoundini = tnt_sound_dir
gennewuser = 0
language = en
tnt_bin_dir = /usr/local/libexec/tnt
tnt_editor = /usr/bin/editor
tnt_log_dir = /var/log
tnt_main_dir = /usr/local
tnt_proc_dir = /var/run
tnt_sound_dir = /usr/local/share/tnt/sounds
tnt_work_dir = /var/spool/tnt

SUBDIRS = sounds

tnt_conf_dir = /usr/local/share/tnt/conf
tnt_doc_dir = /usr/local/share/tnt/doc

exmpdir = ${tnt_conf_dir}
exmpfiles = autostrt.tnt \
		  boxender.tnt \
		  ctext.tnt \
		  ctext.tnt.fr \
		  extrem.tnt \
		  f6fbb.box \
		  fkeys.tnt \
		  names.tnt \
		  netpass.tnt \
		  news.tnt \
		  norem.tnt \
		  notown.tnt \
		  pw.tnt \
		  qtext.tnt \
		  qtext.tnt.fr \
		  routes.tnt \
		  sounds.tnt \
		  sys.tnt \
		  telltexte.tnt \
		  termcap.tnt \
		  tnt.dwn \
		  tnt.ini \
		  tnt.up \
		  tntc.ini \
		  tntrem.inf


helpdir = ${tnt_doc_dir}
help_DATA = tnt.hlp.de \
		  tnt.hlp.en \
		  tnt.hlp.fr \
		  tntrem.hlp.de \
		  tntrem.hlp.en \
		  tntrem.hlp.fr


EXTRA_DIST = sounds.tnt.in tnt.ini.in tntc.ini.in $(exmpfiles) $(help_DATA)
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES =  sounds.tnt tnt.ini tntc.ini
DATA =  $(help_DATA)

DIST_COMMON =  Makefile.am Makefile.in sounds.tnt.in tnt.ini.in \
tntc.ini.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps examples/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status

sounds.tnt: $(top_builddir)/config.status sounds.tnt.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status
tnt.ini: $(top_builddir)/config.status tnt.ini.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status
tntc.ini: $(top_builddir)/config.status tntc.ini.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status

install-helpDATA: $(help_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(helpdir)
	@list='$(help_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(helpdir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(helpdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(helpdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(helpdir)/$$p; \
	  fi; fi; \
	done

uninstall-helpDATA:
	@$(NORMAL_UNINSTALL)
	list='$(help_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(helpdir)/$$p; \
	done

# This directory's subdirectories are mostly independent; you can cd
# into them and run `make' without going through this Makefile.
# To change the values of `make' variables: instead of editing Makefiles,
# (1) if the variable is set in `config.status', edit `config.status'
#     (which will cause the Makefiles to be regenerated when you run `make');
# (2) otherwise, pass the desired values on the `make' command line.



all-recursive install-data-recursive install-exec-recursive \
installdirs-recursive install-recursive uninstall-recursive  \
check-recursive installcheck-recursive info-recursive dvi-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	rev=''; list='$(SUBDIRS)'; for subdir in $$list; do \
	  rev="$$subdir $$rev"; \
	  test "$$subdir" != "." || dot_seen=yes; \
	done; \
	test "$$dot_seen" = "no" && rev=". $$rev"; \
	target=`echo $@ | sed s/-recursive//`; \
	for subdir in $$rev; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test "$$subdir" = . || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) tags); \
	done

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS: tags-recursive $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
   if test "$$subdir" = .; then :; else \
	    test -f $$subdir/TAGS && tags="$$tags -i $$here/$$subdir/TAGS"; \
   fi; \
	done; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = examples

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
	for subdir in $(SUBDIRS); do \
	  if test "$$subdir" = .; then :; else \
	    test -d $(distdir)/$$subdir \
	    || mkdir $(distdir)/$$subdir \
	    || exit 1; \
	    chmod 777 $(distdir)/$$subdir; \
	    (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) top_distdir=../$(top_distdir) distdir=../$(distdir)/$$subdir distdir) \
	      || exit 1; \
	  fi; \
	done
info-am:
info: info-recursive
dvi-am:
dvi: dvi-recursive
check-am: all-am
check: check-recursive
installcheck-am:
installcheck: installcheck-recursive
install-exec-am:
install-exec: install-exec-recursive

install-data-am: install-helpDATA
install-data: install-data-recursive

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-recursive
uninstall-am: uninstall-helpDATA
uninstall: uninstall-recursive
all-am: Makefile $(DATA)
all-redirect: all-recursive
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs: installdirs-recursive
installdirs-am:
	$(mkinstalldirs)  $(DESTDIR)$(helpdir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-tags mostlyclean-generic

mostlyclean: mostlyclean-recursive

clean-am:  clean-tags clean-generic mostlyclean-am

clean: clean-recursive

distclean-am:  distclean-tags distclean-generic clean-am

distclean: distclean-recursive

maintainer-clean-am:  maintainer-clean-tags maintainer-clean-generic \
		distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-recursive

.PHONY: uninstall-helpDATA install-helpDATA install-data-recursive \
uninstall-data-recursive install-exec-recursive \
uninstall-exec-recursive installdirs-recursive uninstalldirs-recursive \
all-recursive check-recursive installcheck-recursive info-recursive \
dvi-recursive mostlyclean-recursive distclean-recursive clean-recursive \
maintainer-clean-recursive tags tags-recursive mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-am uninstall all-redirect all-am all installdirs-am \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


install:
	$(mkinstalldirs) $(DESTDIR)$(exmpdir)
	@for i in $(exmpfiles); \
  	do \
		$(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(exmpdir)/$$i.ex; \
	done;

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
