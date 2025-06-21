#include "libutil/stdafx.h"

#include "libutil/joystick.h"
#include "libutil/keyboard.h"
#include "libutil/mouse.h"
#include "libutil/dinput.h"
#include "libutil/util.h"

extern HINSTANCE inst; /* In dll.c */
static IDirectInput8 *api;

void dinput_init(void)
{
	HRESULT hr;

	hr = DirectInput8Create(inst, DIRECTINPUT_VERSION,
		&IID_IDirectInput8, &api, NULL);
	if (FAILED(hr)) {
		panic(L"DirectInput8Create failed: %#08x\n", hr);
	}
}

IDirectInput8 *dinput_get_api(void)
{
	return api;
}

void dinput_fini(void)
{
	IDirectInput8_Release(api);
}

