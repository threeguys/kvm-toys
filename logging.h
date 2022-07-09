
#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <stdio.h>
#include <errno.h>

#define log(...)    { printf(__VA_ARGS__); printf("\n"); }
#define logm(msg)   log("%s\n", msg)
#define logerr(msg) log("ERROR[%s] - %s\n", strerror(errno), msg)

#endif // __LOGGING_H__
