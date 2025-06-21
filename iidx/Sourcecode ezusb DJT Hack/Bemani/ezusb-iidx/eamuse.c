#include "libutil/stdafx.h"
#include "libutil/eamuse.h"
#include "libutil/util.h"

DLLEXPORT int usbGetPCBID(u8_t *pcb_id)
{
	eam_read_machine_id(pcb_id);
	return 0;
}
