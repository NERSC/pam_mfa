# pam_mfa module

This PAM module is intended to enable different authentication behavior
based on the user.  For example, users who have opted to use multi-factor
authentication could use one authentication chain while others use a standard
LDAP authentication chain.  The module will return a PAM success if the user
is determined to be a member of the list.  Membership can be determined from
a file or an LDAP authorizedService attribute.

The module currently supports the authenticate and account management PAM
facilities.

## Configuration Options

The module supports two modes of determining membership: file-based or an
LDAP attribute.  The default is file based and the module will look for a file
named /etc/mfausers by default.  If all of the LDAP configuration options are
defined, then the LDAP mode is used.

### LDAP Options:
**ldap_server:**  The full URI specification of the LDAP server (e.g.
ldaps://ldap.host.org/)

**ldap_base:** The base search path to use.  This will have "ou=People,uid=<user>""
prepended to it.

**ldap_attr:** The authService attribute that determines membership.  If the user
has this attribute set, then the module will return a PAM success.


### File Options:
**file:** This override the default file location (/etc/mfausers).  It should be the
full path to the file.  The file should contain on user per line.  If the user
appears in the file, then the module will return a success.


## Example Usage
The module is designed to be used similar to the pam_succeed module.  In most
scenarios the site would use the "[success=X default=ignore]" syntax.

For example....

```
auth        required      pam_env.so
auth        [success=2 default=ignore] pam_mfa.so file=/etc/opt-in
auth        required      pam_linotp.so
auth        required      pam_deny.so
auth        sufficient    pam_unix.so try_first_pass nullok
auth        required      pam_deny.so
```
