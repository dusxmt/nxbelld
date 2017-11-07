dnl -- Process this file with autoconf to generate a `configure' script. --

AC_PREREQ([2.69])
AC_INIT([nxbelld], [0.1.2-dev], [dusxmt@gmx.com])

AC_CONFIG_SRCDIR([src/soundio.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile gnulib/Makefile src/Makefile doc/Makefile])

AM_INIT_AUTOMAKE([1.11.6 -Wall -Werror])

# Options for enabling/disabling optional features.
AC_ARG_ENABLE([sound],
              [AS_HELP_STRING([--enable-sound],
               [enable sound support [default=yes]])],
              [enable_sound=$enableval], [enable_sound=yes])
AC_ARG_ENABLE([wave],
              [AS_HELP_STRING([--enable-wave],
               [enable support for wave files [default=auto]])],
              [enable_wave=$enableval], [enable_wave=auto])
AC_ARG_ENABLE([alsa],
              [AS_HELP_STRING([--enable-alsa],
               [enable alsa support [default=auto]])],
              [enable_alsa=$enableval], [enable_alsa=auto])
AC_ARG_ENABLE([oss],
              [AS_HELP_STRING([--enable-oss],
               [enable OSS support [default=auto]])],
              [enable_oss=$enableval], [enable_oss=auto])
AC_ARG_ENABLE([soundio],
              [AS_HELP_STRING([--enable-soundio],
               [enable support for soundio [default=auto]])],
              [enable_soundio=$enableval], [enable_soundio=auto])

# Checks for programs.
AC_PROG_CC
gl_EARLY
gl_INIT
AM_PROG_CC_C_O

# Check the system endianness.
AC_C_BIGENDIAN

# Checks for libraries.
AC_CHECK_LIB([m], [sin])

PKG_CHECK_MODULES([X11], [x11])
AC_SUBST([X11_CFLAGS])
AC_SUBST([X11_LIBS])

# Check for sound support.
have_sound=no
have_wave=no
have_alsa=no
have_oss=no
have_soundio=no
if test x"$enable_sound" != x"no"; then

  # Make sure only one sound API is used.
  if test x"$enable_alsa" = x"yes"; then
    if test x"$enable_oss" = x"yes"; then
      AC_MSG_ERROR([Only a single sound API is allowed.])
    else
      enable_oss=no
    fi
    if test x"$enable_soundio" = x"yes"; then
      AC_MSG_ERROR([Only a single sound API is allowed.])
    else
      enable_soundio=no
    fi
  fi
  if test x"$enable_oss" = x"yes"; then
    if test x"$enable_alsa" = x"yes"; then
      AC_MSG_ERROR([Only a single sound API is allowed.])
    else
      enable_alsa=no
    fi
    if test x"$enable_soundio" = x"yes"; then
      AC_MSG_ERROR([Only a single sound API is allowed.])
    else
      enable_soundio=no
    fi
  fi
  if test x"$enable_soundio" = x"yes"; then
    if test x"$enable_alsa" = x"yes"; then
      AC_MSG_ERROR([Only a single sound API is allowed.])
    else
      enable_alsa=no
    fi
    if test x"$enable_oss" = x"yes"; then
      AC_MSG_ERROR([Only a single sound API is allowed.])
    else
      enable_oss=no
    fi
  fi

  # Detect the used sound API.
  if test x"$enable_alsa" != x"no"; then
    if test x"$have_sound" != x"yes"; then
      have_alsa_lib=no
      if test x"$enable_alsa" = x"yes"; then
        PKG_CHECK_MODULES([ALSA], [alsa], [have_alsa_lib=yes])
      else
        PKG_CHECK_MODULES([ALSA], [alsa], [have_alsa_lib=yes], [true])
      fi

      if test x"$have_alsa_lib" = x"yes"; then
        AC_SUBST([ALSA_CFLAGS])
        AC_SUBST([ALSA_LIBS])
        have_alsa=yes
        have_sound=yes
      fi
    fi
  fi
  if test x"$enable_oss" != x"no"; then
    if test x"$have_sound" != x"yes"; then
      have_oss_inc=no
      AC_CHECK_HEADER([sys/soundcard.h], [have_oss_inc=yes])

      if test x"$have_oss_inc" = x"yes"; then
        have_oss=yes
        have_sound=yes
      fi
      if test x"$enable_oss" = x"yes"; then
        if test x"$have_oss" != x"yes"; then
          AC_MSG_ERROR([Could not find the OSS support header.])
        fi
      fi
    fi
  fi
  if test x"$enable_soundio" != x"no"; then
    if test x"$have_sound" != x"yes"; then
      have_soundio_lib=no
      have_soundio_inc=no
      AC_CHECK_LIB([sndio], [sio_open], [have_soundio_lib=yes])
      AC_CHECK_HEADER([sndio.h], [have_soundio_inc=yes])

      if test x"$have_soundio_lib" = x"yes"; then
        if test x"$have_soundio_inc" = x"yes"; then
          have_soundio=yes
          have_sound=yes
        fi
      fi
      if test x"$enable_soundio" = x"yes"; then
        if test x"$have_soundio" != x"yes"; then
          AC_MSG_ERROR([Could not find the soundio library.])
        fi
      fi
    fi
  fi
  if test x"$enable_sound" = x"yes"; then
    if test x"$have_sound" != x"yes"; then

      AC_MSG_ERROR([Could not find a supported sound API.

Try the --disable-sound option if you don't need sound support.])
    fi
  fi
  if test x"$enable_wave" != x"no"; then
    if test x"$have_sound" = x"yes"; then
      have_wave=yes
    fi
  fi
fi

if test x"$enable_wave" = x"yes"; then
  if test x"$enable_sound" = x"no"; then
    AC_MSG_ERROR([WAVE support requires sound support.])
  fi
fi

# Information for Automake.
AM_CONDITIONAL([NXBELLD_ALSA_ENABLED],    [test x"$have_alsa" = x"yes"])
AM_CONDITIONAL([NXBELLD_OSS_ENABLED],     [test x"$have_oss" = x"yes"])
AM_CONDITIONAL([NXBELLD_SOUNDIO_ENABLED], [test x"$have_soundio" = x"yes"])
AM_CONDITIONAL([NXBELLD_WAVE_ENABLED],    [test x"$have_wave" = x"yes"])


AC_OUTPUT

echo ""
echo ""
echo "Configuration summary for nxbelld:"
echo ""
echo "Sound support:         $have_sound"
echo "WAVE file support:     $have_wave"
echo "ALSA support:          $have_alsa"
echo "OSS support:           $have_oss"
echo "soundio support:       $have_soundio"
