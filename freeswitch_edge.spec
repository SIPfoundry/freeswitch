%define version %{version_no}
%define release %{build_no}
%define prefix %{install_prefix} 

######################################################################################################################

Name:         	freeswitch_edge
Summary:      	FreeSWITCH open source telephony platform
License:      	MPL1.1
Group:        	Productivity/Telephony/Servers
Version:	%{version}
Release:	%{release}
URL:          	http://www.freeswitch.org/
Packager:     	Ken Rice
Vendor:       	http://www.freeswitch.org/

######################################################################################################################
#
#					Source files and where to get them
#
######################################################################################################################
Source0:  http://files.freeswitch.org/%{name}-%{version}.tar.bz2
Prefix: %{prefix}


######################################################################################################################
#
#				Build Dependencies
#
######################################################################################################################

BuildRequires: autoconf
BuildRequires: automake
BuildRequires: curl-devel
BuildRequires: gcc-c++
BuildRequires: gnutls-devel
BuildRequires: libtool >= 1.5.17
BuildRequires: ncurses-devel
BuildRequires: openssl-devel
BuildRequires: perl
%if 0%{?fedora_version} >= 8 || 0%{?rhel} >= 6
BuildRequires: perl-ExtUtils-Embed
%endif
BuildRequires: pkgconfig
%if 0%{?rhel} < 6 && 0%{?fedora} <= 6
BuildRequires: termcap
%endif
BuildRequires: unixODBC-devel
BuildRequires: gdbm-devel
BuildRequires: db4-devel
BuildRequires: python-devel
BuildRequires: libogg-devel
BuildRequires: libvorbis-devel
BuildRequires: libjpeg-devel
BuildRequires: alsa-lib-devel
BuildRequires: which
BuildRequires: zlib-devel
BuildRequires: e2fsprogs-devel
BuildRequires: libtheora-devel
BuildRequires: libxml2-devel
BuildRequires: bison
Requires: alsa-lib
Requires: libogg
Requires: libvorbis
Requires: curl
Requires: ncurses
Requires: openssl
Requires: unixODBC
Requires: libjpeg
Requires: db4
Requires: gdbm
Requires: zlib
Requires: libtiff
Requires: python
Requires: libtheora
Requires: libxml2

%if 0%{?suse_version} > 800
PreReq:       %insserv_prereq %fillup_prereq
%endif


######################################################################################################################
#
#					Where the packages are going to be built
#
######################################################################################################################
BuildRoot:    %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
FreeSWITCH is an open source telephony platform designed to facilitate the creation of voice 
and chat driven products scaling from a soft-phone up to a soft-switch.  It can be used as a 
simple switching engine, a media gateway or a media server to host IVR applications using 
simple scripts or XML to control the callflow. 

We support various communication technologies such as SIP, H.323 and GoogleTalk making 
it easy to interface with other open source PBX systems such as sipX, OpenPBX, Bayonne, YATE or Asterisk.

We also support both wide and narrow band codecs making it an ideal solution to bridge legacy 
devices to the future. The voice channels and the conference bridge module all can operate 
at 8, 16 or 32 kilohertz and can bridge channels of different rates.

FreeSWITCH runs on several operating systems including Windows, Max OS X, Linux, BSD and Solaris 
on both 32 and 64 bit platforms.

Our developers are heavily involved in open source and have donated code and other resources to 
other telephony projects including sipXecs, OpenSER, Asterisk, CodeWeaver and OpenPBX.


######################################################################################################################
#
#		    Sub Package definitions. Description and Runtime Requirements go here
#		What goes into which package is in the files section after the whole build enchilada
#
######################################################################################################################


%prep
%setup -q

%build
if test ! -f Makefile.in 
then 
   ./bootstrap.sh
fi
%configure \
  --prefix=%{prefix} \
  --exec-prefix=%{prefix}/bin \
  --bindir=%{prefix}/bin \
  --sbindir=%{prefix}/sbin \
  --libexecdir=%{prefix}/bin \
  --libdir=%{prefix}/lib \
  --includedir=%{prefix}/include

make %{?_smp_mflags}


%install
make DESTDIR=%{buildroot} install
%{__rm} -rf %{buildroot}/etc
%{__rm} -rf %{buildroot}/%{prefix}/include
%{__rm} -rf %{buildroot}/%{prefix}/conf  
%{__rm} -rf %{buildroot}/%{prefix}/db  
%{__rm} -rf %{buildroot}/%{prefix}/grammar	
%{__rm} -rf %{buildroot}/%{prefix}/htdocs	
%{__rm} -rf %{buildroot}/%{prefix}/include  
%{__rm} -rf %{buildroot}/%{prefix}/log  
%{__rm} -rf %{buildroot}/%{prefix}/recordings  
%{__rm} -rf %{buildroot}/%{prefix}/run  
%{__rm} -rf %{buildroot}/%{prefix}/scripts
%{__rm} -rf %{buildroot}/%{prefix}/lib/pkgconfig


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
%{__rm} -rf %{buildroot}

%files
%dir %attr(0750,-,-) %{prefix}/bin
%dir %attr(0750,-,-) %{prefix}/lib
%dir %attr(0750,-,-) %{prefix}/mod
%attr(0755,-,-) %{prefix}/bin/*
%{prefix}/lib/*
%{prefix}/mod/*


