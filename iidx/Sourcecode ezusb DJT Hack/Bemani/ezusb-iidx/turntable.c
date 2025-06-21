#include "libutil/stdafx.h"

#include "ezusb-iidx/defs.h"
#include "libutil/config.h"

#define TT_FILTER_MIN 5
#define TT_FILTER_MAX 30
#define TT_SUDPLUS_DIVISOR 2

/* Turntable handling is pretty complicated, because the following forces are in play:
  		Turntable inputs may be normal, filtered, stabbed, or mouse-based
  		The speed of the first two must be halved while Start is held, to improve Sud+ handling.
  		Mouse input needs to be scaled by a configurable sensitivity value
  		IIDX AC expects its turntable to lag by a frame. In controllers with non-lagging
  			turntables, this lag must be simulated.
  
   Here is the flow of events:
   In tt_begin_poll():
  		Reset flags and directions, save filtered_active_dir in filtered_active_dir_prev
  
   During input_scan() callback:
  		Conventional TT press sets active_dir. 
  		Filtered TT press set filtered_active_dir.
  		TT Stab increments lag_buffer directly by stab_dir, then negates stab_dir.
  		If a start button is held, then start_held is set.
  
   In tt_end_poll():
  		filtered_counter is incremented if filtered_active_dir != filtered_active_dir_prev, otherwise reset to 0
  		If filtered_counter is < TT_FILTER_MIN or > TT_FILTER_MAX then pending_increment is set.
  
   At this point, both normal and filtered TT events are merged into pending_increment.
  		If start_held is set, then tt_increment must reach +/-TT_SUDPLUS_DIVISOR 
  			before it cascades to tt_lag_buf.
  		Otherwise, lag_buffer is incremented/decremented, and tt_increment is reset.
  		Mouse input is added to mouse_counter, and is multiplied by the current sensitivity.
  			When mouse_counter reaches IIDX_MOUSE_DIVISOR, lag_buffer is incremented/decremented.
  		Finally, if tt_lag is enabled, then its value is assigned to pos immediately.
  			Otherwise, the previous value of lag_buffer is used.
 */

struct tt_state
{
	bool start_held;
	int stab_dir;
	int active_dir;
	int filtered_active_dir_prev;
	int filtered_active_dir;
	int filtered_counter;
	int pending_increment;
	int mouse_counter;
	u8_t lag_buffer;
	u8_t pos;
};

static struct tt_state player[2];
static int mouse_sens;
static bool add_lag;

void tt_init(void)
{
	player[0].stab_dir = 4;
	player[1].stab_dir = 4;
	mouse_sens = config_get_attr("msens") + 1;
	add_lag = config_get_attr("tt_lag");
}

void tt_begin_poll(int player_id)
{
	struct tt_state *p;

	p = &player[player_id];
	p->filtered_active_dir_prev = p->filtered_active_dir;
	p->filtered_active_dir = 0;
	p->active_dir = 0;
	p->start_held = false;
}

void tt_active(int player_id, int dir)
{
	struct tt_state *p;

	p = &player[player_id];
	p->active_dir = dir;
}

void tt_filtered_active(int player_id, int dir)
{
	struct tt_state *p;

	p = &player[player_id];
	p->filtered_active_dir = dir;
}

void tt_stab(int player_id)
{
	struct tt_state *p;

	p = &player[player_id];
	p->lag_buffer += p->stab_dir;
	p->stab_dir *= -1;
}

void tt_start_held(int player_id)
{
	struct tt_state *p;

	p = &player[player_id];
	p->start_held = true;
}

void tt_end_poll(int player_id, int mouse_delta)
{
	struct tt_state *p;

	p = &player[player_id];
	
	if (add_lag) {
		/* Send last frame's state */
		p->pos = p->lag_buffer;
	}

	/* Process standard inputs */
	if (p->active_dir) {
		p->pending_increment += p->active_dir;
	}

	/* Process filtered inputs */
	/* Reset filtered state counter if the direction has changed */
	if (p->filtered_active_dir_prev != p->filtered_active_dir) {
		p->filtered_counter = 0;
	}

	/* Increment counter if filtered input is active */
	if (p->filtered_active_dir != 0) {
		p->filtered_counter++;

		if (p->filtered_counter < TT_FILTER_MIN || p->filtered_counter > TT_FILTER_MAX) {
			p->pending_increment += p->filtered_active_dir;
		}
	}

	/* Spill pending_increment depending on the state of start_held */
	if (p->start_held) {
		if (p->pending_increment <= -TT_SUDPLUS_DIVISOR || p->pending_increment >= +TT_SUDPLUS_DIVISOR) {
			p->lag_buffer += p->pending_increment / TT_SUDPLUS_DIVISOR;
			p->pending_increment = 0;
		}
	} else {
		p->lag_buffer += p->pending_increment;
		p->pending_increment = 0;
	}

	/* Process mouse motion */
	p->mouse_counter += mouse_delta;
	if (p->mouse_counter / IIDX_MOUSE_DIVISOR != 0) {
		p->lag_buffer += p->mouse_counter / IIDX_MOUSE_DIVISOR;
		p->mouse_counter %= IIDX_MOUSE_DIVISOR;
	}

	if (!add_lag) {
		/* Send the lag_buf value immediately (i.e. don't lag it) */
		p->pos = p->lag_buffer;
	}
}

u8_t tt_get(int player_id)
{
	return player[player_id].pos;
}
