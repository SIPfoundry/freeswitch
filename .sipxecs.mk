# format:  ${number of commits past tag}.${refid of HEAD}
freeswitch_VER = 1.0.7
freeswitch_REV = $(shell \
	cd $(SRC)/$(PROJ); \
	$(SRC)/config/revision-gen $(freeswitch_VER))

freeswitch_SRPM = freeswitch-$(freeswitch_VER)-$(freeswitch_REV).src.rpm
freeswitch_SPEC = $(SRC)/$(PROJ)/freeswitch.spec
freeswitch_TARBALL = $(BUILDDIR)/$(PROJ)/freeswitch-$(freeswitch_VER).tar.bz2
freeswitch_SOURCES = $(freeswitch_TARBALL) \
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

freeswitch_SRPM_DEFS = --define "buildno $(freeswitch_REV)"
freeswitch_RPM_DEFS = --define="buildno $(freeswitch_REV)"

# we could, but we don't these targets. FS doesn't support running ./configure from anywhere but source root.
freeswitch.autoreconf freeswitch.configure:;

freeswitch.dist :
	test -d $(dir $(freeswitch_TARBALL)) || mkdir -p $(dir $(freeswitch_TARBALL))
	cd $(SRC)/$(PROJ); \
	  git archive --format tar --prefix freeswitch-$(freeswitch_VER)/ HEAD | bzip2 > $(freeswitch_TARBALL)
