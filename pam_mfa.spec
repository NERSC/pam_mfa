Summary:   MFA PAM Filter
Name:      pam_mfa
Version:   1.0.0
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
* Sun Apr 24 2016 Douglas Jacobsen <dmjacobsen@lbl.gov> - 16.04.0pre1-1
- Initial RPM
