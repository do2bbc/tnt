EXTRA_DIST = de.po fr.po POTFILES.in tnt.pot
localedir = $(datadir)/locale

if ENABLE_NLS
catalogs = de.mo fr.mo
all-local: $(catalogs)

%.mo: $(srcdir)/%.po
	$(MSGFMT) --check --output-file=$@ $<

install-data-local: $(catalogs)
	@for lang in de fr; do \
	  dir="$(DESTDIR)$(localedir)/$$lang/LC_MESSAGES"; \
	  $(MKDIR_P) "$$dir" || exit 1; \
	  $(INSTALL_DATA) "$$lang.mo" "$$dir/$(PACKAGE).mo" || exit 1; \
	done

uninstall-local:
	@for lang in de fr; do \
	  rm -f "$(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(PACKAGE).mo"; \
	done
endif

CLEANFILES = de.mo fr.mo
