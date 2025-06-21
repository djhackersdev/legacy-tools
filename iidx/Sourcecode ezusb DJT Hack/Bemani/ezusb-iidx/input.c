#include "libutil/stdafx.h"

#include "ezusb-iidx/defs.h"
#include "ezusb-iidx/turntable.h"
#include "libutil/config.h"
#include "libutil/input.h"
#include "libutil/mouse.h"
#include "libutil/util.h"
#include "imports/libavs-win32.h"

#define SLIDER_DIVISOR 8
#define SLIDER_MAX (15 * SLIDER_DIVISOR)
#define SLIDER_MID (8 * SLIDER_DIVISOR)
#define SLIDER_INC(i) if (sliders[i] < SLIDER_MAX) sliders[i]++
#define SLIDER_DEC(i) if (sliders[i] > 0) sliders[i]--

enum mouse_binding
{
	MBIND_NONE,
	MBIND_P1_TT,
	MBIND_P2_TT
};

static u8_t sliders[5];
static enum mouse_binding mouse_x;
static enum mouse_binding mouse_y;

#ifndef NDEBUG
static void log_proc(void *ctx, const char *msg, int msg_len)
{
	OutputDebugStringA(msg);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	log_boot(log_proc, NULL);
	return TRUE;
}
#endif

static void dispatch_misc(u8_t action, bool edge, u32_t *pad)
{
	switch (action) {
		case P1_TT_UP:
			tt_active(0, -1);
			break;

		case P1_TT_DOWN:
			tt_active(0, +1);
			break;

		case P1_TT_FILTERED_UP:
			tt_filtered_active(0, -1);
			break;

		case P1_TT_FILTERED_DOWN:
			tt_filtered_active(0, +1);
			break;

		case P1_TT_STAB:
			if (edge) {
				tt_stab(0);
			}

			break;

		case P2_TT_UP:
			tt_active(1, -1);
			break;

		case P2_TT_DOWN:
			tt_active(1, +1);
			break;

		case P2_TT_FILTERED_UP:
			tt_filtered_active(1, -1);
			break;

		case P2_TT_FILTERED_DOWN:
			tt_filtered_active(1, +1);
			break;

		case P2_TT_STAB:
			if (edge) {
				tt_stab(1);
			}

			break;

		case SLIDER0_UP:
			SLIDER_INC(0);
			break;

		case SLIDER0_DOWN:
			SLIDER_DEC(0);
			break;
			
		case SLIDER1_UP:
			SLIDER_INC(1);
			break;

		case SLIDER1_DOWN:
			SLIDER_DEC(1);
			break;

		case SLIDER2_UP:
			SLIDER_INC(2);
			break;

		case SLIDER2_DOWN:
			SLIDER_DEC(2);
			break;

		case SLIDER3_UP:
			SLIDER_INC(3);
			break;

		case SLIDER3_DOWN:
			SLIDER_DEC(3);
			break;

		case SLIDER4_UP:
			SLIDER_INC(4);
			break;

		case SLIDER4_DOWN:
			SLIDER_DEC(4);
			break;

		case EXIT:
			ExitProcess(0);
			break;
	}
}

static void STDCALL input_callback(u8_t action, bool edge, void *ctx)
{
	u32_t *pad = ctx;

	if (action < 0x20) {
		/* Set pad bit */
		*pad |= (1 << action);

		/* Notify the turntable code if a Start button is held, to slow down the TT. */
		if (action == 0x18) {
			tt_start_held(0);
		} else if (action == 0x19) {
			tt_start_held(1);
		}
	} else {
		/* Fiddle with some other control */
		dispatch_misc(action, edge, pad);
	}
}

DLLEXPORT int usbStart(void)
{
	libutil_init(L"iidx");

	tt_init();
	mouse_x = config_get_attr("x_axis");
	mouse_y = config_get_attr("y_axis");
	memset(sliders, SLIDER_MID, 5);

#ifndef NDEBUG
	log_boot(log_proc, NULL);
#endif

	return 0;
}

DLLEXPORT int usbGetTurnTable(int player_id)
{
	return tt_get(player_id);
}

DLLEXPORT int usbGetSlide(int slider)
{
	return sliders[slider] / SLIDER_DIVISOR;
}

DLLEXPORT int usbPadRead(u32_t *pad)
{
	int dx;
	int dy;

	*pad = 0;

	tt_begin_poll(0);
	tt_begin_poll(1);

	input_scan(input_callback, pad);
	mouse_read(&dx, &dy);

	tt_end_poll(0, mouse_x == MBIND_P1_TT ? dx : mouse_y == MBIND_P1_TT ? dy : 0);
	tt_end_poll(1, mouse_x == MBIND_P2_TT ? dx : mouse_y == MBIND_P2_TT ? dy : 0);

	return 0;
}
