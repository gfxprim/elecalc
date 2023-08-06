#
# ELECALC specfile
#
# (C) Cyril Hrubis metan{at}ucw.cz 2023
#

Summary: An electrical calculator
Name: elecalc
Version: git
Release: 1
License: GPL-2.0-or-later
Group: Productivity/Scientific/Electronics
Url: https://github.com/gfxprim/elecalc
Source: elecalc-%{version}.tar.bz2
BuildRequires: libgfxprim-devel

BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot

%description
An electrical calculator

%prep
%setup -n elecalc-%{version}

%build
make %{?jobs:-j%jobs}

%install
DESTDIR="$RPM_BUILD_ROOT" make install

%files -n elecalc
%defattr(-,root,root)
%{_bindir}/elecalc
%{_sysconfdir}/gp_apps/
%{_sysconfdir}/gp_apps/elecalc/
%{_sysconfdir}/gp_apps/elecalc/*
%{_datadir}/applications/elecalc.desktop
%{_datadir}/elecalc/
%{_datadir}/elecalc/elecalc.png

%changelog
* Sun Aug 20 2023 Cyril Hrubis <metan@ucw.cz>

  Initial version.
