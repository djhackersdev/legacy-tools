#ifndef LIBUTIL_INPUT_H
#define LIBUTIL_INPUT_H

#include "libutil/util.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (STDCALL *input_callback_t)(u8_t action, bool edge, void *ctx);

enum dev_class
{
	DEV_CLASS_KEYBOARD,
	DEV_CLASS_JOYSTICK,
	DEV_CLASS_MIDI,
	DEV_CLASS_MAX
};

struct input_code
{
	u8_t dev_class;
	u8_t dev_id;
	u8_t control_id;
};

struct input_binding
{
	struct input_code code;
	u8_t action;
};

void input_init(void);
LIBUTIL_API const wchar_t *input_describe(enum dev_class dev_class, u8_t dev_id, u8_t control_id);
LIBUTIL_API bool input_get_binding(enum dev_class dev_class, u8_t dev_id, u8_t control_id, u8_t *action);
const struct input_binding *input_get_bindings(void);
LIBUTIL_API const wchar_t *input_get_name(enum dev_class dev_class, u8_t dev_id);
int input_get_nbindings(void);
LIBUTIL_API int input_get_ncontrols(enum dev_class dev_class);
LIBUTIL_API int input_get_ndevs(enum dev_class dev_class);
LIBUTIL_API const u8_t *input_get_state(enum dev_class dev_class, u8_t dev_id);
LIBUTIL_API void input_scan(input_callback_t cb, void *ctx);
LIBUTIL_API void input_set_binding(enum dev_class dev_class, u8_t dev_id, u8_t control_id, u8_t action);
void input_set_bindings(struct input_binding *bindings, int nbindings);
LIBUTIL_API void input_unbind(enum dev_class dev_class, u8_t dev_id, u8_t control_id);
LIBUTIL_API void input_update(void);
void input_fini(void);

#ifdef __cplusplus
}
#endif

#endif