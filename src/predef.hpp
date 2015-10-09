
#pragma once

#include "stage/meta.hpp"

#define _PECAR_POSIX       1
#define _PECAR_WIN         2
#define _PECAR_EMBED       3

#define _PECAR_LINUX       11
#define _PECAR_WINDOWS_PC  12
#define _PECAR_OSX         13

#define _PECAR_ANDROID         101
#define _PECAR_WINDOWS_PHONE   102
#define _PECAR_IOS             103

// follows are in planning:
#define _PECAR_OPENWRT         201
#define _PECAR_DDWRT           202
#define _PECAR_COMWARE         203
#define _PECAR_CISCO_IOS       204

#ifdef __linux__
#   define _PECAR_PLATFORM _PECAR_LINUX
#elif defined(__WIN32__)
#   define _PECAR_PLATFORM _PECAR_WINDOWS_PC
#elif defined(__OSX__)
#   define _PECAR_PLATFORM _PECAR_OSX
#elif defined(__ANDROID__)
#   define _PECAR_PLATFORM _PECAR_ANDROID
#elif defined(WINAPI) and defined(WINAPI_FAMILY_PHONE_APP) and \
    (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#   define _PECAR_PLATFORM _PECAR_WINDOWS_PHONE
#elif defined(TARGET_OS_IPHONE)
#   define _PECAR_PLATFORM _PECAR_IOS
#endif

#define _PECAR_IS_LINUX            (_PECAR_PLATFORM == _PECAR_LINUX)
#define _PECAR_IS_WINDOWS_PC       (_PECAR_PLATFORM == _PECAR_WINDOWS_PC)
#define _PECAR_IS_OSX              (_PECAR_PLATFORM == _PECAR_OSX)
#define _PECAR_IS_ANDROID          (_PECAR_PLATFORM == _PECAR_ANDROID)
#define _PECAR_IS_WINDOWS_PHONE    (_PECAR_PLATFORM == _PECAR_WINDOWS_PHONE)
#define _PECAR_IS_IOS              (_PECAR_PLATFORM == _PECAR_IOS)

#define _PECAR_IS_POSIX            (_PECAR_IS_LINUX or _PECAR_IS_OSX or _PECAR_IS_ANDROID or _PECAR_IS_IOS)
#define _PECAR_IS_WINDOWS          (_PECAR_IS_WINDOWS_PC or _PECAR_IS_WINDOWS_PHONE)
