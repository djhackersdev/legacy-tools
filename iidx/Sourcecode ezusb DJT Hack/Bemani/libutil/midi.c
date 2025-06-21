#include "libutil/stdafx.h"

#include "libutil/input.h"
#include "libutil/midi.h"

#define NFRAMES 2

/* The Windows MIDI API is unreasonably simple and clear, for a Windows API or otherwise.
 * This is an outrage! I demand at least 100 lines of pointless boilerplate code! :P */

struct midi_device
{
	HMIDIIN hmidi;
	MIDIINCAPS caps;
	CRITICAL_SECTION cs;
	midi_state_t live;
	midi_state_t snap;
};

static struct midi_device *devs;
static int ndevs;

static void CALLBACK midi_callback(HMIDIIN handle, UINT dummy1, struct midi_device *dev, 
	u32_t msg, void *dummy)
{
	/* See http://recording.songstuff.com/articles.php?selected=55 for MIDI msg format.
	 * The message itself is passed in via the msg parameter */

	u8_t evt;
	u8_t pressure;
	u8_t code;

	/* Unpack event */
	evt = msg & 0xF0; /* Treat all channels the same */
	code = (u8_t) (msg >> 8);
	pressure = (u8_t) (msg >> 16);

	/* Update live state */
	if (evt == 0x90 && pressure > 0) {
		EnterCriticalSection(&dev->cs);
		if (dev->live[code] == 0) dev->live[code] = NFRAMES;
		LeaveCriticalSection(&dev->cs);
	}
}

void midi_init(void)
{
	MMRESULT retval;
	int i;

	ndevs = midiInGetNumDevs();
	devs = calloc(sizeof(struct midi_device), ndevs);

	for (i = 0 ; i < ndevs ; i++) {
		InitializeCriticalSection(&devs[i].cs);

		retval = midiInGetDevCaps(i, &devs[i].caps, sizeof(MIDIINCAPS));
		if (retval != MMSYSERR_NOERROR) {
			panic(L"Error getting capabilities of MIDI device #%d\n", i + 1);
		}

		retval = midiInOpen(&devs[i].hmidi, i, (DWORD_PTR) midi_callback, (DWORD_PTR) (devs + i), CALLBACK_FUNCTION);
		if (retval != MMSYSERR_NOERROR) {
			panic(L"Error opening MIDI device %s\n", devs[i].caps.szPname);
		}

		retval = midiInStart(devs[i].hmidi);
		if (retval != MMSYSERR_NOERROR) {
			panic(L"Error starting MIDI device %s\n", devs[i].caps.szPname);
		}

		trace(L"Opened %s\n", devs[i].caps.szPname);
	}
}

int midi_get_ndevs(void)
{
	return ndevs;
}

const wchar_t *midi_get_name(u8_t dev_id)
{
	if (dev_id >= ndevs) return NULL;
	return devs[dev_id].caps.szPname;
}

const wchar_t *midi_describe(u8_t dev_id, u8_t control_id)
{
	/* Static buffers are lazy, I know. Bite me. */
	static wchar_t buf[32];

	if (dev_id >= ndevs) {
		return NULL;
	}

	if (control_id >= MIDI_NCONTROLS) {
		return NULL;
	}

	swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"Note %02X", control_id);
	return buf;
}

const u8_t *midi_get_state(u8_t dev_id)
{
	return dev_id < ndevs ? devs[dev_id].snap : NULL;
}

void midi_update(void)
{
	int i;
	int j;

	for (i = 0 ; i < ndevs ; i++) {
		EnterCriticalSection(&devs[i].cs);

		/* Snapshot the live state */
		memcpy(devs[i].snap, devs[i].live, sizeof(midi_state_t));

		/* Decay the notes */
		for (j = 0 ; j < sizeof(midi_state_t) ; j++) {
			if (devs[i].live[j] > 0) {
				devs[i].live[j]--;
			}
		}

		LeaveCriticalSection(&devs[i].cs);
	}
}

void midi_fini(void)
{
	int i;

	for (i = 0 ; i < ndevs ; i++) {
		midiInStop(devs[i].hmidi);
		midiInClose(devs[i].hmidi);

		DeleteCriticalSection(&devs[i].cs);
	}

	free(devs);
	ndevs = 0;
}
