/*
 * Header File: BaseWindow.h
 * Last Update: 2020/11/01
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <Windows.h>
#include <windowsx.h>

namespace Hydr10n {
	namespace Windows {
		class BaseWindow {
		public:
			BaseWindow(const BaseWindow&) = delete;
			BaseWindow& operator=(const BaseWindow&) = delete;

			virtual ~BaseWindow() {
				if (m_hWnd != NULL) {
					SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, 0);
					DestroyWindow(m_hWnd);
					UnregisterClassW(m_WndClassEx.lpszClassName, m_WndClassEx.hInstance);
				}
			}

			HWND GetWindowHandle() const { return m_hWnd; }

		private:
			HWND m_hWnd{};
			WNDCLASSEXW m_WndClassEx{ sizeof(m_WndClassEx) };

			static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
				BaseWindow* _this = (BaseWindow*)(uMsg == WM_NCCREATE ? ((LPCREATESTRUCT)lParam)->lpCreateParams : (LPVOID)GetWindowLongPtrW(hWnd, GWLP_USERDATA));
				if (_this)
					return _this->HandleMessage(hWnd, uMsg, wParam, lParam);
				return DefWindowProcW(hWnd, uMsg, wParam, lParam);
			}

		protected:
			BaseWindow(LPCWSTR lpszClassName = L"BaseWindow",
				UINT style = 0,
				LPCWSTR lpszMenuName = NULL,
				HBRUSH hbrBackground = GetStockBrush(WHITE_BRUSH),
				HICON hIcon = LoadIcon(NULL, IDI_APPLICATION),
				HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW)) {
				m_WndClassEx.style = style;
				m_WndClassEx.lpszMenuName = lpszMenuName;
				m_WndClassEx.hbrBackground = hbrBackground;
				m_WndClassEx.lpszClassName = lpszClassName;
				m_WndClassEx.hIcon = hIcon;
				m_WndClassEx.hCursor = hCursor;
				m_WndClassEx.hInstance = (HINSTANCE)GetModuleHandle(NULL);
				m_WndClassEx.lpfnWndProc = WindowProc;
			}

			BaseWindow(const WNDCLASSEXW& wndClassEx) { m_WndClassEx = wndClassEx; }

			const WNDCLASSEXW& GetWndClassEx() const { return m_WndClassEx; }

			BOOL Initialize(DWORD dwExStyle, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu) {
				if (m_hWnd != NULL) {
					SetLastError(ERROR_ALREADY_INITIALIZED);
					return FALSE;
				}
				RegisterClassExW(&m_WndClassEx);
				if ((m_hWnd = CreateWindowExW(dwExStyle, m_WndClassEx.lpszClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, m_WndClassEx.hInstance, this)) != NULL) {
					SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
					return TRUE;
				}
				const DWORD dwLastError = GetLastError();
				UnregisterClassW(m_WndClassEx.lpszClassName, m_WndClassEx.hInstance);
				SetLastError(dwLastError);
				return FALSE;
			}

			virtual LRESULT CALLBACK HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
		};
	};
}