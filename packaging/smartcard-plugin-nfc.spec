Name:             smartcard-plugin-nfc
Summary:          Smartcard plugin nfc
Version:          0.0.3
Release:          0
Group:            libs
License:          Apache License, Version 2.0
Source0:          %{name}-%{version}.tar.gz
Source1001: 	smartcard-plugin-nfc.manifest
BuildRequires:    pkgconfig(glib-2.0)
BuildRequires:    pkgconfig(dlog)
BuildRequires:    pkgconfig(nfc)
BuildRequires:    pkgconfig(smartcard-service-common)
BuildRequires:    cmake
BuildRequires:    gettext-tools
Requires(post):   /sbin/ldconfig
Requires(post):   /usr/bin/vconftool
requires(postun): /sbin/ldconfig
%description
Smartcard Service plugin nfc

%prep
%setup -q
cp %{SOURCE1001} .


%package    devel
Summary:    smartcard service
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
smartcard service.


%build
mkdir obj-arm-limux-qnueabi
cd obj-arm-limux-qnueabi
cmake .. -DCMAKE_INSTALL_PREFIX=%{_prefix}
#make %{?jobs:-j%jobs}


%install
cd obj-arm-limux-qnueabi
%make_install
mkdir -p %{buildroot}/usr/share/license
cp -af %{_builddir}/%{name}-%{version}/packaging/smartcard-plugin-nfc %{buildroot}/usr/share/license/

%post
/sbin/ldconfig


%postun
/sbin/ldconfig

#%post
# -n nfc-common-lib -p /sbin/ldconfig

#%postun
# -n nfc-common-lib -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
/usr/lib/se/lib*.so
/usr/share/license/smartcard-plugin-nfc
