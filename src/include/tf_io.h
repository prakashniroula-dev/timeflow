#if !defined(TIMEFLOW_IO_H)
#define TIMEFLOW_IO_H

#include <stdio.h>

#define tf_print_err(...) fprintf(stderr, "[tf error]: " __VA_ARGS__)
#define tf_print_warn(...) fprintf(stderr, "[tf warn]: " __VA_ARGS__)
#define tf_log(...) fprintf(stdin, "[tf log]: " __VA_ARGS__)
#define tf_print(...) fprintf(stdout, "[tf]: " __VA_ARGS__)

#endif // TIMEFLOW_IO_H
