/**
 * parser.c
 * Parsers for use in timeflow
 * 
 * Author : github.com/prakashniroula-dev
 */

#include <stdbool.h>
#include <parser.h>
#include <types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static inline int _imax(int a, int b) {
  return a > b ? a: b;
}

static inline int _imin(int a, int b) {
  return a < b ? a: b;
}

/* Development phase */

bool parse_int(const char* str, int length, int* store) {
  int value = 0;
  int sgn = 1;
  bool parsable = true;
  for ( int i = 0; i < length; i++ ) {
    char c = str[i];
    if ( c == '-' && i == 0 ) {
      sgn = -1;
      parsable = false; // to prevent empty "-" being parsed
      continue;
    } else if (!(c >= '0' && c <= '9')) {
      return false;
    }
    parsable = true;
    value *= 10;
    value += c - '0';
  }
  if (!parsable) return false;
  *store = sgn * value;
  return parsable;
}

void _rm_all_spaces(char* read) {
  char* write = read;
  while (*read) {
    if (*read != ' ') {
      *write++ = *read;
    }
    read++;
  }
  *write = '\0';
}

bool parse_point_time(const char* str, struct tf_clock* t, int length) {

  // only possible lengths are 4 & 5, quick filtering
  if (length != 4 && length != 5) return false;
  
  char* ptr = strchr(str, ':');
  if (ptr == NULL) return false;

  int hr = 0, min = 0;
  if (!parse_int(str, ptr - str, &hr)) return false;
  if (!parse_int(ptr+1, 2, &min)) return false;

  // validate range
  if (hr < 0 || hr >= 24 || min < 0 || min >= 60) return false;
  
  t->h = hr;
  t->m = min;

  return true;
}

/**
  ## Parses time in the following format
  ( ? = entirely optional, ?(xx) = optional, default = xx if omitted)

  - Absolute time : `h?h:mm`, `h?h:mm - h?h:mm`
  
  - Special time : `now`, `now - hh:mm`, `hh:mm - now`, `full (all day)`

  - Dynamic time : `hh:mm ...`, `... hh:mm`
  
  - With duration : `... <TimeDuration>`, `<TimeDuration> ...`, `<Time?(now)> +/- <TimeDuration>`

 * Any spaces will be stripped off before parsing
 * 
 * @param inputStr String to parse
 * @param t Destination pointer
 * 
 * @returns true on success, false on failure
 */
bool parse_time(const char* inputStr, tf_time* t) {
  struct tf_atm start = {0};
  struct tf_atm end = {0};
  bool parsable = true;
  
  unsigned length = (unsigned) strlen(inputStr) + 1;
  
  char* str = calloc(sizeof *str, length);
  if (str == NULL) {
    fprintf(stderr, "\ncritical : `parse_time` cannot allocate memory!\n");
    return false;
  }
  strcpy(str, inputStr);
  _rm_all_spaces(str);
  
  length = strlen(str);
  parsable &= parse_point_time(str, &start.clock, _imin(length, 5)); // maximum length = "00:00" = 5

  end = start; // for point-times, end is same as start

  if ( parsable && length > 5 ) {
    unsigned pos = str[4] == '-' ? 5: str[5] == '-' ? 6: 0; // skip one position
    parsable &= pos ? parse_point_time(str + pos, &end.clock, length - pos): false;
  }

  free(str);
  
  if (!parsable) return false;
  
  t->start = start;
  t->end = end;
  return parsable;
}