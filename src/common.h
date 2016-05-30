/**
 *  nxbelld, a fork of xbelld, the bell daemon for computers w/o a PC speaker.
 *
 *  Copyright (C) 2016  Marek Benc <dusxmt@gmx.com>
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but HAVEOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NXBELLD_COMMON_H_
#define _NXBELLD_COMMON_H_ 1

/* The project configuration. */
#include <config.h>

/* Useful headers. */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Program name global. */
extern const char *progname;

/* Sanity check. */
#ifdef HAVE_ALSA
# ifndef  HAVE_SOUND
#  define HAVE_SOUND 1
# else
#  error You can compile nxbelld against only a single sound API at the moment.
# endif
#endif

#ifdef HAVE_OSS
# ifndef  HAVE_SOUND
#  define HAVE_SOUND 1
# else
#  error You can compile nxbelld against only a single sound API at the moment.
# endif
#endif

#ifdef HAVE_SOUNDIO
# ifndef  HAVE_SOUND
#  define HAVE_SOUND 1
# else
#  error You can compile nxbelld against only a single sound API at the moment.
# endif
#endif


#endif /* _NXBELLD_COMMON_H_ */
