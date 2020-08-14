#pragma once

#include <Windows.h>
#include <windowsx.h>
#include "SystemErrorHelpers.h"
#include "DirectXHelpers.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "dwrite")

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

// Enable Win32 Visual Styles
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif