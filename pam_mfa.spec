Summary:   MFA PAM Filter
Name:      pam_mfa
Version:   1.0.2
Release:   1
License:   BSD (LBNL-modified)
Group:     System Environment/Base
URL:       https://github.com/NERSC/shifter
Packager:  Shane Canon <scanon@lbl.gov>
Source0:   %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
PAM MFA module used to filter which users use the OTP option

%prep
%setup -q

%build
MAKEFLAGS=%{?_smp_mflags} %{__make}


%install
%make_install

#%check
#%{__make} check
#
%files
%defattr(-, root, root)
/lib64/security/pam_mfa.so
# %doc AUTHORS Dockerfile LICENSE NEWS README* doc extra/cle6

%changelog
* Sun Oct  2 2017 Shane Canon <scanon@lbl.gov> - 1.0.2
- Add nomfa

* Sun Sep 24 2017 Shane Canon <scanon@lbl.gov> - 1.0.1
- Initial RPM
