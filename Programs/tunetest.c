/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2003 by The BRLTTY Team. All rights reserved.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation.  Please see the file COPYING for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

/* tunetest.c - Test program for the tune playing library
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "options.h"
#include "tunes.h"
#include "misc.h"
#include "defaults.h"
#include "brltty.h"
#include "message.h"
#include "brl.h"

int refreshInterval = DEFAULT_REFRESH_INTERVAL;
Preferences prefs;

BEGIN_OPTION_TABLE
  {'d', "device", "device", NULL, 0,
   "Name of tune device."},
#ifdef ENABLE_MIDI_TUNES
  {'i', "instrument", "instrument", NULL, 0,
   "Name of MIDI instrument."},
#endif /* ENABLE_MIDI_TUNES */
  {'v', "level", "volume", NULL, 0,
   "Output volume."},
END_OPTION_TABLE

static const char *deviceNames[] = {"beeper", "pcm", "midi", "fm", NULL};

static unsigned int opt_tuneDevice;
#ifdef ENABLE_MIDI_TUNES
static unsigned char opt_midiInstrument = 0;
#endif /* ENABLE_MIDI_TUNES */
static short opt_outputVolume = 50;

#ifdef ENABLE_MIDI_TUNES
static unsigned char
instrumentArgument (const char *argument) {
  size_t argumentLength = strlen(argument);
  unsigned char instrument;
  for (instrument=0; instrument<midiInstrumentCount; ++instrument) {
    const char *component = midiInstrumentTable[instrument];
    size_t componentLeft = strlen(component);
    const char *word = argument;
    size_t wordLeft = argumentLength;
    {
      const char *delimiter = memchr(component, '(', componentLeft);
      if (delimiter) componentLeft = delimiter - component;
    }
    while (1) {
      while (*component == ' ') component++, componentLeft--;
      if ((componentLeft == 0) != (wordLeft == 0)) break; 
      if (!componentLeft) return instrument;
      {
        size_t wordLength = wordLeft;
        size_t componentLength = componentLeft;
        const char *delimiter;
        if ((delimiter = memchr(word, '-', wordLeft))) wordLength = delimiter - word;
        if ((delimiter = memchr(component, ' ', componentLeft))) componentLength = delimiter - component;
        if (wordLength > componentLength) break;
        if (strncasecmp(word, component, wordLength) != 0) break;
        word += wordLength; wordLeft -= wordLength;
        if (*word) word++, wordLeft--;
        component += componentLength; componentLeft -= componentLength;
      }
    }
  }
  fprintf(stderr, "%s: Invalid instrument: %s\n", programName, argument);
  exit(2);
}
#endif /* ENABLE_MIDI_TUNES */

static int
handleOption (const int option) {
  switch (option) {
    default:
      return 0;
    case 'd':
      opt_tuneDevice = wordArgument(optarg, deviceNames, "device");
      break;
#ifdef ENABLE_MIDI_TUNES
    case 'i':
      opt_midiInstrument = instrumentArgument(optarg);
      break;
#endif /* ENABLE_MIDI_TUNES */
    case 'v':
      opt_outputVolume = integerArgument(optarg, 0, 100, "level");
      break;
  }
  return 1;
}

int
main (int argc, char *argv[]) {
  opt_tuneDevice = getDefaultTuneDevice();
  processOptions(optionTable, optionCount, handleOption,
                 &argc, &argv, "{note duration} ...");
  if (!argc) {
    fprintf(stderr, "%s: Missing tune.\n", programName);
  } else if (argc % 2) {
    fprintf(stderr, "%s: Missing duration.\n", programName);
  } else {
    unsigned int count = argc / 2;
    TuneElement *elements = (TuneElement *)mallocWrapper((sizeof(*elements) * count) + 1);
    TuneElement *element = elements;

    while (argc) {
      short note = integerArgument(*argv++, 1, 127, "note");
      short duration = integerArgument(*argv++, 1, 255, "duration");
      argc -= 2;
      *(element++) = TUNE_NOTE(duration, note);
    }
    *element = TUNE_STOP();

    if (!setTuneDevice(opt_tuneDevice)) {
      fprintf(stderr, "%s: Unsupported tune device: %s\n", programName, deviceNames[opt_tuneDevice]);
      exit(3);
    }

    memset(&prefs, 0, sizeof(prefs));
    prefs.alertMessages = 0;
    prefs.alertDots = 0;
    prefs.alertTunes = 1;
    switch (opt_tuneDevice) {
      default:
        break;
      case tdPcm:
        prefs.pcmVolume = opt_outputVolume;
        break;
      case tdMidi:
        prefs.midiVolume = opt_outputVolume;
        break;
      case tdFm:
        prefs.fmVolume = opt_outputVolume;
        break;
    }
#ifdef ENABLE_MIDI_TUNES
    prefs.midiInstrument = opt_midiInstrument;
#endif /* ENABLE_MIDI_TUNES */
    {
      TuneDefinition tune = {NULL, 0, elements};
      playTune(&tune);
      closeTuneDevice(1);
    }

    free(elements);
    return 0;
  }
  return 2;
}

void
message (const char *text, short flags) {
}
void
showDotPattern (unsigned char dots, unsigned char duration) {
}
