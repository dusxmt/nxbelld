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

#ifndef _NXBELLD_PCM_H_
#define _NXBELLD_PCM_H_ 1

#include "common.h"


#ifdef HAVE_SOUND

typedef struct pcm_data_info       pcm_data_info_t;
typedef struct playable_pcm_buffer playable_pcm_buffer_t;
typedef struct playable_pcm_file   playable_pcm_file_t;

struct pcm_data_info
{
  bool native_endian;
  bool sign;
  int  rate;
  int  channels;
  int  byte_pspl;
  int  bit_pspl;
};

struct playable_pcm_buffer
{
  uint8_t        *data;
  uint32_t        data_len;

  pcm_data_info_t info;
};

struct playable_pcm_file
{
  char    *name;
  FILE    *stream;
  fpos_t   pcm_start_pos;

  pcm_data_info_t info;
};

void free_pcm_buffer (playable_pcm_buffer_t *buffer);
void close_pcm_file (playable_pcm_file_t *file);

/* Note: These two routines are implemented outside of pcm.c: */
bool play_pcm_buffer (playable_pcm_buffer_t *buffer);
bool play_pcm_file (playable_pcm_file_t *file);

#endif /* HAVE_SOUND */
#endif /* _NXBELLD_PCM_H_ */
