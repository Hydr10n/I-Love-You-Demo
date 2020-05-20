/*
Source File: I Love You Demo.c
Last Update: 2020/05/20

This project is hosted on https://github.com/Hydr10n/I-Love-You-Demo
Copyright (C) Programmer-Yang_Xun@outlook.com. All Rights Reserved.
*/

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <windowsx.h>
#include <math.h>

#define SYS_WHITE_BRUSH (HBRUSH)(COLOR_WINDOW + 1)
#define WM_PLAY WM_APP

#define FPS 90
#define TOTAL_DURATION 1000.0
#define FRAME_DURATION_UNIT (1000.0 / FPS)

#define MAIN_WINDOW_WIDTH 800
#define MAIN_WINDOW_HEIGHT 800
#define HEART_WIDTH MAIN_WINDOW_WIDTH
#define HEART_HEIGHT MAIN_WINDOW_HEIGHT

#define Scale(iPixels, iDPI) MulDiv(iPixels, iDPI, USER_DEFAULT_SCREEN_DPI)
#define TransitionVelocity(iSize, iMilliseconds, iFPS) (iSize / (iMilliseconds / 1000.0 * iFPS))

enum Identifiers {
	Identifier_Menu_Play,
	Identifier_Menu_Pause,
	Identifier_Menu_About,
	Identifier_Menu_Exit
};

int iDPI = USER_DEFAULT_SCREEN_DPI;

BOOL DrawHeart(HDC hDC, UINT uWidth, UINT uHeight, UINT uPaddingX, UINT uPaddingY);
LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_Heart(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
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
	HWND hWnd = CreateWindowExW(WS_EX_LAYERED, wndClass.lpszClassName, NULL,
		WS_POPUP,
		(GetSystemMetrics(SM_CXSCREEN) - size.cx) / 2, (GetSystemMetrics(SM_CYSCREEN) - size.cy) / 2, size.cx, size.cy,
		NULL, NULL, hInstance, NULL);
	SetLayeredWindowAttributes(hWnd, 0xffffff, 0, LWA_COLORKEY);
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
	static BOOL bPlay = TRUE;
	static HWND hWnd_Heart;
	switch (uMsg) {
	case WM_CREATE: {
		NONCLIENTMETRICSW nonClientMetrics;
		nonClientMetrics.cbSize = sizeof(nonClientMetrics);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(nonClientMetrics), &nonClientMetrics, 0);
		nonClientMetrics.lfMessageFont.lfHeight = -MulDiv(22, iDPI, 72);
		WNDCLASSW wndClass = { 0 };
		wndClass.hInstance = ((LPCREATESTRUCTW)lParam)->hInstance;
		wndClass.lpszClassName = L"Heart";
		wndClass.lpfnWndProc = WndProc_Heart;
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = SYS_WHITE_BRUSH;
		RegisterClassW(&wndClass);
		RECT rect;
		GetClientRect(hWnd, &rect);
		const SIZE heartSize = (SIZE){ Scale(HEART_WIDTH, iDPI), Scale(HEART_HEIGHT, iDPI) };
		hWnd_Heart = CreateWindowW(wndClass.lpszClassName, NULL,
			WS_CHILD | WS_VISIBLE,
			(rect.right - heartSize.cx) / 2, (rect.bottom - heartSize.cy) / 2, heartSize.cx, heartSize.cy,
			hWnd, NULL, ((LPCREATESTRUCTW)lParam)->hInstance, &nonClientMetrics.lfMessageFont);
	}	break;
	case WM_LBUTTONDOWN: SendMessageW(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0); break;
	case WM_RBUTTONDOWN: {
		HMENU hMenu = CreatePopupMenu();
		AppendMenuW(hMenu, MF_STRING, bPlay ? Identifier_Menu_Pause : Identifier_Menu_Play, bPlay ? L"Pause" : L"Play");
		AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(hMenu, MF_STRING, Identifier_Menu_About, L"About");
		AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(hMenu, MF_STRING, Identifier_Menu_Exit, L"Exit");
		SetForegroundWindow(hWnd);
		POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		ClientToScreen(hWnd, &point);
		TrackPopupMenu(hMenu, TPM_LEFTALIGN, point.x, point.y, 0, hWnd, NULL);
		DestroyMenu(hMenu);
	}	break;
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case Identifier_Menu_Play: case Identifier_Menu_Pause: {
			bPlay = !bPlay;
			SendMessageW(hWnd_Heart, WM_PLAY, bPlay, 0);
		}	break;
		case Identifier_Menu_About: MessageBoxW(hWnd,
			L"Copyright \xa9 2020 Programmer-Yang_Xun@outlook.com\n"
			L"          Welcome to visit https://github.com/Hydr10n",
			L"About I Love You Demo",
			MB_OK);
			break;
		case Identifier_Menu_Exit: PostQuitMessage(0); break;
		}
	}	break;
	case WM_DESTROY: PostQuitMessage(0); break;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_Heart(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static double fontHeight, fontHeightDelta, heartPaddingXDelta, heartPaddingYDelta;
	static HDC hDC_Memory;
	static HBITMAP hBitmap_Old;
	static LOGFONTW logFont;
	switch (uMsg) {
	case WM_CREATE: {
		logFont = *(PLOGFONTW)((LPCREATESTRUCTW)lParam)->lpCreateParams;
		logFont.lfHeight = -MulDiv(18, iDPI, 72);
		fontHeight = logFont.lfHeight;
		RECT rect;
		GetClientRect(hWnd, &rect);
		heartPaddingXDelta = TransitionVelocity(rect.right / 4, TOTAL_DURATION, FPS);
		heartPaddingYDelta = TransitionVelocity(rect.bottom / 4, TOTAL_DURATION, FPS);
		fontHeightDelta = TransitionVelocity(fontHeight / 4, TOTAL_DURATION, FPS);
		hDC_Memory = CreateCompatibleDC(NULL);
		SetTextColor(hDC_Memory, RGB(245, 245, 245));
		SetBkMode(hDC_Memory, TRANSPARENT);
		HDC hDC = GetDC(NULL);
		hBitmap_Old = SelectBitmap(hDC_Memory, CreateCompatibleBitmap(hDC, rect.right, rect.bottom));
		ReleaseDC(hWnd, hDC);
		SetTimer(hWnd, 1, (WPARAM)FRAME_DURATION_UNIT, NULL);
	}	break;
	case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: SendMessageW(GetAncestor(hWnd, GA_ROOT), uMsg, wParam, lParam); break;
	case WM_PLAY: {
		if (wParam)
			SetTimer(hWnd, 1, (WPARAM)FRAME_DURATION_UNIT, NULL);
		else
			KillTimer(hWnd, 1);
	}	break;
	case WM_TIMER: {
		RECT rect;
		GetClientRect(hWnd, &rect);
		FillRect(hDC_Memory, &rect, SYS_WHITE_BRUSH);
		static double heartPaddingX, heartPaddingY;
		if (!DrawHeart(hDC_Memory, rect.right, rect.bottom, (UINT)heartPaddingX, (UINT)heartPaddingY)) {
			PostQuitMessage(GetLastError());
			break;
		}
		HFONT hFont = CreateFontIndirectW(&logFont);
		SelectFont(hDC_Memory, hFont);
		DrawTextW(hDC_Memory, L"I LOVE YOU", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		DeleteFont(hFont);
		HDC hDC = GetDC(hWnd);
		BitBlt(hDC, 0, 0, rect.right, rect.bottom, hDC_Memory, 0, 0, SRCCOPY);
		ReleaseDC(hWnd, hDC);
		static BOOL bZoomIn;
		fontHeight += (bZoomIn ? 1 : -1) * fontHeightDelta;
		logFont.lfHeight = (LONG)fontHeight;
		heartPaddingX += (bZoomIn ? -1 : 1) * heartPaddingXDelta;
		heartPaddingY += (bZoomIn ? -1 : 1) * heartPaddingYDelta;
		static double frameDuration;
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
	const RECT rect = { 0, 0, uWidth, uHeight };
	const SIZE size = { rect.right - 2 * uPaddingX, rect.bottom - 2 * uPaddingY };
	const double pi = 3.14159265, radius = 1, width = radius * 4, height = radius + pi, ratio = width / height,
		magnification = floor(((double)size.cx / size.cy < ratio ? size.cx : size.cy * ratio) / width),
		actualRadius = floor(radius * magnification),
		originX = rect.right / 2.0, originY = rect.bottom / 2.0 - (height / 2 - radius) * magnification;
	const int cPoints = (int)(actualRadius * 4 + 1);
	HANDLE hHeap = GetProcessHeap();
	POINT* points = HeapAlloc(hHeap, 0, sizeof(POINT) * cPoints);
	if (points == NULL)
		return FALSE;
	int i = 0;
	for (double x = actualRadius * 2; x >= 0; x--, i++) {
		LONG y = (LONG)(((-acos(1 - 1 / magnification * x) + pi) * magnification) + originY);
		points[i] = (POINT){ (LONG)(originX - x), y };
		points[cPoints - i - 1] = (POINT){ (LONG)(originX + x), y };
	}
	HRGN hRgn = CreateEllipticRgn((int)(originX - actualRadius * 2), (int)(originY - actualRadius),
		(int)originX, (int)(originY + actualRadius)),
		hRgn2 = CreateEllipticRgn((int)originX + Scale(1, iDPI), (int)(originY - actualRadius),
			(int)(originX + actualRadius * 2 + Scale(1, iDPI)), (int)(originY + actualRadius)),
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
		{ 0, uPaddingY, ColorRed << 8, ColorGreen << 8, ColorBlue << 8 },
		{ rect.right, rect.bottom - uPaddingY, (ColorRed - 200) << 8, ColorGreen << 8, ColorBlue << 8 } };
	GdiGradientFill(hDC, triVertices, _countof(triVertices), &(GRADIENT_RECT){ 0, 1 }, 1, GRADIENT_FILL_RECT_V);
	return TRUE;
}