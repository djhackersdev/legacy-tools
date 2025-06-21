#include "libutil/stdafx.h"
#include "libutil/eamuse.h"
#include "libutil/input.h"
#include "libutil/util.h"

/* And the prize for the most incredibly weird and hard to reverse engineer API goes to... */

#define WARMUP_STEPS 601

enum workflow {
	/* Labels taken from game debug text */
	STEP,
	SLEEP,
	START,
	INIT,
	READY,
	GET_USERID,
	ACTIVE,
	EJECT,
	EJECT_CHECK,
	END,
	CLOSE_EJECT,
	CLOSE_E_CHK,
	CLOSE_END,

	ERR_GETUID = -2
};

struct icca_status {
	u8_t status_code;
	u8_t solenoid;
	u8_t front_sensor;
	u8_t rear_sensor;
	u8_t uid[USER_ID_NBYTES];
	s32_t error;
	u32_t key_edge;
	u32_t key_level;
};

struct unit {
	struct icca_status status;
	enum workflow state;
	bool card_cmd_pressed;
	bool card_in;
	u8_t last_key_serial;
};

static struct unit units[2];
static int warmup_counter;

/*
 * Keyboard read
 */

static void update_10k(int unit_id)
{
	static int scan_codes[] = {
		DIK_NUMPAD0,
		DIK_NUMPAD1,
		DIK_NUMPAD4,
		DIK_NUMPAD7,
		DIK_NUMPADENTER,
		DIK_NUMPAD2,
		DIK_NUMPAD5,
		DIK_NUMPAD8,
		DIK_NUMPADPERIOD,
		DIK_NUMPAD3,
		DIK_NUMPAD6,
		DIK_NUMPAD9
	};

	struct unit *unit;
	const u8_t *keyboard;
	u32_t last_levels;
	u32_t bit;
	int i;

	keyboard = input_get_state(DEV_CLASS_KEYBOARD, 0);
	unit = &units[unit_id];

	/* Initialise key level variables */
	last_levels = unit->status.key_level;
	unit->status.key_level = 0;

	for (i = 0 ; i < 12 ; i++) {
		bit = keyboard[scan_codes[i]];

		if (bit) {
			/* Key is pressed */
			bit <<= i;
			
			if (!(last_levels & bit)) {
				/* This key has just been pressed */
				unit->status.key_edge = 0x80 | (unit->last_key_serial << 4) | i;

				/* Advance the event serial number. Presumably this allows the
				 * game to detect a key being re-pressed. Why the MSBit though? */
				unit->last_key_serial++;
				unit->last_key_serial &= 7;
			}

			unit->status.key_level |= bit;
		}
	}
}

static void update_card(int unit_id)
{
	struct unit *unit;
	const u8_t *keyboard;

	unit = &units[unit_id];
	keyboard = input_get_state(DEV_CLASS_KEYBOARD, 0);

	if (keyboard[DIK_NUMPADPLUS]) {
		if (!unit->card_cmd_pressed) {
			/* User has just pressed the card cmd key. Cycle states */
			if (unit->state == READY) {
				if (eam_read_card(unit_id, unit->status.uid)) {
					unit->state = ACTIVE;
				} else {
					unit->state = ERR_GETUID;
				}

				unit->card_in = true;

			} else if (unit->state == EJECT_CHECK) {
				unit->state = SLEEP;
				unit->card_in = false;
			}
		}

		unit->card_cmd_pressed = true;
	} else {
		unit->card_cmd_pressed = false;
	}
}

/*
 * ACIO stubs (ACIO is the bridge between the PC and the two ICCA units)
 */

DLLEXPORT bool ac_io_begin(int param)
{
	return true;
}

DLLEXPORT void ac_io_end(void)
{}

/* Only ever called to get further information on an error. */
DLLEXPORT void ac_io_get_rs232c_status(int *status)
{}

/* This has to return true on EXACTLY the 602nd call. No more, no less.
 * Quite what the hell is going on here is beyond me. */
DLLEXPORT bool ac_io_is_active(int param)
{
	return ++warmup_counter > WARMUP_STEPS;
}

DLLEXPORT void ac_io_reset(void)
{}

/*
 * ICCA interface
 */

/* Again, check for the presence of an error. */
DLLEXPORT bool ac_io_icca_get_keep_alive_error(int unit_id, int *error)
{
	return false;
}

/* Read out all of the IC Card unit's registers */
DLLEXPORT void ac_io_icca_get_status(struct icca_status *out_status, int unit_id)
{
	memcpy(out_status, &units[unit_id].status, sizeof(struct icca_status));
}

/* Poll for completion of UID read */
DLLEXPORT void ac_io_icca_req_uid_isfinished(int unit_id, int *read_state)
{
	/* This means "We're done reading the UID" */
	*read_state = 8;
}

/* What it says on the tin. */
DLLEXPORT void ac_io_icca_send_keep_alive_packet(int unit_id, int param1, int param2)
{}

/* Send a command to the ICCA unit, and return the unit's response. STEP just means
 * "Report your current status", everything else modifies its internal state somehow. */
DLLEXPORT int ac_io_icca_workflow(int workflow, int unit_id)
{
	struct unit *unit;

	unit = &units[unit_id];

	/* Here's where the magic happens ... */
	switch (workflow)  {
		case STEP:
			if ((GetKeyState(VK_NUMLOCK) & 1) == unit_id) {
				update_10k(unit_id);
				update_card(unit_id);
			}

			break;

		case INIT:
			unit->state = READY;
			break;

		case START:
			unit->state = READY;
			break;

		case CLOSE_EJECT:
			unit->state = unit->card_in ? EJECT_CHECK : SLEEP;
			break;

		case CLOSE_END:
			unit->state = SLEEP;			
			break;

		default:
			break;
	}

	return unit->state;
}