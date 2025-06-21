#ifndef LIBUTIL_KEYBOARD_H
#define LIBUTIL_KEYBOARD_H

#define KEYBOARD_NCONTROLS 256

void keyboard_init(void);
const wchar_t *keyboard_describe(u8_t control_id);
const u8_t *keyboard_get_state(void);
void keyboard_update(void);
void keyboard_fini(void);

#endif