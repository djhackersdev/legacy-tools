#ifndef LIBDEVICE_DEFS_H
#define LIBDEVICE_DEFS_H

enum gfdm_action
{
	GFDM_TEST = 0x01,
	GFDM_SERVICE = 0x03,
	GFDM_DEBUG_AUTO = 0x40,
	GFDM_DEBUG_TIMER = 0x41,
	GFDM_EXIT = 0xFF
};

enum gf_action
{
	GF_1P_START = 0x02,
	GF_1P_WAIL = 0x06,
	GF_1P_RED = 0x09,
	GF_1P_GREEN = 0x0A,
	GF_1P_BLUE = 0x0B,

	GF_2P_START = 0x12,
	GF_2P_WAIL = 0x16,
	GF_2P_RED = 0x19,
	GF_2P_GREEN = 0x1A,
	GF_2P_BLUE = 0x1B,

	GF_1P_PICK_A = 0x20,
	GF_1P_PICK_B = 0x21,

	GF_2P_PICK_A = 0x22,
	GF_2P_PICK_B = 0x23,

	GF_1P_EFFECT = 0x30,
	GF_2P_EFFECT = 0x31
};

enum dm_action
{
	DM_START = 0x02,
	DM_HI_HAT = 0x05,
	DM_SNARE = 0x06,
	DM_HIGH_TOM = 0x07,
	DM_LOW_TOM = 0x08,
	DM_CYMBAL = 0x09,
	DM_BASS = 0x0B,
	DM_MENU_LEFT = 0x17,
	DM_MENU_RIGHT = 0x18,
};

#endif