#ifndef LIBAVS_WIN32_H
#define LIBAVS_WIN32_H

#include "libutil/util.h"

#ifdef __cplusplus
extern "C" {
#endif

DLLIMPORT void log_boot(void (*log_proc)(void *ctx, const char *msg, int msg_len), void *ctx);
DLLIMPORT void std_setenv(const char *key, const char *val);

#ifdef __cplusplus
}
#endif

#endif