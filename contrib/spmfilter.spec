Summary: spmfilter - mail filtering framework
Name: spmfilter
Version: 0.4.0
Release: 1%{?dist}
License: LGPL
Group: Development/Libraries
Vendor: spmfilter.org
URL: http://www.spmfilter.org

Requires: glib2
Requires: libesmtp
Requires: pcre
Requires: openldap
Requires: gmime >= 2.2
Requires: libzdb

BuildRequires: glib2-devel
BuildRequires: libesmtp-devel
BuildRequires: pcre-devel
BuildRequires: libstdc++-devel
BuildRequires: gcc-c++
BuildRequires: cmake
BuildRequires: openssl-devel
BuildRequires: openldap-devel
BuildRequires: gmime-devel >= 2.2
BuildRequires: libzdb-devel


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

%package        devel
Summary:        Header files to develop spmfilter plugins
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}
Requires:       glib2-devel

%description    devel
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
%ifarch x86_64
cmake -DPREFIX=/usr -DLIBDIR:STRING=lib64 .
%else
cmake -DPREFIX=/usr .
%endif
make 
  
%install
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

%attr(0755,root,root) %{_sbindir}/spmfilter
%attr(0755,mail,mail) %{_var}/spool/spmfilter

%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/*
%{_libdir}/pkgconfig/spmfilter.pc
%{_libdir}/%{name}

%changelog
* Sun Apr 04 2010 Axel Steiner <ast@treibsand.com>
- initial Version

