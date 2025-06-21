#include "libutil/stdafx.h"
#include "libutil/input.h"

#include "config/config.h"
#include "config/resource.h"

static HWND devices_ctl;
static HWND controls_ctl;
static HWND actions_ctl;
static HWND pressed_ctl;
static const struct action_desc *action_list;
static enum dev_class cur_dev_class;
static u8_t cur_dev_id;
static u8_t cur_control_id;

static const wchar_t *get_binding_desc(enum dev_class dev_class, u8_t dev_id, u8_t control_id)
{
	const struct action_desc *pos;
	u8_t action;

	if (input_get_binding(dev_class, dev_id, control_id, &action)) {
		for (pos = action_list ; pos->desc != NULL ; pos++) {
			if (pos->value == control_id) {
				return pos->desc;
			}
		}
	}

	return NULL;
}

static void bind_controls(void)
{
	LRESULT index;
	const wchar_t *control_desc;
	int control_id;
	int ncontrols;

	/* Clear up previous control bindings */
	SendMessage(controls_ctl, LB_RESETCONTENT, 0, 0);

	/* Set up new bindings */
	ncontrols = input_get_ncontrols(cur_dev_class);
	for (control_id = 0 ; control_id < ncontrols ; control_id++) {
		control_desc = input_describe(cur_dev_class, cur_dev_id, control_id);

		if (control_desc != NULL) {
			index = SendMessage(controls_ctl, LB_ADDSTRING, 0, (LPARAM)control_desc);
			SendMessage(controls_ctl, LB_SETITEMDATA, (WPARAM)index, (LPARAM)control_id);
		}
	}
}

static void bind_devices(void)
{
	LRESULT index;
	enum dev_class dev_class;
	const wchar_t *device_name;
	int ndevs;
	int i;

	for (dev_class = 0 ; dev_class < DEV_CLASS_MAX ; dev_class++) {
		ndevs = input_get_ndevs(dev_class);

		for (i = 0 ; i < ndevs ; i++) {
			device_name = input_get_name(dev_class, i);

			index = SendMessage(devices_ctl, CB_ADDSTRING, 0, (LPARAM)device_name);
			SendMessage(devices_ctl, CB_SETITEMDATA, (WPARAM)index, (LPARAM)((dev_class << 8) | i));
		}
	}

	SendMessage(devices_ctl, CB_SETCURSEL, 0, 0);
}

static void init_dialog(HWND dialog)
{
	const struct action_desc *pos;
	int index;

	/* Find controls */
	devices_ctl = GetDlgItem(dialog, IDC_DEVICE);
	controls_ctl = GetDlgItem(dialog, IDC_CONTROL);
	actions_ctl = GetDlgItem(dialog, IDC_ACTION);
	pressed_ctl = GetDlgItem(dialog, IDC_PRESSED);

	switch (game_type) {
		case BEATMANIA_IIDX:
			action_list = iidx_actions;
			break;

		case GUITAR_FREAKS:
			action_list = gf_actions;
			break;

		case DRUM_MANIA:
			action_list = dm_actions;
			break;
	}	

	/* Bind action list */
	SendMessage(actions_ctl, LB_ADDSTRING, 0, (LPARAM)L"");
	for (pos = action_list ; pos->desc != NULL ; pos++) {
		index = SendMessage(actions_ctl, LB_ADDSTRING, 0, (LPARAM)pos->desc);
		SendMessage(actions_ctl, LB_SETITEMDATA, index, (LPARAM)pos->value);
	}

	/* Initialise the other controls */
	bind_controls();
	bind_devices();

	/* Create a polling timer */
	SetTimer(dialog, 0, 17, NULL);
}

static void evt_device_change(void)
{
	LRESULT index;
	LRESULT device_info;

	/* Get selected device */
	index = SendMessage(devices_ctl, CB_GETCURSEL, 0, 0);
	device_info = SendMessage(devices_ctl, CB_GETITEMDATA, (WPARAM)index, 0);

	/* Unpack device ID */
	cur_dev_class = device_info >> 8;
	cur_dev_id = (u8_t)device_info;

	/* Rebind controls list */
	bind_controls();
}

static void evt_control_change()
{
	LRESULT index;
	u8_t action_id;
	int i;

	index = SendMessage(controls_ctl, LB_GETCURSEL, 0, 0);
	cur_control_id = (u8_t)SendMessage(controls_ctl, LB_GETITEMDATA, index, 0);

	if (input_get_binding(cur_dev_class, cur_dev_id, cur_control_id, &action_id)) {
		for (i = 0 ; action_list[i].desc != NULL ; i++) {
			if (action_list[i].value == action_id) {
				SendMessage(actions_ctl, LB_SETCURSEL, (WPARAM)(i + 1), 0);
			}
		}
	} else {
		SendMessage(actions_ctl, LB_SETCURSEL, 0, 0);
	}
}

static void evt_action_change(void)
{
	int action_pos;
	LVFINDINFO finder;
	LVITEM item;

	action_pos = SendMessage(actions_ctl, LB_GETCURSEL, 0, 0);

	/* Find the selected control in the control list and prepare to update it */
	finder.flags = LVFI_PARAM;
	finder.lParam = cur_control_id;

	item.mask = LVIF_TEXT;
	item.iItem = ListView_FindItem(controls_ctl, -1, &finder);
	item.iSubItem = 1;

	if (action_pos == 0) {
		/* Update GUI */
		item.pszText = L"";
		ListView_SetItem(controls_ctl, &item);

		/* Update the actual binding */
		input_unbind(cur_dev_class, cur_dev_id, cur_control_id);
	} else {
		action_pos--;

		/* Update GUI */
		item.pszText = (wchar_t *) action_list[action_pos].desc;
		ListView_SetItem(controls_ctl, &item);
		
		/* Update the actual binding */
		input_set_binding(cur_dev_class, cur_dev_id, cur_control_id, action_list[action_pos].value);
	}
}

static void evt_poll(void)
{
	wchar_t prev_txt[MAX_PATH];
	wchar_t buf[MAX_PATH];
	const u8_t *state;
	int ncontrols;
	int i;

	input_update();
	ncontrols = input_get_ncontrols(cur_dev_class);
	state = input_get_state(cur_dev_class, cur_dev_id);

	LoadString(inst, IDS_PRESSED, buf, MAX_PATH);

	for (i = 0 ; i < ncontrols ; i++) {
		if (state[i]) {
			wcscat_s(buf, MAX_PATH, L" ");
			wcscat_s(buf, MAX_PATH, input_describe(cur_dev_class, cur_dev_id, i));
		}
	}

	/* Minimise flicker */
	GetWindowText(pressed_ctl, prev_txt, MAX_PATH);
	if (wcscmp(prev_txt, buf) != 0) {
		SetWindowText(pressed_ctl, buf);
	}
}

static LRESULT CALLBACK dlg_proc(HWND dialog, UINT msg, WPARAM wparam, LPARAM lparam)
{	
	if (msg == WM_TIMER) {
		evt_poll();
		return TRUE;
	}

	if (msg == WM_COMMAND && LOWORD(wparam) == IDC_DEVICE && HIWORD(wparam) == CBN_SELCHANGE) {
		evt_device_change();
		return TRUE;
	} 
	
	if (msg == WM_COMMAND && LOWORD(wparam) == IDC_CONTROL && HIWORD(wparam) == LBN_SELCHANGE) {
		evt_control_change();
		return TRUE;
	} 
	
	if (msg == WM_COMMAND && LOWORD(wparam) == IDC_ACTION && HIWORD(wparam) == LBN_SELCHANGE) {
		evt_action_change();
		return TRUE;
	}

	if (msg == WM_INITDIALOG) {
		init_dialog(dialog);
		return TRUE;
	} 

	return FALSE;
}

HPROPSHEETPAGE common_create_pp(void)
{
	PROPSHEETPAGE psp;

	ZeroMemory(&psp, sizeof(psp));
	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = inst;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_COMMON);
	psp.pfnDlgProc = dlg_proc;

	return CreatePropertySheetPage(&psp);
}
