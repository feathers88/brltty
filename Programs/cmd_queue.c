/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2013 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

#include "prologue.h"

#include <stdio.h>
#include <string.h>

#include "log.h"
#include "cmd_queue.h"
#include "queue.h"
#include "async_alarm.h"
#include "prefs.h"
#include "api_control.h"
#include "ktbdefs.h"
#include "scr.h"
#include "brltty.h"

CommandExecuter *currentCommandExecuter = NULL;

typedef struct CommandHandlerLevelStruct CommandHandlerLevel;
static CommandHandlerLevel *commandHandlerStack = NULL;

struct CommandHandlerLevelStruct {
  CommandHandlerLevel *previousLevel;
  CommandHandler *handleCommand;
  void *handlerData;
  KeyTableCommandContext commandContext;
};

int
pushCommandHandler (KeyTableCommandContext context, CommandHandler *handler, void *data) {
  CommandHandlerLevel *chl;

  if ((chl = malloc(sizeof(*chl)))) {
    memset(chl, 0, sizeof(*chl));
    chl->handleCommand = handler;
    chl->handlerData = data;
    chl->commandContext = context;

    chl->previousLevel = commandHandlerStack;
    commandHandlerStack = chl;
    return 1;
  } else {
    logMallocError();
  }

  return 0;
}

int
popCommandHandler (void) {
  CommandHandlerLevel *chl = commandHandlerStack;
  if (!chl) return 0;

  commandHandlerStack = chl->previousLevel;
  free(chl);
  return 1;
}

KeyTableCommandContext
getCurrentCommandContext (void) {
  const CommandHandlerLevel *chl = commandHandlerStack;
  KeyTableCommandContext context = chl? chl->commandContext: KTB_CTX_DEFAULT;

  if (context == KTB_CTX_DEFAULT) context = getScreenCommandContext();
  return context;
}

int
handleCommand (int command) {
  {
    int real = command;

    if (prefs.skipIdenticalLines) {
      switch (command & BRL_MSK_CMD) {
        case BRL_CMD_LNUP:
          real = BRL_CMD_PRDIFLN;
          break;

        case BRL_CMD_LNDN:
          real = BRL_CMD_NXDIFLN;
          break;

        case BRL_CMD_PRDIFLN:
          real = BRL_CMD_LNUP;
          break;

        case BRL_CMD_NXDIFLN:
          real = BRL_CMD_LNDN;
          break;

        default:
          break;
      }
    }

    if (real == command) {
      logCommand(command);
    } else {
      real |= (command & ~BRL_MSK_CMD);
      logTransformedCommand(command, real);
      command = real;
    }
  }

  {
    const CommandHandlerLevel *chl = commandHandlerStack;

    while (chl) {
      if (chl->handleCommand(command, chl->handlerData)) return 1;
      chl = chl->previousLevel;
    }
  }

  logMessage(LOG_WARNING, "%s: %04X", gettext("unhandled command"), command);
  return 0;
}

typedef struct {
  int command;
} CommandQueueItem;

static void
deallocateCommandQueueItem (void *item, void *data) {
  CommandQueueItem *cmd = item;

  free(cmd);
}

static Queue *
createCommandQueue (void *data) {
  return newQueue(deallocateCommandQueueItem, NULL);
}

static Queue *
getCommandQueue (int create) {
  static Queue *commands = NULL;

  return getProgramQueue(&commands, "command-queue", create,
                         createCommandQueue, NULL);
}

static int
dequeueCommand (Queue *queue) {
  CommandQueueItem *item;

  while ((item = dequeueItem(queue))) {
    int command = item->command;
    free(item);

#ifdef ENABLE_API
    if (apiStarted) {
      if ((command = api_handleCommand(command)) == EOF) {
        continue;
      }
    }
#endif /* ENABLE_API */

    return command;
  }

  return EOF;
}

static void setCommandAlarm (void *data);

static void
handleCommandAlarm (const AsyncAlarmResult *result) {
  Queue *queue = getCommandQueue(0);

  if (queue) {
    int command = dequeueCommand(queue);

    if (command != EOF) {
      CommandExecuter *executeCommand = currentCommandExecuter;

      if (!executeCommand) executeCommand = handleCommand;
      executeCommand(command);
    }

    if (getQueueSize(queue) > 0) setCommandAlarm(result->data);
  }
}

static void
setCommandAlarm (void *data) {
  asyncSetAlarmIn(NULL, 0, handleCommandAlarm, data);
}

int
enqueueCommand (int command) {
  if (command != EOF) {
    Queue *queue = getCommandQueue(1);

    if (queue) {
      CommandQueueItem *item = malloc(sizeof(CommandQueueItem));

      if (item) {
        item->command = command;

        if (enqueueItem(queue, item)) {
          if (getQueueSize(queue) == 1) setCommandAlarm(NULL);
          return 1;
        }

        free(item);
      }
    }
  }

  return 0;
}
