EXTRA_DIST	= isc.cfg linux.cfg netbsd.cfg solaris.cfg \
		  tnt_setup.de.in tnt_setup.en.in tnt_setup.fr.in

setupdir 	= @sbindir@
setup_SCRIPTS	= tnt_setup.de tnt_setup.en tnt_setup.fr

tnt_work_dir    = @tnt_work_dir@
tnt_bin_dir     = @tnt_bin_dir@

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
	@echo -n @language@
	@echo " to configure *"
	@echo "* tnt.ini                   *"
	@echo "*****************************"

