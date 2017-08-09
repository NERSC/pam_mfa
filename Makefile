


all: pam_mfa.so

pam_mfa.so: pam_mfa.c
	gcc -fPIC -DPIC -shared -rdynamic -lldap -llber -o pam_mfa.so pam_mfa.c

tester:
	gcc -o tester test.c pam_mfa.c -DTESTMODE

install:
	printenv
	mkdir -p $(DESTDIR)/lib64/security/
	cp pam_mfa.so $(DESTDIR)/lib64/security/
