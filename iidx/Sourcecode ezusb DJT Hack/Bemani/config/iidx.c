#include "libutil/stdafx.h"

#include "config/config.h"
#include "config/resource.h"

#include "ezusb-iidx/defs.h"

#include "libutil/config.h"

const struct action_desc iidx_actions[] =
{
	{ L"Exit",			EXIT				},
	{ L"P1 1",			P1_1				},
	{ L"P1 2",			P1_2				},
	{ L"P1 3",			P1_3				},
	{ L"P1 4",			P1_4				},
	{ L"P1 5",			P1_5				},
	{ L"P1 6",			P1_6				},
	{ L"P1 7",			P1_7				},
	{ L"P1 Start",		P1_START			},
	{ L"P1 TT +",		P1_TT_UP			},
	{ L"P1 TT -",		P1_TT_DOWN			},
	{ L"P1 TT + [KOC]", P1_TT_FILTERED_UP	},
	{ L"P1 TT - [KOC]", P1_TT_FILTERED_DOWN },
	{ L"P1 TT +/-",		P1_TT_STAB			},
	{ L"P2 1",			P2_1				},
	{ L"P2 2",			P2_2				},
	{ L"P2 3",			P2_3				},
	{ L"P2 4",			P2_4				},
	{ L"P2 5",			P2_5				},
	{ L"P2 6",			P2_6				},
	{ L"P2 7",			P2_7				},
	{ L"P2 Start",		P2_START			},
	{ L"P2 TT +",		P2_TT_UP			},
	{ L"P2 TT -",		P2_TT_DOWN			},
	{ L"P2 TT + [KOC]", P2_TT_FILTERED_UP	},
	{ L"P2 TT - [KOC]", P2_TT_FILTERED_DOWN },
	{ L"P2 TT +/-",		P2_TT_STAB			},
	{ L"VEFX",			VEFX				},
	{ L"Effect",		EFFECT				},
	{ L"Test",			TEST				},
	{ L"Service",		SERVICE				},
	{ L"VEFX+",			SLIDER0_UP			},
	{ L"VEFX-",			SLIDER0_DOWN		},
	{ L"Filter+",		SLIDER1_UP			},
	{ L"Filter-",		SLIDER1_DOWN		},
	{ L"Low-EQ+",		SLIDER2_UP			},
	{ L"Low-EQ-",		SLIDER2_DOWN		},
	{ L"Hi-EQ+",		SLIDER3_UP			},
	{ L"Hi-EQ-",		SLIDER3_DOWN		},
	{ L"Play Volume+",	SLIDER4_UP			},
	{ L"Play Volume-",	SLIDER4_DOWN		},
	{ NULL,				0x00				},
};

static HWND x_axis_ctl;
static HWND y_axis_ctl;
static HWND mouse_sens_ctl;
static HWND lag_ctl;

static void init_dropdown(HWND dd)
{
	SendMessage(dd, CB_ADDSTRING, 0, (LPARAM)get_string(IDS_IIDX_NONE));
	SendMessage(dd, CB_ADDSTRING, 0, (LPARAM)get_string(IDS_IIDX_1P_TT));
	SendMessage(dd, CB_ADDSTRING, 0, (LPARAM)get_string(IDS_IIDX_2P_TT));
}

static void init_dialog(HWND dialog)
{
	x_axis_ctl = GetDlgItem(dialog, IDC_IIDX_X_AXIS);
	y_axis_ctl = GetDlgItem(dialog, IDC_IIDX_Y_AXIS);
	mouse_sens_ctl = GetDlgItem(dialog, IDC_MOUSE_SENS);
	lag_ctl = GetDlgItem(dialog, IDC_TT_LAG);

	init_dropdown(x_axis_ctl);
	init_dropdown(y_axis_ctl);

	SendMessage(x_axis_ctl, CB_SETCURSEL, config_get_attr("x_axis"), 0);
	SendMessage(y_axis_ctl, CB_SETCURSEL, config_get_attr("y_axis"), 0);
	SendMessage(mouse_sens_ctl, TBM_SETRANGE, TRUE, MAKELONG(0, IIDX_MOUSE_DIVISOR - 1));
	SendMessage(mouse_sens_ctl, TBM_SETPOS, TRUE, config_get_attr("msens"));

	/* A checkbox is a type of button, apparently. Right. Because that makes perfect fucking sense */
	SendMessage(lag_ctl, BM_SETCHECK, config_get_attr("tt_lag") ? BST_CHECKED : BST_UNCHECKED, 0);
}

static LRESULT CALLBACK dialog_proc(HWND dialog, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_INITDIALOG) {
		init_dialog(dialog);
		return TRUE;
	}

	if (msg == WM_COMMAND && LOWORD(wparam) == IDC_IIDX_X_AXIS && HIWORD(wparam) == CBN_SELCHANGE) {
		config_set_attr("x_axis", SendMessage(x_axis_ctl, CB_GETCURSEL, 0, 0));
		return TRUE;
	}

	if (msg == WM_COMMAND && LOWORD(wparam) == IDC_IIDX_Y_AXIS && HIWORD(wparam) == CBN_SELCHANGE) {
		config_set_attr("y_axis", SendMessage(y_axis_ctl, CB_GETCURSEL, 0, 0));
		return TRUE;
	}

	if (msg == WM_HSCROLL) {
		/* Hmm. Don't know how to distinguish trackbars, but it doesn't matter here. */
		config_set_attr("msens", SendMessage(mouse_sens_ctl, TBM_GETPOS, 0, 0));
	}

	if (msg == WM_COMMAND && LOWORD(wparam) == IDC_TT_LAG && HIWORD(wparam) == BN_CLICKED) {
		config_set_attr("tt_lag", SendMessage(lag_ctl, BM_GETCHECK, 0, 0) == BST_CHECKED);
		return TRUE;
	}

	return FALSE;
}

HPROPSHEETPAGE iidx_create_pp(void)
{
	PROPSHEETPAGE psp;

	ZeroMemory(&psp, sizeof(psp));
	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = inst;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_IIDX);
	psp.pfnDlgProc = dialog_proc;

	return CreatePropertySheetPage(&psp);
}
