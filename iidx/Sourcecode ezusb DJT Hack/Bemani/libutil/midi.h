#ifndef LIBUTIL_MIDI_H
#define LIBUTIL_MIDI_H

#include "libutil/util.h"
#include "libutil/input.h"

#define MIDI_NCONTROLS 127

typedef u8_t midi_state_t[MIDI_NCONTROLS];

void midi_init(void);
int midi_get_ndevs(void);
const wchar_t *midi_get_name(u8_t dev_id);
const wchar_t *midi_describe(u8_t dev_id, u8_t control_id);
const u8_t *midi_get_state(u8_t dev_id);
void midi_update(void);
void midi_fini(void);

#endif