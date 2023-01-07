SUBDIRS		= sounds

tnt_conf_dir	= @tnt_conf_dir@
tnt_doc_dir	= @tnt_doc_dir@

exmpdir		= ${tnt_conf_dir}
exmpfiles	= autostrt.tnt \
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

helpdir		= ${tnt_doc_dir}
help_DATA	= tnt.hlp.de \
		  tnt.hlp.en \
		  tnt.hlp.fr \
		  tntrem.hlp.de \
		  tntrem.hlp.en \
		  tntrem.hlp.fr

EXTRA_DIST	= sounds.tnt.in tnt.ini.in tntc.ini.in $(exmpfiles) $(help_DATA)

install:
	$(mkinstalldirs) $(DESTDIR)$(exmpdir)
	@for i in $(exmpfiles); \
  	do \
		$(INSTALL_DATA) $(srcdir)/$$i $(DESTDIR)$(exmpdir)/$$i.ex; \
	done;

