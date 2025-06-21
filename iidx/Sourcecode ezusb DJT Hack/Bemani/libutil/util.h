#ifndef LIBUTIL_UTIL_H
#define LIBUTIL_UTIL_H

#define LIBUTIL_VERSION 1

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#define STDCALL __stdcall

#ifndef LIBUTIL_EXPORT
#define LIBUTIL_API DLLEXPORT
#else
#define LIBUTIL_API DLLIMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef  __cplusplus
typedef enum { true = 1, false = 0 } bool;
#endif

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;

typedef signed char s8_t;
typedef signed short s16_t;
typedef signed int s32_t;

LIBUTIL_API void libutil_init(const wchar_t *game_type);
LIBUTIL_API void libutil_fini(void);
LIBUTIL_API void config_save(void);

LIBUTIL_API void trace(const wchar_t *fmt, ...);
LIBUTIL_API void panic(const wchar_t *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif