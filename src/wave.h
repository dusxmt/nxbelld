/**
 *  nxbelld, a fork of xbelld, the bell daemon for computers w/o a PC speaker.
 *
 *  Copyright (C) 2008-2013 Gautam Iyer <gi1242+xbelld@NoSPAM.com>
 *  Copyright (C) 2016  Marek Benc <dusxmt@gmx.com>
 *
 *  Note: replace NoSPAM with gmail
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

#ifndef _NXBELLD_WAVE_H_
#define _NXBELLD_WAVE_H_ 1

#include "common.h"


#ifdef HAVE_WAVE

#ifndef HAVE_SOUND
# error Sound support is needed for playing WAVE files.
#endif

#include "pcm.h"

/* Endian conversion code. */
#include <byteswap.h>
#ifdef WORDS_BIGENDIAN
# define COMPOSE_ID(a, b, c, d) ((d) | ((c) <<8) | ((b) << 16) | ((a) << 24))
# define LE_SHORT(v)            bswap_16 (v)
# define LE_INT(v)              bswap_32 (v)
# define BE_SHORT(v)            (v)
# define BE_INT(v)              (v)
#else
# define COMPOSE_ID(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
# define LE_SHORT(v)            (v)
# define LE_INT(v)              (v)
# define BE_SHORT(v)            bswap_16 (v)
# define BE_INT(v)              bswap_32 (v)
#endif

typedef struct wave_file_hdr  wave_file_hdr_t;
typedef struct wave_chunk_hdr wave_chunk_hdr_t;
typedef struct wave_fmt_chunk wave_fmt_chunk_t;

struct wave_file_hdr
{
  uint32_t magic;     /* Always "RIFF" */
  uint32_t file_len;  /* file length */
  uint32_t type;      /* Always "WAVE" */
};

struct wave_chunk_hdr
{
  uint32_t chunk_id;
  uint32_t chunk_len;
};

struct wave_fmt_chunk
{
  wave_chunk_hdr_t hdr;

  uint16_t pcm_code;  /* Always 0x01 */
  uint16_t channels;  /* Number of channels */
  uint32_t rate;      /* Frequency of sample */
  uint32_t byte_psec; /* Bytes per second */
  uint16_t byte_pspl; /* sample size; 1 or 2 bytes */
  uint16_t bit_pspl;  /* Bits per sample */
};

playable_pcm_buffer_t *load_wave_file_into_buffer (const char *path);
playable_pcm_file_t   *prepare_wave_file (const char *path);

#endif /* HAVE_WAVE */
#endif /* _NXBELLD_WAVE_H_ */
