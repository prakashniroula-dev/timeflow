#if !defined(TIMEFLOW_TASKS_H)
#define TIMEFLOW_TASKS_H

#include <tf_types.h>

typedef struct tf_task {
  const char* title;
  const char* desc;
  tf_time schedule;
} tf_task;

#endif // TIMEFLOW_TASKS_H
