#ifndef LIBUTIL_EAMUSE_H
#define LIBUTIL_EAMUSE_H

#include "libutil/util.h"

#define MACHINE_ID_NBYTES 10
#define USER_ID_NBYTES 8

enum card_mode
{
	CARD_USE_NUMLOCK,
	CARD_ALWAYS_P1,
	CARD_ALWAYS_P2
};

LIBUTIL_API const wchar_t *eam_get_card_path(int unit_num);
LIBUTIL_API const wchar_t *eam_get_machine_id_path(void);
LIBUTIL_API bool eam_read_card(int unit_num, u8_t *user_id);
LIBUTIL_API void eam_read_machine_id(u8_t *machine_id);
LIBUTIL_API void eam_set_card_path(int unit_num, const wchar_t *path);
LIBUTIL_API void eam_set_machine_id_path(const wchar_t *path);

#endif