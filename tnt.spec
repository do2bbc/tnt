Name:           tnt
Version:        2.0.0~rc2
Release:        1
Summary:        Packet-radio hostmode terminal
License:        GPL-2.0-or-later
URL:            https://github.com/do2bbc/tnt
Source0:        tnt-2.0.0-rc2.tar.gz
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  gettext-tools
BuildRequires:  ncurses-devel
Recommends:     dialog
ExclusiveArch:  x86_64

%description
TNT release candidate for Linux packet-radio hostmode terminals.
UTF-8 terminal boundaries preserve the legacy radio encoding.
No service or active node configuration is installed or started.
This RPM targets openSUSE Leap 16 x86_64.

%prep
%setup -q -n tnt-2.0.0-rc2

%build
autoreconf -fi
%configure
%make_build

%check
make check

%install
make DESTDIR=%{buildroot} install
mkdir -p %{buildroot}%{_docdir}/tnt
cp BUILD.md NEWS packaging/README.packages BUILD.txt %{buildroot}%{_docdir}/tnt/
mkdir -p %{buildroot}%{_licensedir}/tnt
cp COPYING %{buildroot}%{_licensedir}/tnt/

%files
%{_bindir}/tnt
%{_bindir}/tntc
%{_sbindir}/tnt_setup.*
%{_includedir}/tntrun.h
%{_libexecdir}/tnt
%{_datadir}/tnt
%{_datadir}/locale/de/LC_MESSAGES/tnt.mo
%{_datadir}/locale/fr/LC_MESSAGES/tnt.mo
%{_infodir}/tntdoc.info*
%dir %{_sysconfdir}/tnt
%config(noreplace) %{_sysconfdir}/tnt/*.ex
%{_localstatedir}/spool/tnt
%{_docdir}/tnt
%{_licensedir}/tnt
