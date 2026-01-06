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

static inline void test_parser() {
  const char* str = "5:58 + 16 hr 49 min";
  const char* ptr = str;
  tf_time t = tfp_parse_time(&ptr, NULL);
  tf_print("%02d:%02d\n", t.start.clock.h, t.start.clock.m);
  tf_print("%02d:%02d", t.end.clock.h, t.end.clock.m);
}