# -- Process this file with automake to generate a `Makefile.in' file. --

bin_PROGRAMS      =	nxbelld

nxbelld_SOURCES   =	common.h	\
			main.c		\
					\
			beep.h		\
			beep.c		\
			pcm.h		\
			pcm.c		\
			wave.h		\
			wave.c		\
					\
			alsa.c		\
			oss.c		\
			soundio.c

nxbelld_CPPFLAGS  =	-I$(top_builddir)/gnulib -I$(top_srcdir)/gnulib \
			@X11_CFLAGS@

nxbelld_LDADD     =	@X11_LIBS@ $(top_builddir)/gnulib/libgnu.a


if NXBELLD_ALSA_ENABLED
nxbelld_CPPFLAGS +=	@ALSA_CFLAGS@ -DHAVE_ALSA
nxbelld_LDADD    +=	@ALSA_LIBS@
endif

if NXBELLD_OSS_ENABLED
nxbelld_CPPFLAGS +=	-DHAVE_OSS
endif

if NXBELLD_SOUNDIO_ENABLED
nxbelld_CPPFLAGS +=	-DHAVE_SOUNDIO
nxbelld_LDADD    +=	-lsndio
endif

if NXBELLD_WAVE_ENABLED
nxbelld_CPPFLAGS +=	-DHAVE_WAVE
endif
