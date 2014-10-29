# For now, we'll assume we're building on linux only
PLATFORMS=linux

.PHONY:	$(PLATFORMS) all clean realclean tar

sim:	PLATFORMS:=sim

all:	$(PLATFORMS)

export MFM_VERSION_NUMBER
doc:	FORCE
	mkdir -p doc/ref
	doxygen

clean:  $(PLATFORMS)

realclean:  $(PLATFORMS)
	rm -f bin/*
	rm -f res/elements/*.so
	rm -rf build/
	rm -rf doc/ref

include config/Makeversion.mk
TAR_EXCLUDES+=--exclude=tools --exclude=*~ --exclude=.git --exclude=doc/internal --exclude=spikes --exclude-backups
tar:	FORCE
	make realclean
	PWD=`pwd`;BASE=`basename $$PWD`;cd ..;tar cvzf mfm-$(MFM_VERSION_NUMBER).tgz $(TAR_EXCLUDES) $$BASE

LCDIR:=/var/www/vhosts/livingcomputation.com/htdocs/MFM
ackleyRelease:	ackleyReleaseDoc ackleyReleaseTar

ackleyReleaseDoc:	ensureReleaseVersion doc
	ssh livingcomputation.com 'cd $(LCDIR)/ref; mkdir -p $(MFM_VERSION_NUMBER); ln -sf $(MFM_VERSION_NUMBER) newest'
	scp -r doc/ref/html/. livingcomputation.com:$(LCDIR)/ref/$(MFM_VERSION_NUMBER)

ackleyReleaseTar:	ensureReleaseVersion tar # Watch out, 'make tar' nukes 'doc/ref'!
	scp -r ../mfm-$(MFM_VERSION_NUMBER).tgz livingcomputation.com:$(LCDIR)/tar/
	ssh livingcomputation.com 'cd $(LCDIR)/tar; ln -sf mfm-$(MFM_VERSION_NUMBER).tgz mfm-newest.tgz'

ensureReleaseVersion:	$(if $(MFM_IS_DEVEL_VERSION), $(error Do not release devel version $(MFM_VERSION_NUMBER)))

include config/Makedebian.mk

# Pass each entry in PLATFORMS down as a target
$(PLATFORMS):
	export MFM_TARGET=$@;$(MAKE) -C src $(MAKECMDGOALS)

.PHONY:	FORCE
