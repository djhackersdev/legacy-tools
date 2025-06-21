#include "libutil/stdafx.h"

#include "libutil/dinput.h"
#include "libutil/joystick.h"
#include "libutil/util.h"

#define CENTER 32767
#define HAT_RIGHT 9000
#define HAT_LEFT 27000
#define HAT_DOWN 18000
#define HAT_UP 0

static const wchar_t *joy_names[] =
{
	L"Right",
	L"Left",
	L"Up",
	L"Down",
	L"Rudder Left",
	L"Rudder Right",
	L"POV Right",
	L"POV Left",
	L"POV Up",
	L"POV Down",
	L"Button 1",
	L"Button 2",
	L"Button 3",
	L"Button 4",
	L"Button 5",
	L"Button 6",
	L"Button 7",
	L"Button 8",
	L"Button 9",
	L"Button 10",
	L"Button 11",
	L"Button 12",
	L"Button 13",
	L"Button 14",
	L"Button 15",
	L"Button 16"
};

enum joy_control
{
	JSC_RIGHT,
	JSC_LEFT,
	JSC_UP,
	JSC_DOWN,
	JSC_RUDDER_RIGHT,
	JSC_RUDDER_LEFT,
	JSC_POV_RIGHT,
	JSC_POV_LEFT,
	JSC_POV_UP,
	JSC_POV_DOWN,
	JSC_BUTTON1
};

struct joystick
{
	IDirectInputDevice8 *dev;
	u8_t state[JOYSTICK_NCONTROLS];
	wchar_t name[64];
};

static struct joystick *sticks;
static u8_t nsticks;

static BOOL CALLBACK joystick_enum_cb(const DIDEVICEINSTANCE *dinst, void *ctx)
{
	IDirectInput8 *api;
	IDirectInputDevice8 *dev;
	HRESULT hr;
	int i;

	api = dinput_get_api();
	hr = IDirectInput8_CreateDevice(api, &dinst->guidInstance,
		&dev, NULL);
	
	if (FAILED(hr)) {
		trace(L"Error opening %s: %#08x\n", dinst->tszInstanceName, hr);
		return DIENUM_CONTINUE;
	}

	/* Allocate a new stick */
	i = nsticks++;
	sticks = realloc(sticks, sizeof(struct joystick) * nsticks);
	memset(&sticks[i], 0, sizeof(struct joystick));

	/* Populate stick state */
	sticks[i].dev = dev;
	wcsncpy_s(sticks[i].name, sizeof(sticks[i].name) / sizeof(wchar_t), dinst->tszInstanceName,
		_TRUNCATE);

	/* DirectInput bureaucratic crap */
	hr = IDirectInputDevice8_SetCooperativeLevel(sticks[i].dev,
		NULL, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) {
		panic(L"Error setting co-op for %s: %#08x\n", sticks[i].name, hr);
	}

	hr = IDirectInputDevice8_SetDataFormat(sticks[i].dev, &c_dfDIJoystick);
	if (FAILED(hr)) {
		panic(L"Error setting format for %s: %#08x\n", sticks[i].name, hr);
	}

	hr = IDirectInputDevice8_Acquire(sticks[i].dev);
	if (FAILED(hr)) {
		panic(L"Error acquiring %s: %#08x\n", sticks[i].name, hr);
	}

	trace(L"Opened %s\n", sticks[i].name);
	return DIENUM_CONTINUE;
}

void joystick_init(void)
{
	IDirectInput8 *api;
	HRESULT hr;

	api = dinput_get_api();
	hr = IDirectInput8_EnumDevices(api, DI8DEVCLASS_GAMECTRL,
		joystick_enum_cb, NULL, DIEDFL_ALLDEVICES);
	if (FAILED(hr)) {
		panic(L"Error enumerating joysticks: %#08x\n", hr);
	}
}

const wchar_t *joystick_describe(u8_t dev_id, u8_t control_id)
{
	if (dev_id >= nsticks) {
		return NULL;
	}

	if (control_id >= JOYSTICK_NCONTROLS) {
		return NULL;
	}

	return joy_names[control_id];
}

const wchar_t *joystick_get_name(u8_t dev_id)
{
	return dev_id < nsticks ? sticks[dev_id].name : NULL;
}

int joystick_get_ndevs(void)
{
	return nsticks;
}

const u8_t *joystick_get_state(u8_t dev_id)
{
	if (dev_id >= nsticks) {
		return NULL;
	} else {
		return sticks[dev_id].state;
	}
}

void joystick_update(void)
{
	HRESULT hr;
	DIJOYSTATE js;
	u8_t *s;
	int i;

	for (i = 0 ; i < nsticks ; i++) {
		/* Poll stick */
		hr = IDirectInputDevice8_Poll(sticks[i].dev);
		if (FAILED(hr)) {
			trace(L"Error polling %s: %08x\n", sticks[i].name, hr);
		}

		/* Get raw state */
		hr = IDirectInputDevice8_GetDeviceState(sticks[i].dev,
			sizeof(DIJOYSTATE), &js);
		if (FAILED(hr)) {
			trace(L"Error retrieving device state for %s: %08x\n", sticks[i].name, hr);
		}

		/* Clear translated state */
		s = sticks[i].state;
		memset(s, 0, JOYSTICK_NCONTROLS);

		/* Translate axes into +ve and -ve virtual buttons */
		if (js.lX > CENTER) s[JSC_RIGHT] = true;
		if (js.lX < CENTER) s[JSC_LEFT] = true;
		if (js.lY > CENTER) s[JSC_UP] = true;
		if (js.lY < CENTER) s[JSC_DOWN] = true;
		if (js.lZ > CENTER) s[JSC_RUDDER_RIGHT] = true;
		if (js.lZ < CENTER) s[JSC_RUDDER_LEFT] = true;
		if (js.rgdwPOV[0] == HAT_RIGHT) s[JSC_POV_RIGHT] = true;
		if (js.rgdwPOV[0] == HAT_LEFT) s[JSC_POV_LEFT] = true;
		if (js.rgdwPOV[0] == HAT_UP) s[JSC_POV_UP] = true;
		if (js.rgdwPOV[0] == HAT_DOWN) s[JSC_POV_DOWN] = true;

		/* Copy button state */
		memcpy(&s[JSC_BUTTON1], js.rgbButtons, 16);
	}
}

void joystick_fini(void)
{
	int i;

	for (i = 0 ; i < nsticks ; i++) {
		IDirectInputDevice8_Unacquire(sticks[i].dev);
		IDirectInputDevice8_Release(sticks[i].dev);
	}

	free(sticks);
	nsticks = 0;
}
