/*
Source File: I Love You Demo.c
Last Update: 2020/02/14

This project is hosted on https://github.com/Hydr10n/I-Love-You-Demo
Copyright (C) Programmer-Yang_Xun@outlook.com. All Rights Reserved.
*/

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#include <Windows.h>
#include <windowsx.h>
#include <math.h>

#pragma warning(disable:6386)
#pragma warning(disable:26451)
#pragma warning(disable:28252)

#define PI 3.14159265
#define FPS 90
#define TOTAL_DURATION 1000.0
#define FRAME_DURATION_UNIT (1000.0 / FPS)
#define SYS_WHITE_BRUSH (HBRUSH)(COLOR_WINDOW + 1)

#define MAIN_WINDOW_WIDTH 1000
#define MAIN_WINDOW_HEIGHT 800
#define HEART_WIDTH 680
#define HEART_HEIGHT 680

#define Scale(iPixels, iDPI) MulDiv(iPixels, iDPI, USER_DEFAULT_SCREEN_DPI)

int iDPI = USER_DEFAULT_SCREEN_DPI;

BOOL DrawHeart(HDC hDC, UINT uWidth, UINT uHeight, UINT uPaddingX, UINT uPaddingY);
LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_Heart(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	SIZE size = { MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT };
	HDC hDC = GetDC(NULL);
	if (hDC) {
		SetProcessDPIAware();
		iDPI = GetDeviceCaps(hDC, LOGPIXELSX);
		size = (SIZE){ Scale(size.cx, iDPI), Scale(size.cy, iDPI) };
		ReleaseDC(NULL, hDC);
	}
	WNDCLASSW wndClass = { 0 };
	wndClass.lpszClassName = L"Win32";
	wndClass.lpfnWndProc = WndProc_Main;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = SYS_WHITE_BRUSH;
	RegisterClassW(&wndClass);
	HWND hWnd = CreateWindowW(wndClass.lpszClassName, L"I Love You Demo ( https://github.com/Hydr10n/I-Love-You-Demo )",
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		(GetSystemMetrics(SM_CXSCREEN) - size.cx) / 2, (GetSystemMetrics(SM_CYSCREEN) - size.cy) / 2, size.cx, size.cy,
		NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);
	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HFONT hFont;
	switch (uMsg) {
	case WM_CREATE: {
		NONCLIENTMETRICSW nonClientMetrics;
		nonClientMetrics.cbSize = sizeof(nonClientMetrics);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(nonClientMetrics), &nonClientMetrics, 0);
		nonClientMetrics.lfMessageFont.lfHeight = -MulDiv(22, iDPI, 72);
		hFont = CreateFontIndirectW(&nonClientMetrics.lfMessageFont);
		RECT rect;
		GetClientRect(hWnd, &rect);
		SIZE heartSize = (SIZE){ Scale(HEART_WIDTH, iDPI), Scale(HEART_HEIGHT, iDPI) };
		WNDCLASSW wndClass = { 0 };
		wndClass.hInstance = ((LPCREATESTRUCTW)lParam)->hInstance;
		wndClass.lpszClassName = L"Heart";
		wndClass.lpfnWndProc = WndProc_Heart;
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = SYS_WHITE_BRUSH;
		RegisterClassW(&wndClass);
		CreateWindowW(wndClass.lpszClassName, NULL,
			WS_CHILD | WS_VISIBLE,
			(rect.right - heartSize.cx) / 2, (rect.bottom - heartSize.cy) / 2, heartSize.cx, heartSize.cy,
			hWnd, NULL, ((LPCREATESTRUCTW)lParam)->hInstance, &nonClientMetrics.lfMessageFont);
	}	break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);
		RECT rect;
		GetClientRect(hWnd, &rect);
		SelectFont(hDC, hFont);
		SetBkMode(hDC, TRANSPARENT);
		SetTextColor(hDC, RGB(0, 102, 204));
		DrawTextW(hDC, L"Copyright \xa9 Programmer-Yang_Xun@outlook.com", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_BOTTOM);
		EndPaint(hWnd, &ps);
	}	break;
	case WM_DESTROY: {
		DeleteFont(hFont);
		PostQuitMessage(0);
	}	break;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_Heart(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static BOOL bZoomIn;
	static double frameDuration, fontHeight, fontHeightDelta, heartPaddingX, heartPaddingY, heartPaddingXDelta, heartPaddingYDelta;
	static HDC hDC_Memory;
	static HBITMAP hBitmap_Old;
	static LOGFONTW logFont;
	switch (uMsg) {
	case WM_CREATE: {
		logFont = *(PLOGFONTW)((LPCREATESTRUCTW)lParam)->lpCreateParams;
		logFont.lfHeight = -MulDiv(18, iDPI, 72);
		fontHeight = logFont.lfHeight;
		hDC_Memory = CreateCompatibleDC(NULL);
		SetTextColor(hDC_Memory, RGB(245, 245, 245));
		SetBkMode(hDC_Memory, TRANSPARENT);
		HDC hDC = GetDC(hWnd);
		hBitmap_Old = SelectBitmap(hDC_Memory, CreateCompatibleBitmap(hDC, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)));
		ReleaseDC(hWnd, hDC);
		RECT rect;
		GetClientRect(hWnd, &rect);
		heartPaddingXDelta = FRAME_DURATION_UNIT / TOTAL_DURATION * rect.right / 4;
		heartPaddingYDelta = FRAME_DURATION_UNIT / TOTAL_DURATION * rect.bottom / 4;
		fontHeightDelta = FRAME_DURATION_UNIT / TOTAL_DURATION * fontHeight / 4;
		SetTimer(hWnd, 1, (WPARAM)FRAME_DURATION_UNIT, NULL);
	}	break;
	case WM_TIMER: {
		RECT rect;
		GetClientRect(hWnd, &rect);
		FillRect(hDC_Memory, &rect, SYS_WHITE_BRUSH);
		if (!DrawHeart(hDC_Memory, rect.right, rect.bottom, (UINT)heartPaddingX, (UINT)heartPaddingY)) {
			PostQuitMessage(GetLastError());
			break;
		}
		HFONT hFont = CreateFontIndirectW(&logFont);
		SelectFont(hDC_Memory, hFont);
		DrawTextW(hDC_Memory, L"I LOVE YOU!!!", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		DeleteFont(hFont);
		HDC hDC = GetDC(hWnd);
		BitBlt(hDC, 0, 0, rect.right, rect.bottom, hDC_Memory, 0, 0, SRCCOPY);
		ReleaseDC(hWnd, hDC);
		fontHeight += (bZoomIn ? 1 : -1) * fontHeightDelta;
		logFont.lfHeight = (LONG)fontHeight;
		heartPaddingX += (bZoomIn ? -1 : 1) * heartPaddingXDelta;
		heartPaddingY += (bZoomIn ? -1 : 1) * heartPaddingYDelta;
		if ((frameDuration += FRAME_DURATION_UNIT) >= TOTAL_DURATION) {
			frameDuration = 0;
			bZoomIn = !bZoomIn;
		}
	}	break;
	case WM_DESTROY: {
		DeleteBitmap(SelectBitmap(hDC_Memory, hBitmap_Old));
		DeleteDC(hDC_Memory);
	}	break;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

BOOL DrawHeart(HDC hDC, UINT uWidth, UINT uHeight, UINT uPaddingX, UINT uPaddingY) {
	/*
	Math Functions:
		f(x) = sqrt(1 - (|x| - 1) ^ 2)
		g(x) = arccos(1 - |x|) - Pi
		x: [-2, 2]
	*/
	const double radius = 1, width = radius * 4, height = radius + PI, ratio = width / height;
	const RECT rect = { 0, 0, uWidth, uHeight };
	const SIZE size = { rect.right - 2 * uPaddingX, rect.bottom - 2 * uPaddingY };
	const double magnification = ((double)size.cx / size.cy < ratio ? size.cx : size.cy * ratio) / width,
		actualRadius = radius * magnification,
		originX = rect.right / 2.0,
		originY = rect.bottom / 2.0 - (LONG)((height / 2 - radius) * magnification);
	const int cPoints = ((int)actualRadius * 2 + 1) * 2;
	HANDLE hHeap = GetProcessHeap();
	POINT* points = HeapAlloc(hHeap, 0, sizeof(POINT) * cPoints);
	if (points == NULL)
		return FALSE;
	int i = 0;
	for (double x = actualRadius * 2; x >= 0; x--, i++) {
		double y = ((-acos(1 - 1 / magnification * x) + PI) * magnification) + originY;
		points[i] = (POINT){ (LONG)(originX - x), (LONG)y };
		points[cPoints - i - 1] = (POINT){ (LONG)(originX + x), (LONG)y };
	}
	HRGN hRgn = CreateEllipticRgn((LONG)(originX - actualRadius * 2), (LONG)(originY - actualRadius),
		(LONG)originX, (LONG)(originY + actualRadius)),
		hRgn2 = CreateEllipticRgn((LONG)originX + Scale(1, iDPI), (LONG)(originY - actualRadius),
		(LONG)(originX + actualRadius * 2 + Scale(1, iDPI)), (LONG)(originY + actualRadius)),
		hRgn3 = CreatePolygonRgn(points, cPoints, ALTERNATE);
	CombineRgn(hRgn, hRgn, hRgn2, RGN_OR);
	CombineRgn(hRgn, hRgn, hRgn3, RGN_OR);
	SelectClipRgn(hDC, hRgn);
	DeleteRgn(hRgn3);
	DeleteRgn(hRgn2);
	DeleteRgn(hRgn);
	HeapFree(hHeap, 0, points);
	const short ColorRed = 240, ColorGreen = 40, ColorBlue = 100;
	TRIVERTEX triVertices[] = {
		{ 0, 0, ColorRed << 8, ColorGreen << 8, ColorBlue << 8 },
		{ rect.right, rect.bottom, (ColorRed - 200) << 8, ColorGreen << 8, ColorBlue << 8 } };
	GdiGradientFill(hDC, triVertices, _countof(triVertices), &(GRADIENT_RECT){ 0, 1 }, 1, GRADIENT_FILL_RECT_V);
	return TRUE;
}