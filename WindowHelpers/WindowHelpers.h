/*
 * Header File: WindowHelpers.h
 * Last Update: 2021/08/08
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <Windows.h>

namespace Hydr10n {
	namespace WindowHelpers {
		BOOL WINAPI CenterMainWindow(HWND hWnd, RECT& rect) {
			MONITORINFO monitorInfo;
			monitorInfo.cbSize = sizeof(monitorInfo);
			const BOOL ret = GetMonitorInfoW(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &monitorInfo);
			if (ret) {
				const auto& rc = monitorInfo.rcMonitor;
				const auto width = rect.right - rect.left, height = rect.bottom - rect.top, x = (rc.right - rc.left - width) / 2, y = (rc.bottom - rc.top - height) / 2;
				rect = { x, y, x + width, y + height };
			}
			return ret;
		}

		enum class WindowMode { Windowed, Borderless, Fullscreen };

		class WindowModeHelper {
		public:
			WindowModeHelper(HWND hMainWindow, const SIZE& clientSize, WindowMode mode, DWORD dwWindowedModeExStyle, DWORD dwWindowedModeStyle, BOOL bHasMenu) : m_hWnd(hMainWindow), m_clientSize(clientSize), m_currentMode(mode), m_lastMode(mode == WindowMode::Fullscreen ? WindowMode::Windowed : WindowMode::Fullscreen), m_WindowedModeExStyle(dwWindowedModeExStyle), m_WindowedModeStyle(dwWindowedModeStyle), m_HasMenu(bHasMenu) {}

			BOOL SetClientSize(const SIZE& size) {
				if (m_currentMode == WindowMode::Fullscreen) {
					m_clientSize = size;
					SetLastError(ERROR_SUCCESS);
					return TRUE;
				}
				RECT rc{ 0, 0, size.cx, size.cy };
				if (!CenterMainWindow(m_hWnd, rc) || (m_currentMode == WindowMode::Windowed && !AdjustWindowRectEx(&rc, m_WindowedModeStyle, m_HasMenu, m_WindowedModeExStyle)))
					return FALSE;
				const BOOL ret = SetWindowPos(m_hWnd, HWND_TOP, static_cast<int>(rc.left), static_cast<int>(rc.top), static_cast<int>(rc.right - rc.left), static_cast<int>(rc.bottom - rc.top), SWP_NOZORDER | SWP_FRAMECHANGED);
				if (ret)
					m_clientSize = size;
				return ret;
			}

			SIZE GetClientSize() const { return m_clientSize; }

			BOOL SetMode(WindowMode mode) {
				BOOL ret;
				if (mode == WindowMode::Fullscreen) {
					if (ret = (SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST) || GetLastError() == ERROR_SUCCESS) && (SetWindowLongPtrW(m_hWnd, GWL_STYLE, 0) || GetLastError() == ERROR_SUCCESS) && SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE)) {
						ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
						if (mode != m_currentMode) {
							m_lastMode = m_currentMode;
							m_currentMode = mode;
						}
						SetLastError(ERROR_SUCCESS);
					}
				}
				else {
					const WindowMode currentMode = m_currentMode;
					m_currentMode = mode;
					if (ret = (SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, static_cast<LONG_PTR>(m_WindowedModeExStyle)) || GetLastError() == ERROR_SUCCESS) && (SetWindowLongPtrW(m_hWnd, GWL_STYLE, static_cast<LONG_PTR>(mode == WindowMode::Windowed ? m_WindowedModeStyle : m_WindowedModeStyle & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX))) || GetLastError() == ERROR_SUCCESS) && SetClientSize(m_clientSize)) {
						ShowWindow(m_hWnd, SW_SHOWNORMAL);
						if (mode != currentMode)
							m_lastMode = m_currentMode == WindowMode::Fullscreen ? WindowMode::Windowed : WindowMode::Fullscreen;
						SetLastError(ERROR_SUCCESS);
					}
				}
				return ret;
			}

			BOOL ToggleMode() { return SetMode(m_lastMode); }

			WindowMode GetMode() const { return m_currentMode; }

		private:
			const BOOL m_HasMenu;
			const DWORD m_WindowedModeExStyle, m_WindowedModeStyle;
			const HWND m_hWnd;

			WindowMode m_lastMode, m_currentMode;
			SIZE m_clientSize;
		};
	}
}