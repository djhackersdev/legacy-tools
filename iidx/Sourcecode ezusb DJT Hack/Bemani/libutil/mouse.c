#include "libutil/stdafx.h"

#include "libutil/dinput.h"
#include "libutil/mouse.h"

static IDirectInputDevice8 *mouse_dev;

void mouse_init(void)
{
	IDirectInput8 *api;
	HRESULT hr;

	api = dinput_get_api();
	hr = IDirectInput8_CreateDevice(api, &GUID_SysMouse, &mouse_dev, NULL);
	if (FAILED(hr)) {
		panic(L"Error opening system mouse: %#08x\n", hr);
	}

	hr = IDirectInputDevice8_SetCooperativeLevel(mouse_dev, 
		NULL, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) {
		panic(L"Error setting mouse co-op: %#08x\n", hr);
	}

	hr = IDirectInputDevice8_SetDataFormat(mouse_dev, &c_dfDIMouse);
	if (FAILED(hr)) {
		panic(L"Error settings mouse data format: %#08x\n", hr);
	}

	hr = IDirectInputDevice8_Acquire(mouse_dev);
	if (FAILED(hr)) {
		panic(L"Error acquiring system mouse: %#08x\n", hr);
	}

	trace(L"Opened system mouse\n");
}

void mouse_read(int *dx, int *dy)
{
	DIMOUSESTATE ms;
	HRESULT hr;

	hr = IDirectInputDevice8_GetDeviceState(mouse_dev, sizeof(ms), &ms);
	if (FAILED(hr)) {
		panic(L"Error polling mouse: %#08x\n", hr);
	}

	*dx = ms.lX;
	*dy = ms.lY;
}

void mouse_fini(void)
{
	IDirectInputDevice8_Unacquire(mouse_dev);
	IDirectInputDevice8_Release(mouse_dev);
}
