/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2017 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://brltty.com/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

#ifndef BRLTTY_INCLUDED_MESSAGE
#define BRLTTY_INCLUDED_MESSAGE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
  MSG_SILENT  = 0X1, /* don't speak the message */
  MSG_WAITKEY = 0X2, /* wait for a key press before scrolling or dismissing the message */
  MSG_NODELAY = 0X4, /* don't wait */
  MSG_SYNC    = 0X8  /* run synchronously */
} MessageOptions;

extern int message (const char *mode, const char *text, MessageOptions options);

extern int messageHoldTimeout;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_MESSAGE */
