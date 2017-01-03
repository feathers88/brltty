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

#ifndef BRLTTY_INCLUDED_MNTPT_MNTTAB
#define BRLTTY_INCLUDED_MNTPT_MNTTAB

#include <sys/mnttab.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct mnttab MountEntry;
#define mountPath mnt_mountp
#define mountReference mnt_special
#define mountType mnt_fstype
#define mountOptions mnt_mntopts

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_MNTPT_MNTTAB */
