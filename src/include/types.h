/**
 * types.h
 * Type definitions for use in timeflow
 * 
 * Author : github.com/prakashniroula-dev
 */

#if !defined(TIMEFLOW_TYPES_H)
#define TIMEFLOW_TYPES_H

/* Development phase */
#include <stdbool.h>
#include <stdint.h>

struct tf_clock {
  uint8_t h;
  uint8_t m;
};

enum tf_inp_enum {
  tf_inp_time_absolute = 0,
  tf_inp_time_now, tf_inp_time_full,
  tf_inp_time_prev, tf_inp_time_next,
  tf_inp_time_dur_prev, tf_inp_time_dur_next
};

enum tf_enum {
  tf_time_absolute = 0,
  tf_time_prev, tf_time_next,
  tf_time_dur_prev, tf_time_dur_next
};


struct tf_inp_atm {
  struct tf_clock clock;
  enum tf_inp_enum type;
};

struct tf_atm {
  struct tf_clock clock;
  enum tf_enum type;
};

typedef struct tf_time {
  struct tf_atm start;
  struct tf_atm end;
} tf_time;

#endif // TIMEFLOW_TYPES_H
