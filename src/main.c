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
#include "pcm.h"
#include "beep.h"
#include "wave.h"

#include <argp.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#include <X11/XKBlib.h>


#ifdef HAVE_SOUND
# define DEFAULT_OP_MODE       GENERATED_BEEP_OP_MODE
# define DEFAULT_GEN_BEEP_TYPE SINE_WAVE_BEEP
#else
# define DEFAULT_OP_MODE       COMMAND_OP_MODE
#endif

const char *progname                 = PACKAGE_NAME;
const char *argp_program_version     = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

static char doc[] = PACKAGE_STRING " -- the new X bell daemon.\v"
#ifdef HAVE_SOUND
#ifdef HAVE_WAVE
                    "The program has three modes of operation: playing "
                    "generated beeps, beeps from a sound file, or beeping by "
                    "running an external command.\n\n"

                    "The default mode is playing a generated sine wave beep, "
                    "and can be changed by the --sine, --square, --wave-file "
                    "and --command options.\n\n"

                    "The --volume, --frequency and --duration options only "
                    "apply to generated beeps, and are ignored by other modes.";

#else /* ! HAVE_WAVE */

                    "The program has two modes of operation: playing generated "
                    "beeps, or beeping by running an external command.\n\n"

                    "The default mode is playing a generated sine wave beep, "
                    "and can be changed by the --sine, --square and --command "
                    "options.\n\n"

                    "The --volume, --frequency and --duration options only "
                    "apply to generated beeps, and are ignored by other modes.";

#endif /* ! HAVE_WAVE */
#else /* ! HAVE_SOUND */

                    "Sound support was disabled at compile-time, therefore "
                    "you may only use " PACKAGE_NAME " to execute a command "
                    "when the bell is rung (the --command option).";

#endif /* ! HAVE_SOUND */


static struct argp_option options[] =
{
  {"background", 'b', 0,      0,  "run in background" },
  {"keep-abell", 'D', 0,      0,  "don't disable audio bell on startup" },
  {"throttle",   't', "N",    0,
   "Interval (ms) during which subsequent bells are throttled" },
  {"test-bell",  'T', 0,      0,  "perform a bell sound as a test on startup" },

#ifdef HAVE_SOUND

  {"square",     'q', 0,      0,  "use a square wave beep for the bell" },
  {"sine",       'i', 0,      0,  "use a sine wave beep for the bell" },
  {"duration",   'd', "DUR",  0,  "beep duration (ms)" },
  {"frequency",  'F', "FREQ", 0,  "beep frequency (hz)" },
  {"volume",     'v', "VOL",  0,  "beep volume (0 -- 100)" },

#ifdef HAVE_WAVE
  {"wave-file",  'f', "FILE", 0,  "use the given wave file for the bell" },
  {"cache",      'c', 0,      0,  "cache audio file in memory" },
#endif

#endif /* HAVE_SOUND */

  {"command",    'e', "CMD",  0,  "run the given command for the bell" },
  { 0 }
};

struct prog_args
{
  bool             background;
  bool             disable_abell;
  bool             test_bell;
  unsigned int     op_mode;
  unsigned int     gen_beep_type;
  unsigned int     gen_beep_vol;
  unsigned int     gen_beep_dur;
  unsigned int     gen_beep_freq;
  unsigned int     throttle;
  const    char   *wave_path;
  bool             cache_file;
  const    char   *command;
};
enum
{
  GENERATED_BEEP_OP_MODE,
  WAVE_FILE_OP_MODE,
  COMMAND_OP_MODE
};
enum
{
  SQUARE_WAVE_BEEP,
  SINE_WAVE_BEEP
};
typedef struct prog_args prog_args_t;

static void
prog_args_set_default (prog_args_t *args, Display *display)
{
  XKeyboardState kbd_state;

  args->background      = false;
  args->disable_abell   = true;
  args->test_bell       = false;
  args->op_mode         = DEFAULT_OP_MODE;
#ifdef HAVE_SOUND
  args->gen_beep_type   = DEFAULT_GEN_BEEP_TYPE;

  /* Get the default bell parameters from X. */
  if (XGetKeyboardControl (display, &kbd_state))
    {
      args->gen_beep_vol  = kbd_state.bell_percent;
      args->gen_beep_dur  = kbd_state.bell_pitch;
      args->gen_beep_freq = kbd_state.bell_duration;

      if (args->gen_beep_vol == 0)
        args->gen_beep_vol = 80;
    }
#endif
  args->throttle        = 0;
  args->wave_path       = NULL;
  args->cache_file      = false;
  args->command         = NULL;
}

static error_t
parse_option (int key, char *arg, struct argp_state *state)
{
  prog_args_t *args = state->input;
  char        *arg_endptr;

  switch (key)
    {
      case 'b':
        args->background = true;
        break;
      case 'D':
        args->disable_abell = false;
        break;
      case 'T':
        args->test_bell = true;
        break;
      case 't':
        args->throttle = strtoul (arg, &arg_endptr, 10);
        if (arg_endptr == NULL || arg_endptr[0] != '\0')
          argp_error (state, "The --throttle option expects an integer argument.");
        break;
#ifdef HAVE_SOUND
      case 'i':
        args->op_mode       = GENERATED_BEEP_OP_MODE;
        args->gen_beep_type = SINE_WAVE_BEEP;
        break;
      case 'q':
        args->op_mode       = GENERATED_BEEP_OP_MODE;
        args->gen_beep_type = SQUARE_WAVE_BEEP;
        break;
      case 'd':
        args->gen_beep_dur = strtoul (arg, &arg_endptr, 10);
        if (arg_endptr == NULL || arg_endptr[0] != '\0')
          argp_error (state, "The --duration option expects an integer argument.");
        break;
      case 'F':
        args->gen_beep_freq = strtoul (arg, &arg_endptr, 10);
        if (arg_endptr == NULL || arg_endptr[0] != '\0')
          argp_error (state, "The --frequency option expects an integer argument.");
        break;
      case 'v':
        args->gen_beep_vol = strtoul (arg, &arg_endptr, 10);
        if (arg_endptr == NULL || arg_endptr[0] != '\0'
            || args->gen_beep_vol > 100)
          argp_error (state, "The --volume option expects an integer argument between 0 and 100.");
        break;
#ifdef HAVE_WAVE
      case 'f':
        args->op_mode    = WAVE_FILE_OP_MODE;
        args->wave_path  = arg;
        break;
      case 'c':
        args->cache_file = true;
        break;
#endif
#endif /* HAVE_SOUND */
      case 'e':
        args->op_mode    = COMMAND_OP_MODE;
        args->command    = arg;
        break;

      default:
        return ARGP_ERR_UNKNOWN;
        break;
    }

  return 0;
}

static struct argp argp = { options, parse_option, 0, doc };

beep_descriptor_t *prepare_beep (prog_args_t *args)
{
  beep_descriptor_t *beep;

  beep = malloc (sizeof (beep_descriptor_t));
  if (beep == NULL)
    {
      fprintf (stderr, "%s: Allocating memory for the beep descriptor failed: %s.\n",
               progname, strerror (errno));

      return NULL;
    }

  switch (args->op_mode)
    {
      case COMMAND_OP_MODE:
        if (args->command == NULL)
          {
            fprintf (stderr, "%s: No external bell command specified.\n",
                     progname);

            free (beep);
            return NULL;
          }
        beep->type = BEEP_TYPE_COMMAND;
        beep->command = strdup (args->command);
        if (beep->command == NULL)
          {
            fprintf (stderr, "%s: Failed to copy the command string: %s.\n",
                     progname, strerror (errno));

            free (beep);
            return NULL;
          }
        break;
#ifdef HAVE_SOUND
      case GENERATED_BEEP_OP_MODE:
        beep->type = BEEP_TYPE_BUFFER;
        switch (args->gen_beep_type)
          {
            case SINE_WAVE_BEEP:
              beep->buffer = generate_sine_beep (args->gen_beep_vol,
                                                 args->gen_beep_freq,
                                                 args->gen_beep_dur);
              break;
            case SQUARE_WAVE_BEEP:
              beep->buffer = generate_square_beep (args->gen_beep_vol,
                                                   args->gen_beep_freq,
                                                   args->gen_beep_dur);
              break;
            default:
              fprintf (stderr, "%s: Invalid beep type.\n", progname);

              free (beep);
              return NULL;
              break;
          }
        if (beep->buffer == NULL)
          {
            fprintf (stderr, "%s: Failed to generate the beep.\n", progname);

            free (beep);
            return NULL;
          }
        break;
#ifdef HAVE_WAVE
      case WAVE_FILE_OP_MODE:
        if (args->cache_file)
          {
            beep->type = BEEP_TYPE_BUFFER;
            beep->buffer = load_wave_file_into_buffer (args->wave_path);
            if (beep->buffer == NULL)
              {
                fprintf (stderr, "%s: Failed to load `%s' into memory.\n",
                         progname, args->wave_path);

                free (beep);
                return NULL;
              }
          }
        else
          {
            beep->type = BEEP_TYPE_FILE;
            beep->file = prepare_wave_file (args->wave_path);
            if (beep->file == NULL)
              {
                fprintf (stderr, "%s: Failed to prepare `%s' for playing.\n",
                         progname, args->wave_path);

                free (beep);
                return NULL;
              }
          }
        break;
#endif /* HAVE_WAVE */
#endif /* HAVE_SOUND */
    default:
      fprintf (stderr, "%s: Invalid mode of operation.\n", progname);

      free (beep);
      return NULL;
      break;
    }

  return beep;
}

static void
bell_daemon (Display *display, int event_code, beep_descriptor_t *beep,
             unsigned int throttle)
{
  XkbEvent           event;
  struct timeval     now;
  struct timeval     last_bell = {0,0};
  unsigned long      time_since_last_bell = 0;

  while (true)
    {
      XNextEvent (display, &event.core);
      if (event.type == event_code)
        {
          if (throttle > 0)
            {
              do
                {
                  gettimeofday (&now, NULL);
                  time_since_last_bell = 1000 * (now.tv_sec - last_bell.tv_sec)
                                         + ((now.tv_usec - last_bell.tv_usec)
                                          / 1000);
                }
              while (last_bell.tv_sec != 0
                     && time_since_last_bell <= throttle);

              if (! perform_beep (beep))
                fprintf (stderr, "%s: Warning: Performing a beep failed.\n",
                         progname);

              last_bell = now;
            }
          else
            {
              if (! perform_beep (beep))
                fprintf (stderr, "%s: Warning: Performing a beep failed.\n",
                         progname);
            }
        }
    }
}

int
main (int argc, char **argv)
{
  prog_args_t        args;
  beep_descriptor_t *beep;

  struct sigaction   action;
  Display           *display;
  int                major;
  int                minor;
  int                xkb_event_code;
  int                xkb_error;

  /**
   * Set signal masks. Dead children are not waitpid()'d, so make sure they
   * don't become zombies.
   */
  action.sa_flags = SA_NOCLDWAIT;
  action.sa_handler = SIG_IGN;
  sigemptyset (&action.sa_mask);
  sigaction (SIGCHLD, &action, NULL);

  major = XkbMajorVersion;
  minor = XkbMinorVersion;
  display = XkbOpenDisplay (NULL, &xkb_event_code, NULL, &major, &minor,
                            &xkb_error);
  if (display == NULL)
    {
      switch (xkb_error)
        {
          case XkbOD_BadLibraryVersion:
            fprintf (stderr, "%s: Xkb version %d.%02d is required (got library "
                             "%d.%02d).\n",
                     progname, XkbMajorVersion, XkbMinorVersion, major, minor);
            break;
          case XkbOD_ConnectionRefused:
            fprintf (stderr, "%s: Cannot open the display: connection refused.\n",
                     progname);
            break;
          case XkbOD_NonXkbServer:
            fprintf (stderr, "%s: XKB extension not present.\n", progname);
            break;
          case XkbOD_BadServerVersion:
            fprintf (stderr, "%s: Xkb version %d.%02d is required (got server "
                             "%d.%02d).\n",
                     progname, XkbMajorVersion, XkbMinorVersion, major, minor);
            break;
          default:
            fprintf (stderr, "%s: Unknown error %d from XkbOpenDisplay",
                     progname, xkb_error);
            break;
        }

      return 1;
    }

  prog_args_set_default (&args, display);
  argp_parse (&argp, argc, argv, 0, 0, &args);

  if (args.background)
    {
      switch (fork ())
        {
          case -1:
            fprintf (stderr, "%s: Forking the process failed: %s.\n",
                     progname, strerror (errno));
            return 1;

          case 0: /* Child process. */
            break;

          default: /* Parent process. */
            return 0;
        }
    }

  beep = prepare_beep (&args);
  if (beep == NULL)
    {
      fprintf (stderr, "%s: Preparing the beeping mechanism failed.\n",
               progname);
      return 1;
    }
  if (args.test_bell)
    {
      if (! perform_beep (beep))
        fprintf (stderr, "%s: Warning: Failed to perform the test beep.\n",
                 progname);
    }

  XkbSelectEvents (display, XkbUseCoreKbd, XkbBellNotifyMask, XkbBellNotifyMask);
  if (args.disable_abell)
    {
      if (! XkbChangeEnabledControls (display, XkbUseCoreKbd,
                                      XkbAudibleBellMask, 0))
        {
          fprintf (stderr, "%s: Couldn't disable the system's audible bell.\n",
                   progname);
          return 1;
        }
    }

  bell_daemon (display, xkb_event_code, beep, args.throttle);

  XCloseDisplay (display);
  free_beep_desc (beep);

  return 0;
}
