%{!?tnt_sound:%global tnt_sound ON}
%{!?tnt_version:%global tnt_version 2.0.0}
%{!?tnt_source_version:%global tnt_source_version 2.0.0}
%{!?tnt_release:%global tnt_release 1}
Name:           tnt
Version:        %{tnt_version}
Release:        %{tnt_release}
Summary:        Packet-radio hostmode terminal
License:        GPL-2.0-or-later
URL:            https://github.com/do2bbc/tnt
Source0:        tnt-%{tnt_source_version}.tar.gz
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  cmake >= 3.20
BuildRequires:  ninja
BuildRequires:  gettext-tools
BuildRequires:  ncurses-devel
BuildRequires:  python3
Recommends:     dialog
ExclusiveArch:  x86_64

%description
TNT for Linux packet-radio hostmode terminals.
UTF-8 terminal boundaries preserve the legacy radio encoding.
No service or active node configuration is installed or started.
This RPM targets openSUSE Leap 16 x86_64.

%prep
%setup -q -n tnt-%{tnt_source_version}

%build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_INSTALL_BINDIR=%{_bindir} \
  -DCMAKE_INSTALL_SBINDIR=%{_sbindir} -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir} \
  -DCMAKE_INSTALL_SYSCONFDIR=%{_sysconfdir} -DCMAKE_INSTALL_INFODIR=%{_infodir} \
  -DTNT_LOCALSTATEDIR=%{_localstatedir} -DTNT_SOUND=%{tnt_sound} \
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
