Name:             smartcard-plugin-nfc
Summary:          Smartcard plugin nfc
Version:          0.0.10
Release:          0
Group:            Network & Connectivity/NFC
License:          Apache-2.0
Source0:          %{name}-%{version}.tar.gz
Source1001: 	smartcard-plugin-nfc.manifest
#ExclusiveArch:    %%arm
BuildRequires:    cmake
BuildRequires:    pkgconfig(glib-2.0)
BuildRequires:    pkgconfig(dlog)
BuildRequires:    pkgconfig(smartcard-service-common)
BuildRequires:    pkgconfig(capi-network-nfc)
Requires(post):   /sbin/ldconfig
Requires(postun): /sbin/ldconfig


%description
Smartcard Service plugin nfc

%prep
%setup -q
cp %{SOURCE1001} .

%build
%cmake .

%install
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/se/lib*.so
%license LICENSE.APLv2
