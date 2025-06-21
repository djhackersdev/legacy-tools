#include "libutil/stdafx.h"

#include "libutil/config.h"
#include "libutil/input.h"
#include "libutil/util.h"

HINSTANCE inst;

static void vtrace(const wchar_t *fmt, va_list ap)
{
	wchar_t buf[1024];
	int retval;

	retval = vswprintf_s(buf, sizeof(buf) / sizeof(wchar_t), fmt, ap);
	if (retval != -1) {
		OutputDebugString(buf);
	}
}

void libutil_init(const wchar_t *game_type)
{
	config_init(game_type);
	input_init();

	config_load();
}

void libutil_fini(void)
{
	input_fini();
	config_fini();
}

void trace(const wchar_t *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vtrace(fmt, ap);
	va_end(ap);
}

void panic(const wchar_t *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vtrace(fmt, ap);
	va_end(ap);

	exit(-1);
}

BOOL WINAPI DllMain(HINSTANCE instParam, DWORD reason, void *reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		inst = instParam;
	}

	return TRUE;
}
