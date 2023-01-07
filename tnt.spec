Name: tnt
Version: 1.9.1
Release: 1
Copyright: GNU GPL
Group: Applications/Communications
Source: ftp://ftp.wspse.de/pub/packet_radio/tnt/%{name}-%{version}.tar.gz
Buildroot: /var/tmp/tnt-root
Summary: terminalprogram for packet radio with hostmode tncs
Summary(de): Terminalprogramm für Packet Radio
Vendor: Matthias Hensler <wsp@gmx.de>


%description
TNT is a console based packet-radio terminal for hostmode tncs.
It supports virtual channels and socketcommunication. It also can used
with ax25 kernel or kiss interfaces together with TFkiss.

%description -l de
TNT is ein konsolen basierter Packet-Radio Terminal für Hostmode TNCs.
Es unterstützt virtuelle Kanäle und Socket Kommunikation. Es kann
außerdem mit einem AX25 Kernel oder Kiss Interfaces im Zusammenspiel
mit TFkiss benutzt werden.

%prep
%setup
%build
aclocal
autoconf
autoheader
automake
./configure --prefix=/usr --enable-ax25k --enable-hibaud --with-language=en --enable-sound --localstatedir=/var
make

%install
rm -rf $RPM_BUILD_ROOT
make install prefix=$RPM_BUILD_ROOT/usr localstatedir=$RPM_BUILD_ROOT/var tnt_main_dir=$RPM_BUILD_ROOT/usr tnt_work_dir=$RPM_BUILD_ROOT/var/spool/tnt tnt_conf_dir=$RPM_BUILD_ROOT/usr/share/tnt/conf tnt_doc_dir=$RPM_BUILD_ROOT/usr/share/tnt/doc tnt_log_dir=$RPM_BUILD_ROOT/var/log tnt_bin_dir=$RPM_BUILD_ROOT/usr/libexec/tnt tnt_sound_dir=$RPM_BUILD_ROOT/usr/share/tnt/sounds
make prefix=$RPM_BUILD_ROOT/usr localstatedir=$RPM_BUILD_ROOT/var tnt_main_dir=$RPM_BUILD_ROOT/usr tnt_work_dir=$RPM_BUILD_ROOT/var/spool/tnt tnt_conf_dir=$RPM_BUILD_ROOT/usr/share/tnt/conf tnt_doc_dir=$RPM_BUILD_ROOT/usr/share/tnt/doc tnt_log_dir=$RPM_BUILD_ROOT/var/log tnt_bin_dir=$RPM_BUILD_ROOT/usr/libexec/tnt tnt_sound_dir=$RPM_BUILD_ROOT/usr/share/tnt/sounds
strip $RPM_BUILD_ROOT/usr/bin/tnt
strip $RPM_BUILD_ROOT/usr/bin/tntc

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS AX25-NOTES ChangeLog FAQ NEWS README TODO doc/ax25krnl.doc doc/iface.doc doc/tnt.doc.de doc/tnt.doc.en doc/tnt.doc.fr doc/tntdoc.dvi doc/tntdoc.html doc/tntdoc.texi
/usr/bin/tnt
/usr/bin/tntc
/usr/include/tntrun.h
/usr/info/tntdoc.info
/usr/info/tntdoc.info-1
/usr/info/tntdoc.info-2
/usr/info/tntdoc.info-3
/usr/info/tntdoc.info-4
/usr/libexec/tnt/.help
/usr/libexec/tnt/.info
/usr/libexec/tnt/.logcall
/usr/libexec/tnt/help
/usr/libexec/tnt/info
/usr/libexec/tnt/logcall
/usr/sbin/tnt_setup.de
/usr/sbin/tnt_setup.en
/usr/sbin/tnt_setup.fr
/usr/share/tnt/conf/autostrt.tnt.ex
/usr/share/tnt/conf/boxender.tnt.ex
/usr/share/tnt/conf/ctext.tnt.ex
/usr/share/tnt/conf/ctext.tnt.fr.ex
/usr/share/tnt/conf/extrem.tnt.ex
/usr/share/tnt/conf/f6fbb.box.ex
/usr/share/tnt/conf/fkeys.tnt.ex
/usr/share/tnt/conf/names.tnt.ex
/usr/share/tnt/conf/netpass.tnt.ex
/usr/share/tnt/conf/news.tnt.ex
/usr/share/tnt/conf/norem.tnt.ex
/usr/share/tnt/conf/notown.tnt.ex
/usr/share/tnt/conf/pw.tnt.ex
/usr/share/tnt/conf/qtext.tnt.ex
/usr/share/tnt/conf/qtext.tnt.fr.ex
/usr/share/tnt/conf/routes.tnt.ex
/usr/share/tnt/conf/sounds.tnt.ex
/usr/share/tnt/conf/sys.tnt.ex
/usr/share/tnt/conf/telltexte.tnt.ex
/usr/share/tnt/conf/termcap.tnt.ex
/usr/share/tnt/conf/tnt.dwn.ex
/usr/share/tnt/conf/tnt.ini.ex
/usr/share/tnt/conf/tnt.up.ex
/usr/share/tnt/conf/tntc.ini.ex
/usr/share/tnt/conf/tntrem.inf.ex
/usr/share/tnt/doc/Readme1.de
/usr/share/tnt/doc/Readme2.de
/usr/share/tnt/doc/Readme3.de
/usr/share/tnt/doc/Readme4.de
/usr/share/tnt/doc/Readme5.de
/usr/share/tnt/doc/Readme6.de
/usr/share/tnt/doc/Readme1.en
/usr/share/tnt/doc/Readme2.en
/usr/share/tnt/doc/Readme3.en
/usr/share/tnt/doc/Readme4.en
/usr/share/tnt/doc/Readme5.en
/usr/share/tnt/doc/Readme6.en
/usr/share/tnt/doc/Readme1.fr
/usr/share/tnt/doc/Readme2.fr
/usr/share/tnt/doc/Readme3.fr
/usr/share/tnt/doc/Readme4.fr
/usr/share/tnt/doc/Readme5.fr
/usr/share/tnt/doc/Readme6.fr
/usr/share/tnt/doc/tnt.hlp.de
/usr/share/tnt/doc/tnt.hlp.en
/usr/share/tnt/doc/tnt.hlp.fr
/usr/share/tnt/doc/tntrem.hlp.de
/usr/share/tnt/doc/tntrem.hlp.en
/usr/share/tnt/doc/tntrem.hlp.fr
/usr/share/tnt/sounds/busy.au
/usr/share/tnt/sounds/connect.au
/usr/share/tnt/sounds/disc.au
/usr/share/tnt/sounds/failure.au
/usr/share/tnt/sounds/play
/usr/share/tnt/sounds/reset.au
/usr/share/tnt/sounds/ring.au
/var/spool/tnt/7plus
/var/spool/tnt/abin
/var/spool/tnt/bcast/save
/var/spool/tnt/bcast/newmail
/var/spool/tnt/ctext
/var/spool/tnt/down
/var/spool/tnt/macro
/var/spool/tnt/newmail
/var/spool/tnt/remote
/var/spool/tnt/tntusers
/var/spool/tnt/up
/var/spool/tnt/yapp
/usr/share/locale/de/LC_MESSAGES/tnt.mo
/usr/share/locale/fr/LC_MESSAGES/tnt.mo

