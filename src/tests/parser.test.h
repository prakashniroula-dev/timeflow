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

static inline void test_parser() {
  const char* str = "25:45";
  const char* ptr = str;
  struct tf_atm t = tfp_parse_time_atm(&ptr, NULL);
  printf("%d:%02d", t.clock.h, t.clock.m);
}