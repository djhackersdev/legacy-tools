#include "libutil/stdafx.h"

#include "config/config.h"
#include "config/resource.h"
#include "libutil/eamuse.h"

static HWND machine_id_path_ctl;
static HWND card0_path_ctl;
static HWND card1_path_ctl;

static void init_dialog(HWND dialog)
{
	const wchar_t *path;

	machine_id_path_ctl = GetDlgItem(dialog, IDC_MACHINE_ID);
	path = eam_get_machine_id_path();
	SetWindowText(machine_id_path_ctl, path);
	SendMessage(machine_id_path_ctl, EM_SETLIMITTEXT, (MAX_PATH - 1) * 2, 0);

	card0_path_ctl = GetDlgItem(dialog, IDC_CARD0);
	path = eam_get_card_path(0);
	SetWindowText(card0_path_ctl, path);
	SendMessage(card0_path_ctl, EM_SETLIMITTEXT, (MAX_PATH - 1) * 2, 0);

	card1_path_ctl = GetDlgItem(dialog, IDC_CARD1);
	path = eam_get_card_path(1);
	SetWindowText(card1_path_ctl, path);
	SendMessage(card1_path_ctl, EM_SETLIMITTEXT, (MAX_PATH - 1) * 2, 0);
}

static bool get_path(HWND dialog, wchar_t *buf, bool must_exist)
{
	wchar_t title[MAX_PATH];
	wchar_t filters[MAX_PATH];
	OPENFILENAME of;

	ZeroMemory(filters, MAX_PATH); /* Garbage shows up in the filter dropdown otherwise... */
	LoadString(inst, IDS_OPEN, title, MAX_PATH);
	LoadString(inst, IDS_OPEN_FILTER, filters, MAX_PATH);

	ZeroMemory(&of, sizeof(of));
	of.lStructSize = sizeof(of);
	of.hwndOwner = dialog;
	of.lpstrFilter = filters;
	of.nFilterIndex = 1;
	of.lpstrFile = buf;
	of.nMaxFile = MAX_PATH;
	of.lpstrTitle = title;
	of.Flags = OFN_HIDEREADONLY;
	of.lpstrDefExt = L"txt";

	if (must_exist) {
		of.Flags |= OFN_FILEMUSTEXIST;
	}

	return GetOpenFileName(&of) != 0;
}

static LRESULT CALLBACK dialog_proc(HWND dialog, UINT msg, WPARAM wparam, LPARAM lparam)
{
	wchar_t buf[MAX_PATH];

	if (msg == WM_INITDIALOG) {
		init_dialog(dialog);
		return TRUE;
	}

	if (msg == WM_COMMAND && HIWORD(wparam) == EN_CHANGE && LOWORD(wparam) == IDC_MACHINE_ID) {
		GetWindowText(machine_id_path_ctl, buf, MAX_PATH);
		eam_set_machine_id_path(buf);

		return TRUE;
	}

	if (msg == WM_COMMAND && HIWORD(wparam) == EN_CHANGE && LOWORD(wparam) == IDC_CARD0) {
		GetWindowText(card0_path_ctl, buf, MAX_PATH);
		eam_set_card_path(0, buf);

		return TRUE;
	}

	if (msg == WM_COMMAND && HIWORD(wparam) == EN_CHANGE && LOWORD(wparam) == IDC_CARD1) {
		GetWindowText(card1_path_ctl, buf, MAX_PATH);
		eam_set_card_path(0, buf);

		return TRUE;
	}

	if (msg == WM_COMMAND && HIWORD(wparam) == BN_CLICKED && LOWORD(wparam) == IDC_BROWSE_MACHINE_ID) {
		GetWindowText(machine_id_path_ctl, buf, MAX_PATH);

		if (get_path(dialog, buf, true)) {
			SetWindowText(machine_id_path_ctl, buf);
			eam_set_machine_id_path(buf);
		}

		return TRUE;
	}

	if (msg == WM_COMMAND && HIWORD(wparam) == BN_CLICKED && LOWORD(wparam) == IDC_BROWSE_CARD0) {
		GetWindowText(card0_path_ctl, buf, MAX_PATH);

		if (get_path(dialog, buf, false)) {
			SetWindowText(card0_path_ctl, buf);
			eam_set_card_path(0, buf);
		}

		return TRUE;
	}

	if (msg == WM_COMMAND && HIWORD(wparam) == BN_CLICKED && LOWORD(wparam) == IDC_BROWSE_CARD1) {
		GetWindowText(card1_path_ctl, buf, MAX_PATH);

		if (get_path(dialog, buf, false)) {
			SetWindowText(card1_path_ctl, buf);
			eam_set_card_path(1, buf);
		}

		return TRUE;
	}

	return FALSE;
}

HPROPSHEETPAGE eam_create_pp(void)
{
	PROPSHEETPAGE psp;

	ZeroMemory(&psp, sizeof(psp));
	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = inst;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_EAMUSE);
	psp.pfnDlgProc = dialog_proc;

	return CreatePropertySheetPage(&psp);
}
