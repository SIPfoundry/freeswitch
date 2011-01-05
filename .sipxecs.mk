# format:  ${number of commits past tag}.${refid of HEAD}
freeswitch_PACKAGE_REVISION = $(shell \
	cd $(SRC)/$(PROJ); \
	$(SRC)/config/git-version-gen .version | \
	sed 's/\([0-9.]\+\)-\([0-9]\+\)-\([0-9a-f]\+\)/\2.\3/g')

freeswitch_SRPM = freeswitch-1.0.7-$(freeswitch_PACKAGE_REVISION).src.rpm
freeswitch_SPEC = $(SRC)/$(PROJ)/freeswitch.spec
freeswitch_SOURCES = $(SRC)/$(PROJ)/freeswitch-1.0.7.tar.bz2 \
	celt-0.7.1.tar.gz \
	flite-1.3.99-latest.tar.gz \
	lame-3.97.tar.gz \
	libshout-2.2.2.tar.gz \
	mpg123.tar.gz \
	openldap-2.4.11.tar.gz \
	pocketsphinx-0.5.99-20091212.tar.gz \
	soundtouch-1.3.1.tar.gz \
	sphinxbase-0.4.99-20091212.tar.gz \
	communicator_semi_6000_20080321.tar.gz \
	libmemcached-0.32.tar.gz

freeswitch_SRPM_DEFS = --define "buildno $(freeswitch_PACKAGE_REVISION)"
freeswitch_RPM_DEFS = --define="buildno $(freeswitch_PACKAGE_REVISION)"

# we could, but we don't these targets. FS doesn't support running ./configure from anywhere but source root.
freeswitch.autoreconf freeswitch.configure:;

freeswitch.dist :
	cd $(SRC)/$(PROJ); \
	  git archive --format tar --prefix freeswitch-1.0.7/ HEAD | bzip2 > freeswitch-1.0.7.tar.bz2
