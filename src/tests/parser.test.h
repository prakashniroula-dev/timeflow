/**
 * parser.test.h
 * Testing for parsers in timeflow
 * 
 * Author : github.com/prakashniroula-dev
 */

/* Development phase */

#include <parser.h>
#include <test.h>
#include <stdbool.h>
#include <stdio.h>

static inline void test_parser() {
  tf_time t;

  puts("");
  
  printf("%d\n", parse_time("4:00", &t));
  printf("%d:%02d\n", t.start.clock.h, t.start.clock.m);
  printf("%d:%02d\n\n", t.end.clock.h, t.end.clock.m);

  printf("%d\n", parse_time("04:00 - 08:12", &t));
  printf("%d:%02d\n", t.start.clock.h, t.start.clock.m);
  printf("%d:%02d\n\n", t.end.clock.h, t.end.clock.m);
  
}