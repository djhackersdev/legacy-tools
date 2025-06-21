#ifndef LIBUTIL_JOYSTICK_H
#define LIBUTIL_JOYSTICK_H

#include "libutil/util.h"

#define JOYSTICK_NCONTROLS 26

void joystick_init(void);
const wchar_t *joystick_describe(u8_t dev_id, u8_t control_id);
const wchar_t *joystick_get_name(u8_t dev_id);
int joystick_get_ndevs(void);
const u8_t *joystick_get_state(u8_t dev_id);
void joystick_update(void);
void joystick_fini(void);

#endif