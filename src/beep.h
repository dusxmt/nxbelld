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

#ifndef _NXBELLD_BEEP_H_
#define _NXBELLD_BEEP_H_ 1

#include "common.h"
#include "pcm.h"


/* Beep descriptor. */
typedef struct beep_descriptor beep_descriptor_t;

struct beep_descriptor
{
  unsigned int           type;

#ifdef HAVE_SOUND

  playable_pcm_buffer_t *buffer;
  playable_pcm_file_t   *file;

#endif

  char                  *command;
};
enum
{
  BEEP_TYPE_BUFFER,
  BEEP_TYPE_FILE,
  BEEP_TYPE_COMMAND
};

#ifdef HAVE_SOUND

playable_pcm_buffer_t *generate_sine_beep (unsigned int volume,
                                           unsigned int frequency,
                                           unsigned int duration);

playable_pcm_buffer_t *generate_complex_beep (unsigned int volume,
                                              unsigned int frequency,
                                              unsigned int duration);

playable_pcm_buffer_t *generate_square_beep (unsigned int volume,
                                             unsigned int frequency,
                                             unsigned int duration);

#endif /* HAVE_SOUND */


bool perform_beep   (beep_descriptor_t *beep);
void free_beep_desc (beep_descriptor_t *beep);


#endif /* _NXBELLD_BEEP_H_ */
