/**
 *  nxbelld, a fork of xbelld, the X bell daemon for computers w/o a PC speaker.
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

#include "common.h"

#ifdef HAVE_WAVE

#include "pcm.h"
#include "wave.h"
#include <math.h>

static bool
find_wave_pcm_data (FILE *stream, uint32_t *data_len, fpos_t *pcm_start_pos)
{
  wave_chunk_hdr_t header;
  while (true)
    {
      if (fread (&header, sizeof (wave_chunk_hdr_t), 1, stream) != 1)
        {
          fprintf (stderr, "%s: Failed to read a chunk of the file header: %s.\n",
                   progname,
                   feof (stream) ? "Unexpected end of file" : strerror (errno));

          return false;
        }
      if (header.chunk_id != COMPOSE_ID ('d', 'a', 't', 'a'))
        {
          fseek (stream, LE_INT (header.chunk_len), SEEK_CUR);
        }
      else
        break;
    }

  if (pcm_start_pos != NULL)
    {
      if (fgetpos (stream, pcm_start_pos) != 0)
        {
          fprintf (stderr, "%s: Failed to store the location of the PCM data: %s.\n",
                   progname, strerror (errno));

          return false;
        }
    }

  if (data_len != NULL)
    *data_len = LE_INT (header.chunk_len);

  return true;
}

static bool
parse_wave_header (FILE *stream, pcm_data_info_t *info)
{
  wave_file_hdr_t  header;
  wave_fmt_chunk_t format;

  if (fread (&header, sizeof (wave_file_hdr_t), 1, stream) != 1)
    {
      fprintf (stderr, "%s: Failed to read the file header: %s.\n",
               progname, strerror (errno));

      return false;
    }

  if (header.magic != COMPOSE_ID ('R', 'I', 'F', 'F')
      || header.type != COMPOSE_ID ('W', 'A', 'V', 'E'))
    {
      fprintf (stderr, "%s: Not a RIFF WAVE file.\n", progname);

      return false;
    }

  if (fread (&format, sizeof (wave_fmt_chunk_t), 1, stream) != 1)
    {
      fprintf (stderr, "%s: Failed to read the format chunk from the file header: %s.\n",
               progname, strerror (errno));

      return false;
    }

  /* Swap some endian, if needed. Only swap fields we make use of. */
  format.hdr.chunk_len    = LE_INT   (format.hdr.chunk_len);
  format.data_format      = LE_SHORT (format.data_format);
  format.channels         = LE_SHORT (format.channels);
  format.sample_rate      = LE_INT   (format.sample_rate);
  format.bits_per_sample  = LE_SHORT (format.bits_per_sample);

  if (format.hdr.chunk_id != COMPOSE_ID ('f', 'm', 't', ' ')
      || format.hdr.chunk_len != 0x10 || format.data_format != 0x01)
    {
      fprintf (stderr,
               "%s: Unsupported WAVE format; Try encoding the file with:\n"
               "    ffmpeg -i <file> -vn -acodec pcm_s16le out.wav\n",
               progname);

      return false;
    }

  if (format.channels <= 0)
    {
      fprintf (stderr, "%s: Cannot play a file with %d channels.\n",
               progname, format.channels);

      return false;
    }

  info->native_endian     = false;
  info->sign              = (format.bits_per_sample > 8);
  info->sample_rate       = format.sample_rate;
  info->channels          = format.channels;
  info->bits_per_sample   = format.bits_per_sample;
  info->bytes_per_sample  = ceil (format.bits_per_sample / 8.0);

  return true;
}


playable_pcm_buffer_t *
load_wave_file_into_buffer (const char *path)
{
  playable_pcm_buffer_t *buffer;
  size_t                 read_bytes;
  FILE                  *stream;

  buffer = malloc (sizeof (playable_pcm_buffer_t));
  if (buffer == NULL)
    {
      fprintf (stderr, "%s: Memory allocation in wave_prepare_buffer () failed: %s.\n",
               progname, strerror (errno));

      return NULL;
    }

  stream = fopen (path, "rb");
  if (stream == NULL)
    {
      fprintf (stderr, "%s: Failed to open `%s' for reading: %s.\n",
               progname, path, strerror (errno));

      free (buffer);
      return NULL;
    }

  if (! parse_wave_header (stream, &(buffer->info)))
    {
      fprintf (stderr, "%s: Failed to parse the WAVE header of `%s'.\n",
               progname, path);

      fclose (stream);
      free (buffer);
      return NULL;
    }

  if (! find_wave_pcm_data (stream, &(buffer->data_len), NULL))
    {
      fprintf (stderr, "%s: Failed to locate the PCM data in the WAVE file `%s'.\n",
               progname, path);

      fclose (stream);
      free (buffer);
      return NULL;
    }

  if (buffer->data_len == 0)
    {
      fprintf (stderr, "%s: The file `%s' does not contain any sound data.\n",
               progname, path);

      fclose (stream);
      free (buffer);
      return NULL;
    }

  buffer->data = malloc (buffer->data_len);
  if (buffer->data == NULL)
    {
      fprintf (stderr, "%s: Allocating the buffer for the PCM data failed: %s.\n",
               progname, strerror (errno));

      fclose (stream);
      free (buffer);
      return NULL;
    }

  read_bytes = fread (buffer->data, 1, buffer->data_len, stream);

  if (read_bytes != buffer->data_len)
    {
      if (ferror (stream))
        {
          fprintf (stderr, "%s: Reading from `%s' failed: %s.\n",
                   progname, path, strerror (errno));

          fclose (stream);
          free (buffer->data);
          free (buffer);
          return NULL;
        }

      fprintf (stderr, "%s: Warning: The PCM data in `%s' is truncated.\n",
               progname, path);

      if (read_bytes == 0)
        {
          fprintf (stderr, "%s: The file `%s' does not contain any sound data.\n",
                   progname, path);

          fclose (stream);
          free (buffer->data);
          free (buffer);
          return NULL;
        }
      else
        {
          buffer->data_len = read_bytes;
          buffer->data = realloc (buffer->data, buffer->data_len);
          if (buffer->data == NULL)
            {
              fprintf (stderr, "%s: Adjusting the PCM data buffer's size failed: %s.\n",
                       progname, strerror (errno));

              fclose (stream);
              free (buffer);
              return NULL;
            }
        }
    }

  fclose (stream);
  return buffer;
}

playable_pcm_file_t *
prepare_wave_file (const char *path)
{
  playable_pcm_file_t *file;

  file = malloc (sizeof (playable_pcm_file_t));
  if (file == NULL)
    {
      fprintf (stderr,
               "%s: Memory allocation in wave_prepare_file () failed: %s.\n",
               progname, strerror (errno));

      return NULL;
    }
  file->name = strdup (path);
  if (file->name == NULL)
    {
      fprintf (stderr,
               "%s: Memory allocation in wave_prepare_file () failed: %s.\n",
               progname, strerror (errno));

      free (file);
      return NULL;
    }

  file->stream = fopen (file->name, "rb");
  if (file->stream == NULL)
    {
      fprintf (stderr, "%s: Failed to open `%s' for reading: %s.\n",
               progname, file->name, strerror (errno));

      free (file->name);
      free (file);
      return NULL;
    }

  if (! parse_wave_header (file->stream, &(file->info)))
    {
      fprintf (stderr, "%s: Failed to parse the WAVE header of `%s'.\n",
               progname, file->name);

      fclose (file->stream);
      free (file->name);
      free (file);
      return NULL;
    }

  if (! find_wave_pcm_data (file->stream, NULL, &(file->pcm_start_pos)))
    {
      fprintf (stderr, "%s: Failed to locate the PCM data in the WAVE file `%s'.\n",
               progname, file->name);

      fclose (file->stream);
      free (file->name);
      free (file);
      return NULL;
    }

  return file;
}

#endif /* HAVE_WAVE */
