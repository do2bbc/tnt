Name:           tnt
Version:        2.0.0~rc3
Release:        1
Summary:        Packet-radio hostmode terminal
License:        GPL-2.0-or-later
URL:            https://github.com/do2bbc/tnt
Source0:        tnt-2.0.0-rc3.tar.gz
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  cmake >= 3.20
BuildRequires:  ninja
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
%setup -q -n tnt-2.0.0-rc3

%build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_INSTALL_BINDIR=%{_bindir} \
  -DCMAKE_INSTALL_SBINDIR=%{_sbindir} -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir} \
  -DCMAKE_INSTALL_SYSCONFDIR=%{_sysconfdir} -DCMAKE_INSTALL_INFODIR=%{_infodir} \
  -DTNT_LOCALSTATEDIR=%{_localstatedir} \
  -DCMAKE_C_FLAGS="%{optflags}"
cmake --build build --parallel 2

%check
ctest --test-dir build --output-on-failure

%install
DESTDIR=%{buildroot} cmake --install build
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
