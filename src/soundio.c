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

#ifdef HAVE_SOUNDIO

#include "pcm.h"
#include <sndio.h>

bool
play_pcm_buffer (playable_pcm_buffer_t *buffer)
{
  int               status;
  struct sio_hdl   *handle;
  struct sio_par    parameters;
  size_t            playback_chunk;
  size_t            to_write;
  size_t            already_wrote;
  size_t            wrote_bytes;


  handle = sio_open (SIO_DEVANY, SIO_PLAY, 0);
  if (handle == NULL)
    {
      fprintf (stderr, "%s: Failed to open the playback device.\n", progname);

      return false;
    }

  sio_initpar (&parameters);

  parameters.bits   = buffer->info.bits_per_sample;
  parameters.bps    = buffer->info.bytes_per_sample;
  parameters.sig    = buffer->info.sign ? 1 : 0;
  parameters.le     = buffer->info.native_endian ? SIO_LE_NATIVE : 1;
  parameters.pchan  = buffer->info.channels;
  parameters.rate   = buffer->info.sample_rate;
  parameters.xrun   = SIO_IGNORE;

  status = sio_setpar (handle, &parameters);
  if (!status)
    {
      fprintf (stderr, "%s: Failed to configure the playback device.\n",
               progname);

      sio_close (handle);
      return false;
    }

  status = sio_getpar (handle, &parameters);
  if (!status)
    {
      fprintf (stderr, "%s: Failed to check the playback device configuration.\n",
               progname);

      sio_close (handle);
      return false;
    }

  if (parameters.bits     != buffer->info.bits_per_sample
      || parameters.bps   != buffer->info.bytes_per_sample
      || parameters.pchan != buffer->info.channels
      || parameters.rate  != buffer->info.sample_rate)
    {
      fprintf (stderr, "%s: Configuring the playback device for the given data failed.\n",
               progname);

      sio_close (handle);
      return false;
    }

  if (parameters.appbufsz == 0)     /* Just in case, you never know... */
    parameters.appbufsz = 8192;

  playback_chunk = parameters.appbufsz * parameters.bps * parameters.pchan;
  already_wrote  = 0;

  status = sio_start (handle);
  if (!status)
    {
      fprintf (stderr, "%s: Failed to start playback.\n", progname);

      sio_close (handle);
      return false;
    }

  while (already_wrote < buffer->data_len)
    {
      if (buffer->data_len - already_wrote < playback_chunk)
        to_write = buffer->data_len - already_wrote;
      else
        to_write = playback_chunk;

      wrote_bytes = sio_write (handle, buffer->data + already_wrote, to_write);

      already_wrote += wrote_bytes;
      if (wrote_bytes != to_write)
        fprintf (stderr, "%s: Warning: Wrote only %lu bytes instead of the "
                         "expected %lu to the playback device.\n",
                 progname, wrote_bytes, to_write);
    }

  sio_stop (handle);
  sio_close (handle);

  return true;
}

bool
play_pcm_file (playable_pcm_file_t *file)
{
  int               status;
  struct sio_hdl   *handle;
  struct sio_par    parameters;
  uint8_t          *playback_buf;
  size_t            playback_buflen;
  size_t            read_bytes;
  size_t            wrote_bytes;

  if (fsetpos (file->stream, &(file->pcm_start_pos)) != 0)
    {
      fprintf (stderr, "%s: Failed to seek to the PCM data of `%s': %s.\n",
               progname, file->name, strerror (errno));

      return false;
    }

  handle = sio_open (SIO_DEVANY, SIO_PLAY, 0);
  if (handle == NULL)
    {
      fprintf (stderr, "%s: Failed to open the playback device.\n", progname);

      return false;
    }

  sio_initpar (&parameters);

  parameters.bits   = file->info.bits_per_sample;
  parameters.bps    = file->info.bytes_per_sample;
  parameters.sig    = file->info.sign ? 1 : 0;
  parameters.le     = file->info.native_endian ? SIO_LE_NATIVE : 1;
  parameters.pchan  = file->info.channels;
  parameters.rate   = file->info.sample_rate;
  parameters.xrun   = SIO_IGNORE;

  status = sio_setpar (handle, &parameters);
  if (!status)
    {
      fprintf (stderr, "%s: Failed to configure the playback device.\n",
               progname);

      sio_close (handle);
      return false;
    }

  status = sio_getpar (handle, &parameters);
  if (!status)
    {
      fprintf (stderr, "%s: Failed to check the playback device configuration.\n",
               progname);

      sio_close (handle);
      return false;
    }

  if (parameters.bits     != file->info.bits_per_sample
      || parameters.bps   != file->info.bytes_per_sample
      || parameters.pchan != file->info.channels
      || parameters.rate  != file->info.sample_rate)
    {
      fprintf (stderr, "%s: Configuring the playback device for `%s' failed.\n",
               progname, file->name);

      sio_close (handle);
      return false;
    }

  if (parameters.appbufsz == 0)     /* Just in case, you never know... */
    parameters.appbufsz = 8192;

  playback_buflen = parameters.appbufsz * parameters.bps * parameters.pchan;
  playback_buf = malloc (playback_buflen);
  if (playback_buf == NULL)
    {
      fprintf (stderr, "%s: Failed to create a data buffer: %s\n",
               progname, strerror (errno));

      sio_close (handle);
      return false;
    }

  status = sio_start (handle);
  if (!status)
    {
      fprintf (stderr, "%s: Failed to start playback.\n", progname);

      sio_close (handle);
      free (playback_buf);
      return false;
    }

  while (true)
    {
      read_bytes = fread (playback_buf, 1, playback_buflen, file->stream);
      if (read_bytes == 0)
        {
          if (ferror (file->stream))
            {
              fprintf (stderr, "%s: An error occured while reading from `%s': %s.\n",
                       progname, file->name, strerror (errno));

              sio_stop (handle);
              sio_close (handle);
              free (playback_buf);

              return false;
            }

          break;
        }

      wrote_bytes = sio_write (handle, playback_buf, read_bytes);
      if (wrote_bytes != read_bytes)
        fprintf (stderr, "%s: Warning: Read %lu bytes, but wrote only %lu.\n",
                 progname, read_bytes, wrote_bytes);
    }

  sio_stop (handle);
  sio_close (handle);

  free (playback_buf);

  return true;
}

#endif /* HAVE_SOUNDIO */
