#include "libutil/stdafx.h"
#include "libutil/util.h"
#include "libutil/input.h"

#include "imports/libavs-win32.h"
#include "imports/libsystem.h"

#define PICK_BIT 0x20

/* Weird backwards two-bit integer */
static const u32_t eff_lookup[] = { 0x0000, 0x0100, 0x0080, 0x0180 };
static const char *dbg_keys[] = { "autoplay", "neverend" };

static u32_t state[2];
static u8_t effector[2];
static bool dbg_state[2];

/* While Guitar Freaks only has one pick direction, Guitar Hero controllers have two,
 * and rapid picking causes these controllers to alternate between these directions
 * without going via zero on their vertical axis. A naive binding of both pick
 * directions would therefore cause the pick to get 'stuck' at any reasonably fast
 * picking speed.
 *
 * Therefore, we have separate synthetic 'Pick A' and 'Pick B' bindings. Alternation
 * between these two actions causes a pick to be signalled to the game for one frame. */

static void STDCALL input_callback(u8_t action, bool edge, void *ctx)
{
	u8_t index;

	if (action < 0x10) {
		/* Update 1P input bit */
		state[0] |= (1 << action);

	} else if (action < 0x20) {
		/* Update 2P input bit */
		state[1] |= (1 << (action - 0x10));

	} else if (action < 0x30 && edge) {
		/* Update 1P/2P pick */
		index = action - 0x20;
		state[index / 2] |= PICK_BIT;

	} else if (action < 0x40 && edge) {
		/* Update 1P/2P effector */
		index = action - 0x30;
		effector[index] = (effector[index] + 1) % 4;

	} else if (action < 0x50 && edge) {
		/* Fire a debugging toggle */
		index = action -  0x40;
		dbg_state[index] = !dbg_state[index];
		sys_debug_dip_set_param(dbg_keys[index], dbg_state[index]);

	} else if (action == 0xFF) {
		/* Exit */
		ExitProcess(0);
	}
}

#ifndef NDEBUG
static void log_proc(void *ctx, const char *msg, int msg_len)
{
	OutputDebugStringA(msg);
}
#endif

DLLEXPORT int device_check_secplug(int param)
{
	static const int response[] = { 257, 256 };
	return response[param];
}

DLLEXPORT void device_finalize(void)
{
	timeEndPeriod(1);
	libutil_fini();
}

DLLEXPORT void device_get_coinstock(unsigned short *,unsigned short *)
{}

DLLEXPORT unsigned char device_get_dipsw(int)
{
	return 0;
}

DLLEXPORT unsigned int device_get_input(int player)
{
	return state[player];
}

/* I don't know what this is used for, actually. It would be interesting
 * to mess around with it. */
DLLEXPORT unsigned __int64 device_get_input_time(void)
{
	return timeGetTime();
}

DLLEXPORT int device_get_jamma_history(struct T_JAMMA_HISTORY_INFO *,int)
{
	return 0;
}

DLLEXPORT int device_get_status(void)
{
	return 0;
}

DLLEXPORT int device_get_subboard_version(char *dest, int nbytes)
{
	/* It's a shoutout to 2ch akin to lololololol, w or 0x77 doesn't actually
	 * mean anything :) */
	strncpy(dest, "wwwwwwwwwwwwwwwwwww", nbytes);
	return 0;
}

DLLEXPORT int device_initialize(void)
{
	const wchar_t *cmdline;
	bool quote;

	timeBeginPeriod(1);

#ifndef NDEBUG
	log_boot(log_proc, NULL);
#endif

	cmdline = GetCommandLine();
	quote = false;

	/* Skip executable name */
	while (*cmdline != L'\0' && !iswspace(*cmdline) && !quote) {
		if (*cmdline == L'"') {
			quote = !quote;
		}

		cmdline++;
	}

	/* Skip space after executable name */
	while (*cmdline != L'\0' && iswspace(*cmdline)) {
		cmdline++;
	}

	/* Determine game type */
	if (wcscmp(cmdline, L"-g") == 0) {
		std_setenv("/env/profile/mcode", "xxG33JAA");
		libutil_init(L"gf");
	} else if (wcscmp(cmdline, L"-d") == 0) {
		std_setenv("/env/profile/mcode", "xxG32JAA");
		libutil_init(L"dm");
	} else {
		panic(L"Process launched with unknown game type. Command line is: \"%s\"\n", GetCommandLine());
	}

	/* Set other security data */
	std_setenv("/env/profile/hwid",	"0000000000000000");
	std_setenv("/env/profile/softid", "0000000000000000");
	std_setenv("/env/profile/sysid", "0000000000000000");

	return 0;
}

DLLEXPORT void device_poweroff(void)
{
	ExitProcess(0);
}

DLLEXPORT void device_set_coincounter_controllable(unsigned char,unsigned char)
{}

DLLEXPORT void device_set_coincounter_merge(unsigned char)
{}

DLLEXPORT void device_set_coincounter_work(unsigned char,unsigned char)
{}

DLLEXPORT void device_set_jamma_asyncmode(void)
{}

DLLEXPORT void device_set_jamma_unti_inputskip(int)
{}

DLLEXPORT void device_set_portoutbit(int,int)
{}

DLLEXPORT void device_update(void)
{
	int i;

	memset(state, 0, sizeof(state));
	input_scan(input_callback, NULL);

	for (i = 0 ; i < 2 ; i++) {
		state[i] |= eff_lookup[effector[i]];
	}
}

DLLEXPORT void device_update_secplug(void)
{}
