EXTRA_DIST	= isc.cfg linux.cfg netbsd.cfg solaris.cfg \
		  tnt_setup.de.in tnt_setup.en.in tnt_setup.fr.in

setupdir 	= @sbindir@
setup_SCRIPTS	= tnt_setup.de tnt_setup.en tnt_setup.fr

tnt_work_dir    = @tnt_work_dir@
tnt_bin_dir     = @tnt_bin_dir@

all-local:
	chmod +x $(setup_SCRIPTS)

install-data-hook:
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/7plus"         |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/abin"          |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/bcast"         |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/bcast/newmail" |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/bcast/save"    |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/ctext"         |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/down"          |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/macro"         |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/newmail"       |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/remote"        |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/tntusers"      |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/up"            |true
	$(MKDIR_P) "$(DESTDIR)${tnt_work_dir}/yapp"          |true
	@if [ ! -d "$(DESTDIR)${tnt_work_dir}/bin" ]; then \
		ln -s "${tnt_bin_dir}" "$(DESTDIR)${tnt_work_dir}/bin" |true; \
	fi
	@echo "*****************************"
	@echo "*       TNT installed       *"
	@echo "*                           *"
	@echo "* Maybe you want to run     *"
	@echo -n "* tnt_setup."
	@echo -n @language@
	@echo " to configure *"
	@echo "* tnt.ini                   *"
	@echo "*****************************"

