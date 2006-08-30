/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2006 by The BRLTTY Developers.
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

#include "prologue.h"

#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "tbl.h"
#include "tbl_internal.h"

static int
testCharacter (unsigned char character, unsigned char *index, const unsigned char *characters, unsigned char count) {
  const unsigned char *address = memchr(characters, character, count);
  if (!address) return 0;
  *index = address - characters;
  return 1;
}

static int
testDotNumber (unsigned char character, unsigned char *index) {
  return testCharacter(character, index, tblDotNumbers, sizeof(tblDotNumbers));
}

static int
testHexadecimalDigit (unsigned char character, unsigned char *index) {
  static const unsigned char digits[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
  };
  return testCharacter(character, index, digits, sizeof(digits));
}

static int
getBit (TblInputData *input, unsigned char *set, unsigned char *mask) {
  const unsigned char *location = input->location;
  int length = tblFindSpace(input) - location;

  if (!*location) {
    tblReportError(input, "missing bit state");
    return 0;
  }

  *mask = 0;
  if (tblTestWord(location, length, "on")) {
    *set = 1;
  } else if (tblTestWord(location, length, "off")) {
    *set = 0;
  } else {
    static const unsigned char operators[] = {'=', '~'};
    if (testCharacter(*input->location, set, operators, sizeof(operators))) {
      typedef struct {
        const char *name;
        unsigned char mask;
      } BitEntry;
      static const BitEntry bitTable[] = {
        {"fg-blue"  , 0X01},
        {"fg-green" , 0X02},
        {"fg-red"   , 0X04},
        {"fg-bright", 0X08},
        {"bg-blue"  , 0X10},
        {"bg-green" , 0X20},
        {"bg-red"   , 0X40},
        {"blink"    , 0X80},
        {"bit01"    , 0X01},
        {"bit02"    , 0X02},
        {"bit04"    , 0X04},
        {"bit08"    , 0X08},
        {"bit10"    , 0X10},
        {"bit20"    , 0X20},
        {"bit40"    , 0X40},
        {"bit80"    , 0X80},
        {NULL       , 0X00},
      };
      const BitEntry *bit = bitTable;

      ++location, --length;
      while (bit->name) {
        if (tblTestWord(location, length, bit->name)) {
          *mask = bit->mask;
          break;
        }
        ++bit;
      }
    }

    if (!*mask) {
      tblReportError(input, "invalid bit state");
      return 0;
    }
  }

  input->location = location + length;
  return 1;
}

static int
getByte (TblInputData *input, unsigned char *byte) {
  const unsigned char *location = input->location;
  switch (*location++) {
    default:
      *byte = *(location - 1);
      break;

    case '\\':
      switch (*location++) {
        default:
          tblReportError(input, "invalid special byte");
          return 0;

        case 'X':
        case 'x': {
          int count = 2;
          *byte = 0;
          while (count--) {
            unsigned char digit;
            if (!testHexadecimalDigit(tolower(*location++), &digit)) {
              tblReportError(input, "invalid hexadecimal byte");
              return 0;
            }
            *byte = (*byte << 4) | digit;
          }
          break;
        }

        case '\\':
        case '#':
          *byte = *(location - 1);
          break;

        case 's':
          *byte = ' ';
          break;
      }
      break;

    case 0:
      tblReportError(input, "missing byte");
      return 0;
  }

  if (*location && !isspace(*location)) {
    tblReportError(input, "invalid byte");
    return 0;
  }

  input->location = location;
  return 1;
}

static int
getCell (TblInputData *input, unsigned char *cell) {
  const unsigned char *location = input->location;
  int none = 0;
  unsigned char enclosed = (*location == '(')? ')':
                           0;
  *cell = 0;

  if (enclosed) {
    ++location;
  } else if (!*location) {
    tblReportError(input, "missing cell");
    return 0;
  } else if (memchr(tblNoDots, *location, tblNoDotsSize)) {
    ++location;
    none = 1;
  }

  while (*location) {
    unsigned char character = *location++;
    int space = isspace(character);

    if (enclosed) {
      if (character == enclosed) {
        enclosed = 0;
        break;
      }
      if (space) continue;
    } else if (space) {
      --location;
      break;
    }

    {
      unsigned char dot;
      if (none || !testDotNumber(character, &dot)) {
        tblReportError(input, "invalid dot number");
        return 0;
      }

      {
        unsigned char bit = tblDotBit(dot);
        if (*cell & bit) {
          tblReportError(input, "duplicate dot number");
          return 0;
        }
        *cell |= bit;
      }
    }
  }

  if (enclosed) {
    tblReportError(input, "incomplete cell");
    return 0;
  }

  input->location = location;
  return 1;
}

static int
getDot (TblInputData *input, unsigned char *dot) {
  if (!*input->location) {
    tblReportError(input, "missing dot number");
    return 0;
  }

  if (!testDotNumber(*input->location, dot) ||
      ((tblFindSpace(input) - input->location) != 1)) {
    tblReportError(input, "invalid dot number");
    return 0;
  }

  ++input->location;
  return 1;
}

static void
processByteDirective (TblInputData *input) {
  unsigned char index;
  tblSkipSpace(input);
  if (getByte(input, &index)) {
    unsigned char cell;
    tblSkipSpace(input);
    if (getCell(input, &cell)) {
      tblSkipSpace(input);
      if (tblIsEndOfLine(input)) {
        tblSetByte(input, index, cell);
      }
    }
  }
}

static void
processDotDirective (TblInputData *input) {
  unsigned char index;
  tblSkipSpace(input);
  if (getDot(input, &index)) {
    unsigned char set;
    unsigned char mask;
    tblSkipSpace(input);
    if (getBit(input, &set, &mask)) {
      tblSkipSpace(input);
      if (tblIsEndOfLine(input)) {
        input->masks[index] = mask;
        mask = tblDotBit(index);
        if (set) {
          input->undefined |= mask;
        } else {
          input->undefined &= ~mask;
        }
      }
    }
  }
}

static int
processTableLine (TblInputData *input) {
  tblSkipSpace(input);
  if (!tblIsEndOfLine(input)) {
    typedef struct {
      const char *name;
      void (*handler) (TblInputData *input);
    } DirectiveEntry;
    static const DirectiveEntry directiveTable[] = {
      {"byte", processByteDirective},
      {"dot" , processDotDirective },
      {NULL  , processByteDirective}
    };
    const DirectiveEntry *directive = directiveTable;
    int length = tblFindSpace(input) - input->location;

    while (directive->name) {
      if (tblTestWord(input->location, length, directive->name)) {
        input->location += length;
        break;
      }
      ++directive;
    }

    directive->handler(input);
  }

  return 1;
}

int
tblLoad_Native (const char *path, FILE *file, TranslationTable table, int options) {
  return tblProcessLines(path, file, table, processTableLine, options);
}
