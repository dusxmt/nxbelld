# -- Process this file with automake to generate a `Makefile.in' file. --

ACLOCAL_AMFLAGS = -I m4
SUBDIRS         = gnulib src doc
EXTRA_DIST      = m4/gnulib-cache.m4 ChangeLog.xbelld


dist-hook: generate-chlog

generate-chlog:
	if test -d .git; then					\
	  $(top_srcdir)/build-aux/gitlog-to-changelog		\
	    > $(distdir)/ChangeLog.new;				\
	  rm -f $(distdir)/ChangeLog;				\
	  mv $(distdir)/ChangeLog.new $(distdir)/ChangeLog;	\
	fi
