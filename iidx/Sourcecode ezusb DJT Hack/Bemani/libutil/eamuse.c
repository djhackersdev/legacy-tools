#include "libutil/stdafx.h"
#include "libutil/eamuse.h"

static wchar_t machine_id_path[MAX_PATH];
static wchar_t card_paths[2][MAX_PATH];

const wchar_t *eam_get_machine_id_path(void)
{
	return machine_id_path;
}

const wchar_t *eam_get_card_path(int unit_id)
{
	return card_paths[unit_id];
}

bool eam_read_card(int unit_id, u8_t *user_id)
{
	FILE *f;
	int i;

	f = _wfopen(card_paths[unit_id], L"r");

	if (f != NULL) {
		memset(user_id, 0, USER_ID_NBYTES);

		for (i = 0 ; i < USER_ID_NBYTES ; i++) {
			fscanf(f, "%02x", user_id + i);
		}

		fclose(f);
		return true;
	} else {
		return false;
	}
}

void eam_read_machine_id(u8_t *machine_id)
{
	FILE *f;
	int i;

	/* Set machine ID to zeros. If we can't read a machine ID file, then just
	 * return the zeros: a game will probably crash if this is not successful. */

	memset(machine_id, 0, MACHINE_ID_NBYTES);
	f = _wfopen(machine_id_path, L"r");

	if (f != NULL) {
		for (i = 0 ; i < MACHINE_ID_NBYTES ; i++) {
			fscanf(f, "%02x", machine_id + i);
		}

		fclose(f);
	}
}

void eam_set_machine_id_path(const wchar_t *path)
{
	wcsncpy(machine_id_path, path, MAX_PATH);
}

void eam_set_card_path(int unit_id, const wchar_t *path)
{
	wcsncpy(card_paths[unit_id], path, MAX_PATH);
}
