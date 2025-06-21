#ifndef LIBUTIL_CONFIG_H
#define LIBUTIL_CONFIG_H

#include "libutil/util.h"

void config_init(const wchar_t *game_type);
LIBUTIL_API int config_get_attr(const char *tag);
void config_load(void);
LIBUTIL_API void config_save(void);
LIBUTIL_API void config_set_attr(const char *tag, int value);
void config_fini(void);

#endif