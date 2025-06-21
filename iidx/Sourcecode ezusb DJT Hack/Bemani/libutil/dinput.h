#ifndef LIBUTIL_DINPUT_H
#define LIBUTIL_DINPUT_H

#include "libutil/util.h"

#ifdef __cplusplus
extern "C" {
#endif

void dinput_init(void);
IDirectInput8 *dinput_get_api(void);
void dinput_fini(void);

#ifdef __cplusplus
}
#endif

#endif