#include "libutil/stdafx.h"

#include "config/config.h"
#include "libdevice/defs.h"

const struct action_desc gf_actions[] =
{
	{ L"Exit",			GFDM_EXIT			},
	{ L"Test",			GFDM_TEST			},
	{ L"Service",		GFDM_SERVICE		},
	{ L"1P Start",		GF_1P_START			},
	{ L"1P Red",		GF_1P_RED			},
	{ L"1P Green",		GF_1P_GREEN			},
	{ L"1P Blue",		GF_1P_BLUE			},
	{ L"1P Pick A",		GF_1P_PICK_A		},
	{ L"1P Pick B",		GF_1P_PICK_B		},
	{ L"1P Wail",		GF_1P_WAIL			},
	{ L"1P Effector",	GF_1P_EFFECT		},
	{ L"2P Start",		GF_2P_START			},
	{ L"2P Red",		GF_2P_RED			},
	{ L"2P Green",		GF_2P_GREEN			},
	{ L"2P Blue",		GF_2P_BLUE			},
	{ L"2P Pick A",		GF_2P_PICK_A		},
	{ L"2P Pick B",		GF_2P_PICK_B		},
	{ L"2P Wail",		GF_2P_WAIL			},
	{ L"2P Effector",	GF_2P_EFFECT		},
	{ L"Debug S/W: Auto Play", GFDM_DEBUG_AUTO },
	{ L"Debug S/W: Disable Menu Timer", GFDM_DEBUG_TIMER },
	{ NULL,				0					}
};

const struct action_desc dm_actions[] =
{
	{ L"Exit",			GFDM_EXIT			},
	{ L"Test",			GFDM_TEST			},
	{ L"Service",		GFDM_SERVICE		},
	{ L"Start",			DM_START			},
	{ L"Hi-Hat",		DM_HI_HAT			},
	{ L"Snare",			DM_SNARE			},
	{ L"High Tom",		DM_HIGH_TOM			},
	{ L"Low Tom",		DM_LOW_TOM			},
	{ L"Cymbal",		DM_CYMBAL			},
	{ L"Bass Drum",		DM_BASS				},
	{ L"Menu Left",		DM_MENU_LEFT		},
	{ L"Menu Right",	DM_MENU_RIGHT		},
	{ L"Debug S/W: Auto Play", GFDM_DEBUG_AUTO },
	{ L"Debug S/W: Disable Menu Timer", GFDM_DEBUG_TIMER },
	{ NULL,				0					}
};