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

enum tf_enum {
  tf_time_invalid = -1,
  tf_time_absolute = 0, // absolute time = hh:mm
  tf_time_null,
  // dtime = dynamic time (... blocks)
  tf_dtime_prev, tf_dtime_next,
  tf_dtime_minus_dur, tf_dtime_plus_dur
};

// atomic time (smallest unit of time timeflow uses)
struct tf_atm {
  struct tf_clock clock;
  enum tf_enum type;
};

// main tf_time (start and finish both)
typedef struct tf_time {
  struct tf_atm start;
  struct tf_atm end;
} tf_time;

enum tf_repeat_enum {
  tf_repeat_none = 0,
  tf_repeat_daily, tf_repeat_weekly, tf_repeat_monthly, tf_repeat_yearly,
  tf_repeat_workdays, tf_repeat_weekends, tf_repeat_holidays
};

typedef struct tf_date {
  uint8_t date;
  uint8_t month;
  uint8_t year;
} tf_date;

#endif // TIMEFLOW_TYPES_H
