/**
 * test.h
 * Small plug-in utility for testing, uses few macro tricks & variables
 * 
 * Author : github.com/prakashniroula-dev
 */

#if !defined(__TEST_PLUG_IN_H)
#define __TEST_PLUG_IN_H

#include <assert.h>

static int __PRINT_ERR = 1;
static int __PRINT_SUCCESS = 0;

#define __PRINT_ERR(...) fprintf(stderr, __VA_ARGS__)
#define __PRINT_SUCCESS(...) printf(__VA_ARGS__)
#define expect(expr, argFail, ...) for (; (__PRINT_ERR+__PRINT_SUCCESS) && (!(expr) ? __PRINT_ERR argFail: __PRINT_SUCCESS __VA_ARGS__ && 0); assert(expr))

#endif // __TEST_PLUG_IN_H
