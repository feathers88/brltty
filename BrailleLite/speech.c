/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2001 by The BRLTTY Team. All rights reserved.
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

/* BrailleLite/speech.c - Speech library
 * For Blazie Engineering's Braille Lite 18/40
 * Maintained by Nikhil Nair <nn201@cus.cam.ac.uk>
 */

#define SPEECH_C 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "speech.h"		/* for BLite speech definitions */
#include "../spk.h"
#include "../spk_driver.h"
#include "../misc.h"

/* This is shared with BrailleLite/brl.c: */
extern int blite_fd;


#if 0
/* charset conversion table from iso latin-1 == iso 8859-1 to cp437==ibmpc
 * for chars >=128. 
 */
static unsigned char latin2cp437[128] =
  {199, 252, 233, 226, 228, 224, 229, 231,
   234, 235, 232, 239, 238, 236, 196, 197,
   201, 181, 198, 244, 247, 242, 251, 249,
   223, 214, 220, 243, 183, 209, 158, 159,
   255, 173, 155, 156, 177, 157, 188, 21,
   191, 169, 166, 174, 170, 237, 189, 187,
   248, 241, 253, 179, 180, 230, 20, 250,
   184, 185, 167, 175, 172, 171, 190, 168,
   192, 193, 194, 195, 142, 143, 146, 128,
   200, 144, 202, 203, 204, 205, 206, 207,
   208, 165, 210, 211, 212, 213, 153, 215,
   216, 217, 218, 219, 154, 221, 222, 225,
   133, 160, 131, 227, 132, 134, 145, 135,
   138, 130, 136, 137, 141, 161, 140, 139,
   240, 164, 149, 162, 147, 245, 148, 246,
   176, 151, 163, 150, 129, 178, 254, 152};
#endif


static void
identspk (void)
{
  LogAndStderr(LOG_NOTICE, "Using the Braille Lite's built-in speech.");
}


static void
initspk (char **parameters)
{
}


static void
say (unsigned char *buffer, int len)
{
  static unsigned char pre_speech[] = { PRE_SPEECH };
  static unsigned char post_speech[] = { POST_SPEECH };

  /* Keep it simple for now: */
  if (pre_speech[0])
    write (blite_fd, pre_speech + 1, pre_speech[0]);
  write (blite_fd, buffer, len);
  if (post_speech[0])
    write (blite_fd, post_speech + 1, post_speech[0]);

#if 0
  unsigned char c;
  int i;

  if (pre_speech[0])
    {
      memcpy (rawdata, pre_speech + 1, pre_speech[0]);
      write (brl_fd, rawdata, pre_speech[0]);
    }
  for (i = 0; i < len; i++)
    {
      c = buffer[i];
      if (c >= 128) c = latin2cp437[c];
      if (c < 33)	/* space or control character */
	{
	  rawdata[0] = ' ';
	  write (brl_fd, rawdata, 1);
	}
      else if (c > MAX_TRANS)
	write (brl_fd, &c, 1);
      else
	{
	  memcpy (rawdata, vocab[c - 33], strlen (vocab[c - 33]));
	  write (brl_fd, rawdata, strlen (vocab[c - 33]));
	}
    }
  if (post_speech[0])
    {
      memcpy (rawdata, post_speech + 1, post_speech[0]);
      write (brl_fd, rawdata, post_speech[0]);
    }
#endif
}


static void
mutespk (void)
{
  unsigned char mute_seq[] = {MUTE_SEQ };

  write (blite_fd, mute_seq + 1, mute_seq[0]);
}


static void
closespk (void)
{
}