FROM centos:6
#
# This is used primarily to build the RPM and test the module.

RUN \
    yum -y install rpm-build gcc make pam-devel gcc-c++ openldap-devel wget rsyslog

# Install pamtester
RUN \
       wget http://prdownloads.sourceforge.net/pamtester/pamtester-0.1.2.tar.gz && \
       tar xzf pamtester-0.1.2.tar.gz && \
       cd pamtester-0.1.2 && ./configure && \
       make && make install && \
       cd .. && rm -rf pamtester-0.1.2 && \
       useradd auser -m


ADD . /src/pam_mfa-1.0.0


RUN \
    mkdir -p /root/rpmbuild/SOURCES && \
    cd /src/ && tar czf /root/rpmbuild/SOURCES/pam_mfa-1.0.0.tar.gz pam_mfa-1.0.0/ && \
    rpmbuild -ba /src/pam_mfa-1.0.0/pam_mfa.spec

RUN \
    rpm -ihv ~/rpmbuild/RPMS/x86_64/pam_mfa-1.0.0-1.x86_64.rpm && \
    cp /src/pam_mfa-1.0.0/tester.conf /etc/pam.d/tester  && \
    cp /src/pam_mfa-1.0.0/test.sh /tmp/ && \
    echo auser > /etc/mfausers
