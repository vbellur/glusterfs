EXTRA_DIST = autogen.sh COPYING INSTALL README AUTHORS THANKS NEWS EXCEPTIONS glusterfs.spec

SUBDIRS = argp-standalone libglusterfs rpc xlators glusterfsd $(FUSERMOUNT_SUBDIR) doc extras cli

CLEANFILES = 

gitclean: distclean
	find . -name Makefile.in -exec rm -f {} \;
	find . -name Makefile -exec rm -f {} \;
	find . -name mount.glusterfs -exec rm -f {} \;
	rm -fr autom4te.cache
	rm -f missing aclocal.m4 config.h.in config.guess config.sub ltmain.sh install-sh configure depcomp
	rm -fr argp-standalone/autom4te.cache
	rm -f argp-standalone/aclocal.m4 argp-standalone/config.h.in argp-standalone/configure argp-standalone/depcomp argp-standalone/install-sh argp-standalone/missing
