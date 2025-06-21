#ifndef EZUSB_IIDX_TURNTABLE_H
#define EZUSB_IIDX_TURNTABLE_H

#include "libutil/util.h"

void tt_init(void);
void tt_begin_poll(int player_id);
void tt_active(int player_id, int dir);
void tt_filtered_active(int player_id, int dir);
void tt_stab(int player_id);
void tt_start_held(int player_id);
void tt_end_poll(int player_id, int mouse_delta);
u8_t tt_get(int player_id);

#endif