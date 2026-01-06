/**
 * parser.test.h
 * Testing for parsers in timeflow
 * 
 * Author : github.com/prakashniroula-dev
 */

/* Development phase */

#include <parser.h>
#include <stdbool.h>
#include <stdio.h>
#include <tf_io.h>

static int _count = 0;

void tf_print_time(tf_time t) {
  _count++;
  enum tf_enum s_typ = t.start.type;
  enum tf_enum e_typ = t.end.type;
  printf("%d. Time = ", _count);

  if ( s_typ == tf_time_absolute )
    printf("%02d:%02d ", t.start.clock.h, t.start.clock.m);
  else if (s_typ == tf_dtime_prev)
    printf("... ");
  else if (e_typ == tf_time_null);
  else
    printf("<invalid> ");

  if (e_typ == tf_time_absolute)
    printf(s_typ == tf_dtime_prev ? "%02d:%02d": "- %02d:%02d", t.end.clock.h, t.end.clock.m);
  else if (e_typ == tf_dtime_next)
    printf("...");
  else if (e_typ == tf_dtime_plus_dur)
    printf("+ %dhr %dmin", t.end.clock.h, t.end.clock.m);
  else if (e_typ == tf_dtime_minus_dur)
    printf("- %dhr %dmin", t.end.clock.h, t.end.clock.m);
  else if (e_typ == tf_time_null);
  else if (s_typ == tf_time_invalid && e_typ == tf_time_invalid);
  else
    printf("<invalid>");

  puts("\n====================\n");
}

static inline void test_parser() {
  const char* str;
  const char* ptr;
  tf_time t;

  // === Working parsable inputs ===
  
  str = "12:00"; // point time
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // 12:00
  
  str = "now"; // current_time (point time)
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // e.g. 15:15
  
  str = "now + 2h"; // current_time - (current_time + 2h)
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // e.g. 15:15 - 17:15
  
  str = "now - 1h"; // (current_time - 1h) - current_time
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // e.g. 14:15 - 15:15
  
  str = "14:00 + 2h 14m";
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // 14:00 - 16:14
  
  str = "14:00 - 2hr 15min"; // different format
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // 11:45 - 14:00
  
  str = "11:00 ..."; // 11:00 to next block start eqv to 11:00 - ...
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // 11:00 ...
  
  str = "... - 12:00"; // prev block end to 12:00, eqv to ... 12:00 (different format)
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // ... 12:00
  
  str = "... + 2h 5m"; // prev block end to 2hr 5min more
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // ... + 2hr 5min
  
  str = "... - 2hr 6min"; // prev block start - 2hr 6min to prev block start (different format)
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t); // ... - 2hr 6min
  
  // === Errors ===
  
  str = "random"; // invalid format
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t);
  
  str = "24:00"; // hr > 23 : invalid format
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t);
  
  str = "12:80"; // min > 60 : invalid format
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t);
  
  str = "12:12 - "; // incomplete!!
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t);
  
  str = "5:00 - 12h"; // out of bounds
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t);
  
  str = "5:01 - 5hr 2min"; // out of bounds
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t);
  
  str = "23:01 + 1hr"; // out of bounds
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t);
  
  str = "12:xx + 1hr"; // invalid format in middle
  ptr = str;
  t = tfp_parse_time(&ptr, NULL);
  tf_print_time(t);

}