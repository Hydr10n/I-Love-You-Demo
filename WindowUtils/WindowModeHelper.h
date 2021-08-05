/*
 * Header File: WindowModeHelper.h
 * Last Update: 2021/08/05
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <Windows.h>
#include "WindowUtils.h"
#include "../DisplayUtils/Resolution.h"

namespace Hydr10n {
	namespace WindowUtils {
		enum class WindowMode { Windowed, Borderless, FullScreen };

		class WindowModeHelper {
		public:
			WindowModeHelper(HWND hMainWindow, const DisplayUtils::Resolution& resolution, WindowMode mode, DWORD dwWindowedModeExStyle, DWORD dwWindowedModeStyle, BOOL bHasMenu) : m_hWnd(hMainWindow), m_Resolution(resolution), m_CurrentMode(mode), m_PreviousMode(mode == WindowMode::FullScreen ? WindowMode::Windowed : WindowMode::FullScreen), m_WindowedModeExStyle(dwWindowedModeExStyle), m_WindowedModeStyle(dwWindowedModeStyle), m_HasMenu(bHasMenu) {}

			BOOL SetResolution(const DisplayUtils::Resolution& resolution) {
				if (m_CurrentMode == WindowMode::FullScreen) {
					m_Resolution = resolution;
					SetLastError(ERROR_SUCCESS);
					return TRUE;
				}
				RECT rc{ 0, 0, static_cast<LONG>(resolution.Width), static_cast<LONG>(resolution.Height) };
				if (!CenterMainWindow(rc) || (m_CurrentMode == WindowMode::Windowed && !AdjustWindowRectEx(&rc, m_WindowedModeStyle, m_HasMenu, m_WindowedModeExStyle)))
					return FALSE;
				const BOOL ret = SetWindowPos(m_hWnd, HWND_TOP, static_cast<int>(rc.left), static_cast<int>(rc.top), static_cast<int>(rc.right - rc.left), static_cast<int>(rc.bottom - rc.top), SWP_NOZORDER | SWP_FRAMECHANGED);
				if (ret)
					m_Resolution = resolution;
				return ret;
			}

			const DisplayUtils::Resolution GetResolution() const { return m_Resolution; }

			BOOL SetMode(WindowMode mode) {
				BOOL ret;
				if (mode == WindowMode::FullScreen) {
					if (ret = (SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST) || GetLastError() == ERROR_SUCCESS) && (SetWindowLongPtrW(m_hWnd, GWL_STYLE, 0) || GetLastError() == ERROR_SUCCESS) && SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE)) {
						ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
						if (mode != m_CurrentMode) {
							m_PreviousMode = m_CurrentMode;
							m_CurrentMode = mode;
						}
						SetLastError(ERROR_SUCCESS);
					}
				}
				else {
					const WindowMode currentMode = m_CurrentMode;
					m_CurrentMode = mode;
					if (ret = (SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, static_cast<LONG_PTR>(m_WindowedModeExStyle)) || GetLastError() == ERROR_SUCCESS) && (SetWindowLongPtrW(m_hWnd, GWL_STYLE, static_cast<LONG_PTR>(mode == WindowMode::Windowed ? m_WindowedModeStyle : m_WindowedModeStyle & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX))) || GetLastError() == ERROR_SUCCESS) && SetResolution(m_Resolution)) {
						ShowWindow(m_hWnd, SW_SHOWNORMAL);
						if (mode != currentMode)
							m_PreviousMode = m_CurrentMode == WindowMode::FullScreen ? WindowMode::Windowed : WindowMode::FullScreen;
						SetLastError(ERROR_SUCCESS);
					}
				}
				return ret;
			}

			BOOL ToggleMode() { return SetMode(m_PreviousMode); }

			WindowMode GetMode() const { return m_CurrentMode; }

		private:
			const BOOL m_HasMenu;
			const DWORD m_WindowedModeExStyle, m_WindowedModeStyle;
			const HWND m_hWnd;

			WindowMode m_PreviousMode, m_CurrentMode;
			DisplayUtils::Resolution m_Resolution;
		};
	}
}