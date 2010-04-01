Summary: spmfilter - mail filtering framework
Name: spmfilter
Version: 0.4.0
Release: el5.1
License: LGPL
Group: Development/Libraries
Vendor: spmfilter.org
URL: http://www.spmfilter.org
Requires: libzdb >= 2.6
Requires: glib2 >= 2.12
Requires: openldap >= 2.3
Requires: libesmtp >= 1.0.4
Requires: gmime >= 2.4
Requires: pcre >= 6.6
BuildRequires: cmake >= 2.6
BuildRequires: libzdb-devel >= 2.6
BuildRequires: glib2-devel >= 2.12
BuildRequires: openldap-devel >= 2.3
BuildRequires: libesmtp-devel >= 1.0.4
BuildRequires: pcre-devel >= 6.6
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root

Source: %{name}-%{version}.tar.gz

%description
spmfilter is a high-performance mail filtering framework, written in C. 
It attempts to be a general filtering framework for any purposes. Filtering 
mechanisms are provided by plugins, the API enables spmfilter plugins to 
access messages as they are being processed by the MTA. This allows them 
to examine and modify message content and meta-information during the SMTP 
transaction. Plugins are loaded at runtime and can be processed in any 
sequence, the processing chain can also be altered by a single plugin, for 
example if the plugin has to stop further processing (e.g. the clamav-plugin 
detected malicious software like a virus and the infected message is 
actually discarded - so further processing is stopped by the plugin).

%prep

%setup -q -n %{name}-%{version}

%build
cmake -DPREFIX=/usr -DENABLE_DEBUG:BOOL=TRUE .
make 
  
%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/ld.so.conf.d
echo "%{_libdir}/spmfilter" > $RPM_BUILD_ROOT%{_sysconfdir}/ld.so.conf.d/spmfilter.conf
install spmfilter.conf.sample $RPM_BUILD_ROOT%{_sysconfdir}/spmfilter.conf
mkdir -p $RPM_BUILD_ROOT%{_var}/spool/spmfilter

%post
ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_sysconfdir}/ld.so.conf.d/spmfilter.conf
%config(noreplace) %{_sysconfdir}/spmfilter.conf
%{_libdir}/%{name}
%{_mandir}/man*/*
%{_includedir}/spmfilter/*
%{_libdir}/pkgconfig/spmfilter.pc
%attr(0755,root,root) %{_bindir}/spmfilter
%attr(0755,mailnull,mailnull) %{_var}/spool/spmfilter

%changelog
* Wed Mar 31 2010 Axel Steiner <ast@treibsand.com>
- initial Version
