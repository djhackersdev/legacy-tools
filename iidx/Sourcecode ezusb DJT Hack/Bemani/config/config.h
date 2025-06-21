#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

#include "libutil/util.h"

enum game_type
{
	BEATMANIA_IIDX,
	GUITAR_FREAKS,
	DRUM_MANIA,
	POPN_MUSIC
};

struct action_desc
{
	const wchar_t *desc;
	u8_t value;
};

extern const struct action_desc iidx_actions[];
extern const struct action_desc gf_actions[];
extern const struct action_desc dm_actions[];

extern HINSTANCE inst;
extern enum game_type game_type;

const wchar_t *get_string(UINT string_id);
HPROPSHEETPAGE common_create_pp(void);
HPROPSHEETPAGE eam_create_pp(void);
HPROPSHEETPAGE iidx_create_pp(void);

#endif