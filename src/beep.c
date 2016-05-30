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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "pcm.h"
#include "beep.h"
#include <math.h>

#ifdef HAVE_SOUND

#define SAMPLE_RATE 44100

playable_pcm_buffer_t *
generate_sine_beep (unsigned int volume, unsigned int frequency,
                    unsigned int duration)
{
  playable_pcm_buffer_t *buffer;
  int16_t               *samples;
  uint32_t               samples_count;
  unsigned int           period_counter;
  unsigned int           period_length;
  unsigned int           iter;


  buffer = malloc (sizeof (playable_pcm_buffer_t));
  if (buffer == NULL)
    {
      fprintf (stderr, "%s: generate_sine_beep (): Memory allocation "
                       "failed: %s.\n",
               progname, strerror (errno));

      return NULL;
    }

  buffer->info.native_endian     = true;
  buffer->info.sign              = true;
  buffer->info.sample_rate       = SAMPLE_RATE;
  buffer->info.channels          = 1;
  buffer->info.bytes_per_sample  = 2;
  buffer->info.bits_per_sample   = 16;

  samples_count = (SAMPLE_RATE * duration) / 1000;
  buffer->data_len = samples_count * sizeof (int16_t);

  buffer->data = malloc (buffer->data_len);
  if (buffer->data == NULL)
    {
      fprintf (stderr, "%s: generate_sine_beep (): Failed to allocate a buffer "
                       "for the PCM data: %s.\n",
               progname, strerror (errno));

      free (buffer);
      return NULL;
    }
  samples = (int16_t *)(buffer->data);

  period_length  = SAMPLE_RATE / frequency;
  period_counter = period_length;
  for (iter = 0; iter < samples_count; iter++)
    {
      if (period_counter == period_length)
        period_counter = 0;
      else
        period_counter++;

      samples[iter] = INT16_MAX
                      * sin (2 * M_PI * period_counter / period_length)
                      * (volume / 100.0);
    }

  return buffer;
}

/**
 * This is sin(x) + sin(3x)/sqrt(3) + ... + sin(9x)/sqrt(9).
 *
 * After a little experimentation, this seems to be a nice mellow beep with
 * neither the piercing quality of a pure sine wave nor the loud harshness
 * of a square wave.
 */
static double
complex_wave (double param)
{
  return (sin (param)
          + (sin (3 * param) * 0.192450089729875254836382926833)
          + (sin (5 * param) * 0.089442719099991587856366946749)
          + (sin (7 * param) * 0.053994924715603889602073790890)
          + (sin (9 * param) * 0.037037037037037037037037037037));
}

playable_pcm_buffer_t *
generate_complex_beep (unsigned int volume, unsigned int frequency,
                       unsigned int duration)
{
  playable_pcm_buffer_t *buffer;
  int16_t               *samples;
  uint32_t               samples_count;
  unsigned int           period_counter;
  unsigned int           period_length;
  unsigned int           iter;


  buffer = malloc (sizeof (playable_pcm_buffer_t));
  if (buffer == NULL)
    {
      fprintf (stderr, "%s: generate_complex_beep (): Memory allocation "
                       "failed: %s.\n",
               progname, strerror (errno));

      return NULL;
    }

  buffer->info.native_endian     = true;
  buffer->info.sign              = true;
  buffer->info.sample_rate       = SAMPLE_RATE;
  buffer->info.channels          = 1;
  buffer->info.bytes_per_sample  = 2;
  buffer->info.bits_per_sample   = 16;

  samples_count = (SAMPLE_RATE * duration) / 1000;
  buffer->data_len = samples_count * sizeof (int16_t);

  buffer->data = malloc (buffer->data_len);
  if (buffer->data == NULL)
    {
      fprintf (stderr, "%s: generate_complex_beep (): Failed to allocate a "
                       "buffer for the PCM data: %s.\n",
               progname, strerror (errno));

      free (buffer);
      return NULL;
    }
  samples = (int16_t *)(buffer->data);

  period_length  = SAMPLE_RATE / frequency;
  period_counter = period_length;
  for (iter = 0; iter < samples_count; iter++)
    {
      if (period_counter == period_length)
        period_counter = 0;
      else
        period_counter++;

      samples[iter] = INT16_MAX
                      * complex_wave (2 * M_PI * period_counter / period_length)
                      * (volume / 100.0);
    }

  return buffer;
}

playable_pcm_buffer_t *
generate_square_beep (unsigned int volume, unsigned int frequency,
                      unsigned int duration)
{
  playable_pcm_buffer_t *buffer;
  uint8_t               *samples;
  uint32_t               samples_count;
  uint8_t                current_sample;
  unsigned int           halfperiod_counter;
  unsigned int           halfperiod_length;
  unsigned int           iter;


  buffer = malloc (sizeof (playable_pcm_buffer_t));
  if (buffer == NULL)
    {
      fprintf (stderr, "%s: generate_square_beep (): Memory allocation "
                       "failed: %s.\n",
               progname, strerror (errno));

      return NULL;
    }

  buffer->info.native_endian     = true;
  buffer->info.sign              = false;
  buffer->info.sample_rate       = SAMPLE_RATE;
  buffer->info.channels          = 1;
  buffer->info.bytes_per_sample  = 1;
  buffer->info.bits_per_sample   = 8;

  samples_count = (SAMPLE_RATE * duration) / 1000;
  buffer->data_len =  samples_count * sizeof (uint8_t);

  buffer->data = malloc (buffer->data_len);
  if (buffer->data == NULL)
    {
      fprintf (stderr, "%s: generate_square_beep (): Failed to allocate a "
                       "buffer for the PCM data: %s.\n",
               progname, strerror (errno));

      free (buffer);
      return NULL;
    }
  samples = (uint8_t *)(buffer->data);

  current_sample     = 0;
  halfperiod_counter = 0;
  halfperiod_length  = (SAMPLE_RATE / frequency) / 2;
  for (iter = 0; iter < samples_count; iter++)
    {
      if (halfperiod_counter == 0)
        {
          if (current_sample == 0)
            current_sample = 0xff;
          else
            current_sample = 0;

          halfperiod_counter = halfperiod_length;
        }
      else
        halfperiod_counter--;

      samples[iter] = current_sample * (volume / 100.0);
    }

  return buffer;
}

#endif /* HAVE_SOUND */


bool
perform_beep (beep_descriptor_t *beep)
{
  switch (beep->type)
    {
#ifdef HAVE_SOUND
      case BEEP_TYPE_BUFFER:
        return play_pcm_buffer (beep->buffer);
        break;

      case BEEP_TYPE_FILE:
        return play_pcm_file (beep->file);
        break;
#endif
      case BEEP_TYPE_COMMAND:
        system (beep->command);
        return true;
        break;

      default:
        fprintf (stderr, "%s: perform_beep (): bad `type' field in beep "
                         "descriptor, this is a bug.\n",
                 progname);
        exit (1);
        break;
    }
}

void
free_beep_desc (beep_descriptor_t *beep)
{
  if (beep == NULL)
    return;

  switch (beep->type)
    {
#ifdef HAVE_SOUND
      case BEEP_TYPE_BUFFER:
        if (beep->buffer != NULL)
          free_pcm_buffer (beep->buffer);
        break;

      case BEEP_TYPE_FILE:
        if (beep->file != NULL)
          close_pcm_file (beep->file);
        break;
#endif
      case BEEP_TYPE_COMMAND:
        if (beep->command != NULL)
          free (beep->command);
        break;
    }

  free (beep);
}
