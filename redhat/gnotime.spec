%define name gnotime
%define ver 2.1.7
%define rel 1
%define prefix /usr

Summary: A time manager
Name: gnotime
Version: %ver
Release: %rel
Copyright: GPL
Group: Applications/Productivity
Source: gnotime-%{PACKAGE_VERSION}.tar.gz
BuildRoot: /tmp/gnotime-%{PACKAGE_VERSION}-root
Packager: Eric Anderson <eric.anderson@cordata.net>

%description
A combination of stop-watch, diary, consultant billing system, and project
manager.

%prep
%setup

%build
./configure --prefix=/usr
make

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{prefix} mandir=$RPM_BUILD_ROOT%{_mandir} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc AUTHORS ChangeLog COPYING INSTALL NEWS README TODO
%{prefix}/share/*/*
%{prefix}/bin/*
%{_mandir}/*/*
