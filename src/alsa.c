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

#ifdef HAVE_ALSA

#include "pcm.h"
#include <alsa/asoundlib.h>

snd_pcm_format_t determine_pcm_format (pcm_data_info_t *info)
{
  if (info->native_endian)
    {
      if (info->sign)
        {
          switch (info->bit_pspl)
            {
              case 8:  return SND_PCM_FORMAT_S8;
              case 16: return SND_PCM_FORMAT_S16;
              case 24: return SND_PCM_FORMAT_S24;
              case 32: return SND_PCM_FORMAT_S32;
            }
        }
      else
        {
          switch (info->bit_pspl)
            {
              case 8:  return SND_PCM_FORMAT_U8;
              case 16: return SND_PCM_FORMAT_U16;
              case 24: return SND_PCM_FORMAT_U24;
              case 32: return SND_PCM_FORMAT_U32;
            }
        }
    }
  else
    {
      if (info->sign)
        {
          switch (info->bit_pspl)
            {
              case 8:  return SND_PCM_FORMAT_S8;
              case 16: return SND_PCM_FORMAT_S16_LE;
              case 24: return SND_PCM_FORMAT_S24_LE;
              case 32: return SND_PCM_FORMAT_S32_LE;
            }
        }
      else
        {
          switch (info->bit_pspl)
            {
              case 8:  return SND_PCM_FORMAT_U8;
              case 16: return SND_PCM_FORMAT_U16_LE;
              case 24: return SND_PCM_FORMAT_U24_LE;
              case 32: return SND_PCM_FORMAT_U32_LE;
            }
        }
    }

  return SND_PCM_FORMAT_UNKNOWN;
}


bool
play_pcm_buffer (playable_pcm_buffer_t *buffer)
{
  int                 status;
  snd_pcm_t          *handle;
  snd_pcm_format_t    format;
  snd_pcm_sframes_t   frames_wrote;
  int                 frames_count;
  size_t              bytes_handled;
  size_t              bytes_to_write;


  format = determine_pcm_format (&(buffer->info));
  if (format == SND_PCM_FORMAT_UNKNOWN)
    {
      fprintf (stderr, "%s: Unable to determine the beep's PCM data format.\n",
               progname);

      return false;
    }

  status = snd_pcm_open (&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (status < 0)
    {
      fprintf (stderr, "%s: Failed to open the playback device: %s\n",
               progname, snd_strerror (status));

      return false;
    }

  status = snd_pcm_set_params (handle, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                               buffer->info.channels, buffer->info.rate,
                               1,          /* soft_resample */
                               0);         /* latency (us).*/
  if (status < 0)
    {
      fprintf (stderr, "%s: Failed to configure the playback device: %s.\n",
               progname, snd_strerror (status));

      snd_pcm_close (handle);
      return false;
    }

  bytes_handled = 0;
  while (bytes_handled < buffer->data_len)
    {
      if (buffer->data_len - bytes_handled < BUFSIZ)
        bytes_to_write = buffer->data_len - bytes_handled;
      else
        bytes_to_write = BUFSIZ;

      frames_count = snd_pcm_bytes_to_frames (handle, bytes_to_write);
      frames_wrote = snd_pcm_writei (handle, buffer->data + bytes_handled,
                                     frames_count);
      if (frames_wrote < 0)
        frames_wrote = snd_pcm_recover (handle, frames_wrote, 0);

      if (frames_wrote < 0)
        {
          fprintf (stderr, "%s: Writing to the playback device failed: %s.\n",
                   progname, snd_strerror (frames_wrote));

          snd_pcm_close (handle);
          return false;
        }

      if (frames_wrote > 0 && frames_wrote < frames_count)
        {
          fprintf (stderr, "%s: Warning: Wrote only %ld frames instead of the "
                   "expected %d to the playback device.\n",
                   progname, frames_wrote, frames_count);
        }

      bytes_handled += bytes_to_write;
    }

  snd_pcm_drain (handle);
  snd_pcm_close (handle);

  return true;
}

bool
play_pcm_file (playable_pcm_file_t *file)
{
  int                 status;
  snd_pcm_t          *handle;
  snd_pcm_format_t    format;
  snd_pcm_sframes_t   frames_wrote;
  uint8_t             playback_buf[BUFSIZ];
  int                 frames_count;
  size_t              read_bytes;


  format = determine_pcm_format (&(file->info));
  if (format == SND_PCM_FORMAT_UNKNOWN)
    {
      fprintf (stderr, "%s: Unable to determine the beep's PCM data format.\n",
               progname);

      return false;
    }

  if (fsetpos (file->stream, &(file->pcm_start_pos)) != 0)
    {
      fprintf (stderr, "%s: Failed to seek to the PCM data of `%s': %s.\n",
               progname, file->name, strerror (errno));

      return false;
    }

  status = snd_pcm_open (&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (status < 0)
    {
      fprintf (stderr, "%s: Failed to open the playback device: %s\n",
               progname, snd_strerror (status));

      return false;
    }

  status = snd_pcm_set_params (handle, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                               file->info.channels, file->info.rate,
                               1,          /* soft_resample */
                               0);         /* latency (us).*/
  if (status < 0)
    {
      fprintf (stderr, "%s: Failed to configure the playback device: %s.\n",
               progname, snd_strerror (status));

      snd_pcm_close (handle);
      return false;
    }

  while (true)
    {
      read_bytes = fread (playback_buf, 1, BUFSIZ, file->stream);
      if (read_bytes == 0)
        {
          if (ferror (file->stream))
            {
              fprintf (stderr, "%s: An error occured while reading from `%s': %s.\n",
                       progname, file->name, strerror (errno));

              snd_pcm_close (handle);
              return false;
            }

          break;
        }
      frames_count = snd_pcm_bytes_to_frames (handle, read_bytes);
      frames_wrote = snd_pcm_writei (handle, playback_buf, frames_count);
      if (frames_wrote < 0)
        frames_wrote = snd_pcm_recover (handle, frames_wrote, 0);

      if (frames_wrote < 0)
        {
          fprintf (stderr, "%s: Writing to the playback device failed: %s.\n",
                   progname, snd_strerror (frames_wrote));

          snd_pcm_close (handle);
          return false;
        }

      if (frames_wrote > 0 && frames_wrote < frames_count)
        {
          fprintf (stderr, "%s: Warning: Wrote only %ld frames instead of the "
                   "expected %d to the playback device.\n",
                   progname, frames_wrote, frames_count);
        }
    }

  snd_pcm_drain (handle);
  snd_pcm_close (handle);

  return true;
}

#endif /* HAVE_SOUNDIO */
