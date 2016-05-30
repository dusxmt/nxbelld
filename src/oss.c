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

#ifdef HAVE_OSS

#include "pcm.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#define BUF_SIZE    4096 /* Recommended buffer size for "normal use" of OSS. */
#define DEVICE_NAME "/dev/dsp"

static int
determine_pcm_format (pcm_data_info_t *info)
{
  if (info->native_endian)
    {
      if (info->sign)
        {
          switch (info->bits_per_sample)
            {
              case 8:  return AFMT_S8;
              case 16: return AFMT_S16_NE;
              case 24: return AFMT_S24_NE;
              case 32: return AFMT_S32_NE;
            }
        }
      else
        {
          switch (info->bits_per_sample)
            {
              case 8:  return AFMT_U8;
              case 16: return AFMT_U16_NE;
              case 24: return AFMT_U24_NE;
              case 32: return AFMT_U32_NE;
            }
        }
    }
  else
    {
      if (info->sign)
        {
          switch (info->bits_per_sample)
            {
              case 8:  return AFMT_S8;
              case 16: return AFMT_S16_LE;
              case 24: return AFMT_S24_LE;
              case 32: return AFMT_S32_LE;
            }
        }
      else
        {
          switch (info->bits_per_sample)
            {
              case 8:  return AFMT_U8;
              case 16: return AFMT_U16_LE;
              case 24: return AFMT_U24_LE;
              case 32: return AFMT_U32_LE;
            }
        }
    }

  return -1;
}

static bool
configure_oss_device (int device, pcm_data_info_t *info)
{
  int               status;
  int               format;
  int               ioctl_ret;


  format = determine_pcm_format (info);
  if (format == -1)
    {
      fprintf (stderr, "%s: Unable to determine the beep's PCM data format.\n",
               progname);

      return false;
    }

  ioctl_ret = format;
  status = ioctl (device, SNDCTL_DSP_SETFMT, &ioctl_ret);
  if (status == -1)
    {
      fprintf (stderr, "%s: Failed to set the playback device's sound format: %s.\n",
               progname, strerror (errno));

      return false;
    }
  if (ioctl_ret != format)
    {
      fprintf (stderr, "%s: Failed to set the playback device's sound format.\n",
               progname);

      return false;
    }

  ioctl_ret = info->channels;
  status = ioctl (device, SNDCTL_DSP_CHANNELS, &ioctl_ret);
  if (status == -1)
    {
      fprintf (stderr, "%s: Failed to set the playback device's channel count: %s.\n",
               progname, strerror (errno));

      return false;
    }
  if (ioctl_ret != info->channels)
    {
      fprintf (stderr, "%s: Failed to set the playback device's channel count.\n",
               progname);

      return false;
    }

  ioctl_ret = info->sample_rate;
  status = ioctl (device, SNDCTL_DSP_SPEED, &ioctl_ret);
  if (status == -1)
    {
      fprintf (stderr, "%s: Failed to set the playback device's sampling rate: %s.\n",
               progname, strerror (errno));

      return false;
    }
  if (ioctl_ret != info->sample_rate)
    {
      fprintf (stderr, "%s: Failed to set the playback device's sampling rate.\n",
               progname);

      return false;
    }

  return true;
}


bool
play_pcm_buffer (playable_pcm_buffer_t *buffer)
{
  int               status;
  int               format;
  int               ioctl_ret;
  int               device;
  size_t            to_write;
  size_t            already_wrote;
  ssize_t           wrote_bytes;


  device = open (DEVICE_NAME, O_WRONLY, 0);
  if (device == -1)
    {
      fprintf (stderr, "%s: Failed to open `%s' for writing: %s.\n",
               progname, DEVICE_NAME, strerror (errno));

      return false;
    }

  if (! configure_oss_device (device, &(buffer->info)))
    {
      fprintf (stderr, "%s: Failed to configure the playback device.\n",
               progname);

      close (device);
      return false;
    }

  already_wrote  = 0;
  while (already_wrote < buffer->data_len)
    {
      if (buffer->data_len - already_wrote < BUF_SIZE)
        to_write = buffer->data_len - already_wrote;
      else
        to_write = BUF_SIZE;

      wrote_bytes = write (device, buffer->data + already_wrote, to_write);

      if (wrote_bytes == -1)
        {
          fprintf (stderr, "%s: An error occured while writing to the playback device: %s.\n",
                   progname, strerror (errno));

          close (device);
          return false;
        }

      already_wrote += wrote_bytes;
      if (wrote_bytes != to_write)
        fprintf (stderr, "%s: Warning: Wrote only %ld bytes instead of the "
                         "expected %lu to the playback device.\n",
                 progname, wrote_bytes, to_write);
    }

  close (device);
  return true;
}

bool
play_pcm_file (playable_pcm_file_t *file)
{
  int               status;
  int               device;
  uint8_t           playback_buf[BUF_SIZE];
  size_t            read_bytes;
  ssize_t           wrote_bytes;

  if (fsetpos (file->stream, &(file->pcm_start_pos)) != 0)
    {
      fprintf (stderr, "%s: Failed to seek to the PCM data of `%s': %s.\n",
               progname, file->name, strerror (errno));

      return false;
    }

  device = open (DEVICE_NAME, O_WRONLY, 0);
  if (device == -1)
    {
      fprintf (stderr, "%s: Failed to open `%s' for writing: %s.\n",
               progname, DEVICE_NAME, strerror (errno));

      return false;
    }

  if (! configure_oss_device (device, &(file->info)))
    {
      fprintf (stderr, "%s: Failed to configure the playback device.\n",
               progname);

      close (device);
      return false;
    }


  while (true)
    {
      read_bytes = fread (playback_buf, 1, BUF_SIZE, file->stream);
      if (read_bytes == 0)
        {
          if (ferror (file->stream))
            {
              fprintf (stderr, "%s: An error occured while reading from `%s': %s.\n",
                       progname, file->name, strerror (errno));

              close (device);
              return false;
            }

          break;
        }

      wrote_bytes = write (device, playback_buf, read_bytes);

      if (wrote_bytes == -1)
        {
          fprintf (stderr, "%s: An error occured while writing to the playback device: %s.\n",
                   progname, strerror (errno));

          close (device);
          return false;
        }

      if (wrote_bytes != read_bytes)
        fprintf (stderr, "%s: Warning: Read %lu bytes, but wrote only %ld.\n",
                 progname, read_bytes, wrote_bytes);
    }

  close (device);
  return true;
}

#endif /* HAVE_OSS */
