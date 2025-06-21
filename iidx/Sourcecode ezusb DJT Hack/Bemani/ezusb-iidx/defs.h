#ifndef EZUSB_IIDX_DEFS_H
#define EZUSB_IIDX_DEFS_H

#define IIDX_MOUSE_DIVISOR 10

enum iidx_action
{
	/* usbReadPad() bit mappings (0x00 - 0x20) */

	P1_1 = 0x08,
	P1_2 = 0x09,
	P1_3 = 0x0A,
	P1_4 = 0x0B,
	P1_5 = 0x0C,
	P1_6 = 0x0D,
	P1_7 = 0x0E,
	
	P2_1 = 0x0F,
	P2_2 = 0x10,
	P2_3 = 0x11,
	P2_4 = 0x12,
	P2_5 = 0x13,
	P2_6 = 0x14,
	P2_7 = 0x15,
	P2_8 = 0x16,

	P1_START = 0x18,
	P2_START = 0x19,
	VEFX = 0x1A,
	EFFECT = 0x1B,
	TEST = 0x1C,
	SERVICE = 0x1D,

	/* Synthetic inputs for turntable, sliders, etc */

	P1_TT_UP = 0x20,
	P1_TT_DOWN = 0x21,
	P1_TT_FILTERED_UP = 0x22,
	P1_TT_FILTERED_DOWN = 0x23,
	P1_TT_STAB = 0x24,

	P2_TT_UP = 0x25,
	P2_TT_DOWN = 0x26,
	P2_TT_FILTERED_UP = 0x27,
	P2_TT_FILTERED_DOWN = 0x28,
	P2_TT_STAB = 0x29,

	SLIDER0_UP = 0x2A,
	SLIDER0_DOWN = 0x2B,
	SLIDER1_UP = 0x2C,
	SLIDER1_DOWN = 0x2D,
	SLIDER2_UP = 0x2E,
	SLIDER2_DOWN = 0x2F,
	SLIDER3_UP = 0x30,
	SLIDER3_DOWN = 0x31,
	SLIDER4_UP = 0x32,
	SLIDER4_DOWN = 0x33,

	EXIT = 0xFF
};

#endif