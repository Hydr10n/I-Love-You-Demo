#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1effects.h>
#include <dwrite.h>
#include <math.h>

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

template <class Interface>
inline void SafeRelease(Interface** ppInterfaceToRelease) {
    if (*ppInterfaceToRelease != nullptr) {
        (*ppInterfaceToRelease)->Release();
        *ppInterfaceToRelease = nullptr;
    }
}

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr))
        throw hr;
}