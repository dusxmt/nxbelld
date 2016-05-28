/**
 *  nxbelld, a fork of xbelld, the X bell daemon for computers w/o a PC speaker.
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

#include "common.h"
#include "pcm.h"

#ifdef HAVE_SOUND

void
free_pcm_buffer (playable_pcm_buffer_t *buffer)
{
  if (buffer == NULL)
    return;

  if (buffer->data != NULL)
    free (buffer->data);

  free (buffer);
}

void
close_pcm_file (playable_pcm_file_t *file)
{
  if (file == NULL)
    return;

  if (file->name != NULL)
    free (file->name);
  if (file->stream != NULL)
    fclose (file->stream);

  free (file);
}

#endif /* HAVE_SOUND */
