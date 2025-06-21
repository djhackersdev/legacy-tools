#include "libutil/stdafx.h"
#include "libutil/util.h"

#include "config/config.h"
#include "config/resource.h"

HINSTANCE inst;
enum game_type game_type;

static enum game_type determine_game_type(const wchar_t *cmd_line)
{
	wchar_t str_title[1024];
	wchar_t str_msg[1024];

	if (wcscmp(cmd_line, L"iidx") == 0) {
		return BEATMANIA_IIDX;
	} else if (wcscmp(cmd_line, L"gf") == 0) {
		return GUITAR_FREAKS;
	} else if (wcscmp(cmd_line, L"dm") == 0) {
		return DRUM_MANIA;
	} else {
		LoadString(inst, IDS_ERROR, str_title, sizeof(str_title) / sizeof(wchar_t));
		LoadString(inst, IDS_UNKNOWN_GAME, str_msg, sizeof(str_msg) / sizeof(wchar_t));

		MessageBox(NULL, str_title, str_msg, MB_ICONERROR | MB_OK);
		ExitProcess(-1);
	}
}

static bool run_ui(void)
{
	HPROPSHEETPAGE psp[3];
	PROPSHEETHEADER psh;
	int npages;

	ZeroMemory(&psh, sizeof(psh));
	npages = 2;

	psp[0] = common_create_pp();
	psp[1] = eam_create_pp();

	if (game_type == BEATMANIA_IIDX) {
		psp[2] = iidx_create_pp();
		npages++;
	}

	psh.dwSize = sizeof(psh);
	psh.dwFlags = PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP | PSH_USEHICON;
	psh.hInstance = inst;
	psh.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	psh.pszCaption = get_string(IDS_TITLE);
	psh.nPages = npages;
	psh.phpage = psp;

	return (bool)PropertySheet(&psh);
}

const wchar_t *get_string(UINT string_id)
{
	static wchar_t buf[MAX_PATH];

	LoadString(inst, string_id, buf, MAX_PATH);
	return buf;
}

int WINAPI wWinMain(HINSTANCE inst_param, HINSTANCE dummy, wchar_t *cmd_line, int show)
{
	InitCommonControls();
	inst = inst_param;

	game_type = determine_game_type(cmd_line);
	libutil_init(cmd_line);

	if (run_ui()) {
		/* OK clicked */
		config_save();
	}

	libutil_fini();

	return 0;
}
