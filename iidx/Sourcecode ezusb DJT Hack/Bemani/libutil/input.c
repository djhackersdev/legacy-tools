#include "libutil/stdafx.h"

#include "libutil/dinput.h"
#include "libutil/input.h"
#include "libutil/joystick.h"
#include "libutil/keyboard.h"
#include "libutil/midi.h"
#include "libutil/mouse.h"

static const char config_file[] = "config.dat";
static u8_t state[16];
static u8_t state_prev[16];
static struct input_binding *bindings;
static int nbindings;

/*
 * Binding utils
 */

static int binding_cmp(const void *p1, const void *p2)
{
	const struct input_binding *lhs = p1;
	const struct input_binding *rhs = p2;
	int diff;

	diff = lhs->code.dev_class - rhs->code.dev_class;
	if (diff != 0) return diff;

	diff = lhs->code.dev_id - rhs->code.dev_id;
	if (diff != 0) return diff;

	diff = lhs->code.control_id - rhs->code.control_id;
	return diff;
}

static void input_add_binding(enum dev_class dev_class, u8_t dev_id, u8_t control_id, int action)
{
	/* Expand bindings[] and perform an insert, preserving binding_cmp() order */

	struct input_binding new_binding;
	struct input_binding *new_bindings;
	int i;

	new_binding.code.dev_class = dev_class;
	new_binding.code.dev_id = dev_id;
	new_binding.code.control_id = control_id;
	new_binding.action = action;

	new_bindings = malloc(sizeof(struct input_binding) * (nbindings + 1));

	for (i = 0 ; i < nbindings && binding_cmp(&bindings[i], &new_binding) < 0 ; i++) {
		new_bindings[i] = bindings[i];
	}

	new_bindings[i] = new_binding;

	for ( ; i < nbindings ; i++) {
		new_bindings[i + 1] = bindings[i];
	}

	free(bindings);

	bindings = new_bindings;
	nbindings++;
}

/*
 * Public API
 */

void input_init(void)
{
	dinput_init();
	joystick_init();
	keyboard_init();
	midi_init();
	mouse_init();
}

const wchar_t *input_describe(enum dev_class dev_class, u8_t dev_id, u8_t control_id)
{
	switch (dev_class) {
		case DEV_CLASS_KEYBOARD:
			return keyboard_describe(control_id);

		case DEV_CLASS_JOYSTICK:
			return joystick_describe(dev_id, control_id);

		case DEV_CLASS_MIDI:
			return midi_describe(dev_id, control_id);

		default:
			return NULL;
	}
}

bool input_get_binding(enum dev_class dev_class, u8_t dev_id, u8_t control_id, u8_t *action)
{
	struct input_binding *binding;
	struct input_binding dummy;

	dummy.code.dev_class = dev_class;
	dummy.code.dev_id = dev_id;
	dummy.code.control_id = control_id;
	binding = bsearch(&dummy, bindings, nbindings, sizeof(struct input_binding), binding_cmp);

	if (binding) {
		*action = binding->action;
		return true;
	} else {
		return false;
	}
}

const struct input_binding *input_get_bindings(void)
{
	return bindings;
}

const wchar_t *input_get_name(enum dev_class dev_class, u8_t dev_id)
{
	switch (dev_class) {
		case DEV_CLASS_KEYBOARD:
			return L"Keyboard";

		case DEV_CLASS_JOYSTICK:
			return joystick_get_name(dev_id);

		case DEV_CLASS_MIDI:
			return midi_get_name(dev_id);

		default:
			return NULL;
	}
}

int input_get_nbindings(void)
{
	return nbindings;
}

int input_get_ncontrols(enum dev_class dev_class)
{
	switch (dev_class) {
		case DEV_CLASS_KEYBOARD:
			return KEYBOARD_NCONTROLS;

		case DEV_CLASS_JOYSTICK:
			return JOYSTICK_NCONTROLS;

		case DEV_CLASS_MIDI:
			return MIDI_NCONTROLS;

		default:
			return 0;
	}
}

int input_get_ndevs(enum dev_class dev_class)
{
	switch (dev_class) {
		case DEV_CLASS_KEYBOARD:
			return 1;

		case DEV_CLASS_JOYSTICK:
			return joystick_get_ndevs();

		case DEV_CLASS_MIDI:
			return midi_get_ndevs();

		default:
			return 0;
	}
}

const u8_t *input_get_state(enum dev_class dev_class, u8_t dev_id)
{
	switch (dev_class) {
		case DEV_CLASS_KEYBOARD:
			return dev_id == 0 ? keyboard_get_state() : NULL;

		case DEV_CLASS_JOYSTICK:
			return joystick_get_state(dev_id);

		case DEV_CLASS_MIDI:
			return midi_get_state(dev_id);

		default:
			return NULL;
	}
}

void input_scan(input_callback_t cb, void *ctx)
{
	int i;
	u8_t bit;
	u8_t byte;
	const u8_t *dev_state;
	
	input_update();
	memset(state, 0, sizeof(state));

	for (i = 0 ; i < nbindings ; i++) {
		/* Get state of bound control */
		switch (bindings[i].code.dev_class) {
			case DEV_CLASS_KEYBOARD:
				dev_state = keyboard_get_state();
				break;

			case DEV_CLASS_JOYSTICK:
				dev_state = joystick_get_state(bindings[i].code.dev_id);
				break;

			case DEV_CLASS_MIDI:
				dev_state = midi_get_state(bindings[i].code.dev_id);				
				break;
		}

		if (dev_state != NULL && dev_state[bindings[i].code.control_id]) {
			/* Locate action bit in state[] */
			bit = 1 << (bindings[i].action % 8);
			byte = bindings[i].action / 8;

			/* Fire callback, set action state bit */
			cb(bindings[i].action, (state_prev[byte] & bit) == 0, ctx);
			state[byte] |= bit;
		}
	}

	memcpy(state_prev, state, sizeof(state));
}

void input_set_binding(enum dev_class dev_class, u8_t dev_id, u8_t control_id, u8_t action)
{
	struct input_binding dummy;
	struct input_binding *binding;

	trace(L"input_set_binding(%d, %d, %d, %d)\n", dev_class, dev_id, control_id, action);

	dummy.code.dev_class = dev_class;
	dummy.code.dev_id = dev_id;
	dummy.code.control_id = control_id;
	binding = bsearch(&dummy, bindings, nbindings, sizeof(struct input_binding), binding_cmp);

	if (binding != NULL) {
		binding->action = action;
	} else {
		input_add_binding(dev_class, dev_id, control_id, action);
	}
}

void input_set_bindings(struct input_binding *new_bindings, int new_nbindings)
{
	free(bindings);

	bindings = new_bindings;
	nbindings = new_nbindings;
}

void input_unbind(enum dev_class dev_class, u8_t dev_id, u8_t control_id)
{
	struct input_binding dummy;
	int i;

	trace(L"input_unbind(%d, %d, %d)\n", dev_class, dev_id, control_id);

	dummy.code.dev_class = dev_class;
	dummy.code.dev_id = dev_id;
	dummy.code.control_id = control_id;

	/* Locate the offending binding */
	for (i = 0 ; i < nbindings && binding_cmp(&bindings[i], &dummy) != 0 ; i++);

	/* Move everything else down to cover the gap */
	if (i < nbindings) {
		for ( ; i + 1 < nbindings ; i++) {
			bindings[i] = bindings[i + 1];
		}

		nbindings--;
	}
}

void input_update(void)
{
	joystick_update();
	keyboard_update();
	midi_update();
}

void input_fini(void)
{
	free(bindings);
	bindings = NULL;
	nbindings = 0;

	mouse_fini();
	midi_fini();
	keyboard_fini();
	joystick_fini();
	dinput_fini();
}
