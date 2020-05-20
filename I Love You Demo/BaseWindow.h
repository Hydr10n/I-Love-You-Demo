#pragma once

#include <Windows.h>

template <class DerivedWindow>
class BaseWindow {
public:
	HWND GetWindowHandle() const { return m_hWnd; }

protected:
	HWND m_hWnd = NULL;
	WNDCLASSEXW wndClassEx = { sizeof(wndClassEx) };

	BaseWindow() {
		wndClassEx.lpszClassName = L"BaseWindow";
		wndClassEx.lpfnWndProc = WindowProc;
		wndClassEx.hInstance = GetModuleHandleW(NULL);
	}

	void SetWndClassEx(LPCWSTR lpszClassName, HICON hIcon = NULL, HCURSOR hCursor = NULL, HBRUSH hbrBackground = NULL, UINT style = 0, int cbClsExtra = 0, int cbWndExtra = 0, LPCWSTR lpszMenuName = NULL, HICON hIconSm = NULL) {
		wndClassEx.lpszClassName = lpszClassName;
		wndClassEx.hIcon = hIcon;
		wndClassEx.hCursor = hCursor;
		wndClassEx.hbrBackground = hbrBackground;
		wndClassEx.style = style;
		wndClassEx.cbClsExtra = cbClsExtra;
		wndClassEx.cbWndExtra = cbWndExtra;
		wndClassEx.lpszMenuName = lpszMenuName;
		wndClassEx.hIconSm = hIconSm;
	}

	BOOL Initialize(DWORD dwExStyle, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu) {
		RegisterClassExW(&wndClassEx);
		m_hWnd = CreateWindowExW(dwExStyle, wndClassEx.lpszClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, wndClassEx.hInstance, this);
		BOOL ret = m_hWnd != NULL;
		if (!ret)
			UnregisterClassW(wndClassEx.lpszClassName, wndClassEx.hInstance);
		return ret;
	}

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		BaseWindow<DerivedWindow>* pThis = NULL;
		if (uMsg == WM_NCCREATE) {
			LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
			pThis = (DerivedWindow*)pCreateStruct->lpCreateParams;
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
		}
		else
			pThis = (DerivedWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (pThis)
			return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	virtual LRESULT CALLBACK HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};