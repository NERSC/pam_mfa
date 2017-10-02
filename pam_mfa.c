#define  PAM_SM_SESSION
#define _GNU_SOURCE

#include <errno.h>
#include <pwd.h>
#include <security/pam_modules.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <unistd.h>
#define LDAP_DEPRECATED 1
#include <ldap.h>

#define DEBUG 0
#define ATTR "authorizedService"
#define BUFFSIZE 15L*3000L
#define MFALIST_FILE "/etc/mfausers"
#define NOMFA_FILE "/etc/nomfa"
#define MFA_USE_FILE 0
#define MFA_USE_LDAP 1

typedef struct {
	int mode;
	char *mfafile;
	char *server;
	char *base;
	char *aval;
} MFAConfig ;

int get_config(int argc, const char *argv[], MFAConfig *config) {

    int ret = PAM_SUCCESS;
		int i;

    /* reset configuration */
		config->mode=MFA_USE_FILE;
		config->mfafile=MFALIST_FILE;
		config->server=NULL;
		config->base=NULL;
		config->aval=NULL;
		for ( i = 0; i < argc; i++ ) {
			char *temp;
			if (strncasecmp(argv[i], "ldap_server=",12) ==0){
				 config->server=strdup(argv[i]+12);
			}
			else if (strncasecmp(argv[i], "ldap_base=",10) ==0){
				 config->base=strdup(argv[i]+10);
			}
			else if (strncasecmp(argv[i], "ldap_attr=",10) ==0){
				 config->aval=strdup(argv[i]+10);
			}
			else if (strncasecmp(argv[i], "file=", 5) ==0){
				 config->mfafile=strdup(argv[i]+5);
			}
		}
		if (config->server!=NULL && config->base!=NULL && config->aval!=NULL){
			config->mode=MFA_USE_LDAP;
		}
		return ret;
}


int check_user_ldap(const char *user, MFAConfig *config){

    LDAP *ld;
    int rc;
    //int reqcert = LDAP_OPT_X_TLS_NEVER;
    int version = LDAP_VERSION3;
    int i;
    char* dn = NULL;
    char* attr;
    char** vals;
    char base[1024];
    char* filter="(objectClass=*)";
    BerElement* ber;
    LDAPMessage* msg;
    LDAPMessage* entry;
    int rv=0;

    putenv("LDAPTLS_REQCERT=never");
    if (ldap_initialize (&ld, config->server)) {
        perror("ldap_init"); /* no error here */
        return(1);
    }
    base[0]=0;
    strcat(base,"uid=");
    strcat(base,user);
    strcat(base,",");
    strcat(base,config->base);

    rc=ldap_set_option (ld, LDAP_OPT_PROTOCOL_VERSION, &version);
    rc = ldap_simple_bind_s(ld, NULL, NULL);

    if( rc != LDAP_SUCCESS ) {
        return 0;
    }
    if (ldap_search_s(ld, base, LDAP_SCOPE_SUBTREE, filter, NULL, 0, &msg) != LDAP_SUCCESS) {
        return 0;
    }

    rv=0;

    entry = ldap_first_entry(ld, msg);

    // Find the right attribute
    for( attr = ldap_first_attribute(ld, entry, &ber);
          attr != NULL;
          attr = ldap_next_attribute(ld, entry, ber)) {
       if (strstr(attr,ATTR)==attr){
         // Look for the value we care about
         if ((vals = ldap_get_values(ld, entry, attr)) != NULL)  {
            for(i = 0; vals[i] != NULL; i++) {
               if (strstr(vals[i],config->aval)==vals[i]){
                 if (DEBUG)printf("%s: %s\n", attr, vals[i]);
                 rv=1;
               }
            }
            ldap_value_free(vals);
         }
       }
       ldap_memfree(attr);
    }

    if (ber != NULL) {
       ber_free(ber,0);
    }

    ldap_unbind(ld);
    return rv;
}

int check_user_file(const char *user, MFAConfig *config)
{
	FILE *MFALIST;
	int nsize;
	char *buffer;
	char *ptr;
        char usern[1024];

	MFALIST=fopen(config->mfafile,"r");
	if (MFALIST==NULL){
		return -1;
	}
	// Let's just allocate a buffer for up to around 3000 users
	buffer = malloc(BUFFSIZE);
	if (buffer==NULL)
		return -1;
  if (strlen(user)>1020)
    return -1;
  strcpy(usern,user);
  strcat(usern,"\n");

	nsize=fread(buffer, 1, BUFFSIZE, MFALIST);
	ptr = strstr(buffer,usern);
	if (ptr==NULL)
		return 0;
	else if (ptr==buffer)  /* Lucky us, it was the first one) */
		return 1;
	else if (ptr>buffer && ptr[-1]=='\n') {
		/* if it is later, make sure we match the whole entry */
		return 1;
	}
	return 0;

}

#ifndef TESTMODE

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags,
			int argc, const char **argv) {
	MFAConfig config;
	struct passwd *pwd;
	const char *user;
	int pam_err;
  int result;

	result = get_config(argc, argv, &config);
	if (result != PAM_SUCCESS) {
			 //log_error("Failed to read the pam config");
			 return result;
	}
        /* Ignore if NOMFA file exist */
	if( access( NOMFA_FILE, F_OK ) == 0 ) {
     		return (PAM_IGNORE);
	}

	/* identify user */
	if ((pam_err = pam_get_user(pamh, &user, NULL)) != PAM_SUCCESS)
		return (pam_err);
	if ((pwd = getpwnam(user)) == NULL)
		return (PAM_USER_UNKNOWN);

	if (config.mode==MFA_USE_FILE){
	  	result = check_user_file(user, &config);
	}
	else {
		result = check_user_ldap(user, &config);

	}
  if (result==1)
     return PAM_SUCCESS;
  else if (result==0)
     return (PAM_IGNORE);
  else
     return (PAM_SYSTEM_ERR);
	return (pam_err);
}

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags,
	int argc, const char *argv[])
{
	MFAConfig config;

	struct passwd *pwd;
	const char *user;
	int pam_err;
  int result;

	result = get_config(argc, argv, &config);
	if (result != PAM_SUCCESS) {
			 //log_error("Failed to read the pam config");
			 return result;
	}

        /* Ignore if NOMFA file exist */
	if( access( NOMFA_FILE, F_OK ) == 0 ) {
     		return (PAM_IGNORE);
	}
	/* identify user */
	if ((pam_err = pam_get_user(pamh, &user, NULL)) != PAM_SUCCESS)
		return (pam_err);

	if ((pwd = getpwnam(user)) == NULL)
		return (PAM_USER_UNKNOWN);

		if (config.mode==MFA_USE_FILE){
	  	result = check_user_file(user, &config);
		}
		else {
			result = check_user_ldap(user, &config);

		}
  if (result==1)
     return PAM_SUCCESS;
  else if (result==0)
     return (PAM_IGNORE);
  else
     return (PAM_SYSTEM_ERR);
	return (pam_err);
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t * pamh, int flags, int argc,
        const char **argv) {
    return PAM_SUCCESS;
}

#endif
