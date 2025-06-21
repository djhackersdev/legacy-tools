#ifndef LIBUTIL_MOUSE_H
#define LIBUTIL_MOUSE_H

#include "libutil/util.h"

#ifdef __cpluspus
extern "C" {
#endif

void mouse_init(void);
LIBUTIL_API void mouse_read(int *dx, int *dy);
void mouse_fini(void);

#ifdef __cplusplus
}
#endif

#endif