#include "libutil/stdafx.h"

#include "libutil/dinput.h"
#include "libutil/keyboard.h"

#define KBD_LABEL_LEN 32

static IDirectInputDevice8 *kbd_dev;
static wchar_t kbd_names[KEYBOARD_NCONTROLS][KBD_LABEL_LEN];
static u8_t kbd_state[KEYBOARD_NCONTROLS];

void keyboard_init(void)
{
	IDirectInput8 *api;
	DIDEVICEOBJECTINSTANCE obj;
	HRESULT hr;
	int i;

	api = dinput_get_api();
	hr = IDirectInput8_CreateDevice(api, &GUID_SysKeyboard,
		&kbd_dev, NULL);
	if (FAILED(hr)) {
		panic(L"Error opening system keyboard: %#08x\n", hr);
	}

	hr = IDirectInputDevice8_SetCooperativeLevel(kbd_dev,
		NULL, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) {
		panic(L"Error setting keyboard co-op: %#08x\n", hr);
	}

	hr = IDirectInputDevice8_SetDataFormat(kbd_dev, &c_dfDIKeyboard);
	if (FAILED(hr)) {
		panic(L"Error setting keyboard data format: %#08x\n", hr);
	}

	for (i = 0 ; i < KEYBOARD_NCONTROLS ; i++) {
		memset(&obj, 0, sizeof(obj));
		obj.dwSize = sizeof(obj);

		hr = IDirectInputDevice8_GetObjectInfo(kbd_dev, &obj, i, DIPH_BYOFFSET);

		if (SUCCEEDED(hr)) {
			wcsncpy_s(kbd_names[i], KBD_LABEL_LEN, obj.tszName, _TRUNCATE);
		} else {
			kbd_names[i][0] = L'\0';
		}
	}

	hr = IDirectInputDevice8_Acquire(kbd_dev);
	if (FAILED(hr)) {
		panic(L"Error acquiring system keyboard: %#08x\n", hr);
	}

	trace(L"Opened system keyboard\n");
}

const wchar_t *keyboard_describe(u8_t control_id)
{
	const wchar_t *str;

	/* Return NULL if the control name is an empty string */
	str = kbd_names[control_id];
	return str[0] ? str : NULL;
}

const u8_t *keyboard_get_state(void)
{
	return kbd_state;
}

void keyboard_update(void)
{
	HRESULT hr;

	hr = IDirectInputDevice8_GetDeviceState(kbd_dev, KEYBOARD_NCONTROLS, kbd_state);
	if (FAILED(hr)) {
		panic(L"Error polling keyboard: %#08x\n", hr);
	}
}

void keyboard_fini(void)
{
	IDirectInputDevice8_Unacquire(kbd_dev);
	IDirectInputDevice8_Release(kbd_dev);

	kbd_dev = NULL;
}
