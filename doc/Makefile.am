# -- Process this file with automake to generate a `Makefile.in' file. --

EXTRA_DIST = nxbelld.pod
dist_man1_MANS = nxbelld.1

maintainer-clean-local:
	-rm -f nxbelld.1

.pod.1:
	pod2man --section=1 --center="User Commands" --release="@PACKAGE_STRING@" --nourls $< > $@
