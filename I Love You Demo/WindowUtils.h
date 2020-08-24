#pragma once

#include <Windows.h>
#include "DisplayUtils.h"

namespace Hydr10n {
	namespace WindowHelpers {
		inline BOOL WINAPI CenterMainWindow(RECT& rc) {
			const LONG cxScreen = GetSystemMetrics(SM_CXSCREEN), cyScreen = GetSystemMetrics(SM_CYSCREEN);
			const BOOL ret = cxScreen && cyScreen;
			if (ret) {
				const LONG width = rc.right - rc.left, height = rc.bottom - rc.top, x = (cxScreen - width) / 2, y = (cyScreen - height) / 2;
				rc = { x, y, x + width, y + height };
			}
			return ret;
		}

		enum class WindowMode { Windowed, Borderless, FullScreen };

		class WindowModeUtil final {
		public:
			WindowModeUtil(HWND hMainWindow, WindowMode initialMode, DWORD dwWindowedModeExStyle, DWORD dwWindowedModeStyle, bool hasMenu = false) : m_hWnd(hMainWindow), m_Mode(initialMode), m_WindowedModeExStyle(dwWindowedModeExStyle), m_WindowedModeStyle(dwWindowedModeStyle), m_HasMenu(hasMenu) {}

			bool SetResolution(const DisplayUtils::DisplayResolution& resolution, bool resize) {
				if (!resize) {
					m_Resolution = resolution;
					return true;
				}
				RECT rc = { 0, 0, (LONG)resolution.PixelWidth, (LONG)resolution.PixelHeight };
				if (!CenterMainWindow(rc) || (m_Mode == WindowMode::Windowed && !AdjustWindowRectEx(&rc, m_WindowedModeStyle, m_HasMenu, m_WindowedModeExStyle)))
					return false;
				const bool ret = SetWindowPos(m_hWnd, HWND_TOP, (int)rc.left, (int)rc.top, (int)(rc.right - rc.left), (int)(rc.bottom - rc.top), SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
				if (ret)
					m_Resolution = resolution;
				return ret;
			}

			const DisplayUtils::DisplayResolution& GetResolution() const { return m_Resolution; }

			bool SetMode(WindowMode mode) {
				BOOL ret;
				if (mode == WindowMode::FullScreen) {
					if (ret = (SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST) || GetLastError() == ERROR_SUCCESS) && (SetWindowLongPtrW(m_hWnd, GWL_STYLE, 0) || GetLastError() == ERROR_SUCCESS) && SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE)) {
						ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
						if (ret = GetLastError() == ERROR_SUCCESS)
							m_Mode = mode;
					}
				}
				else {
					const WindowMode modeCopy = m_Mode;
					m_Mode = mode;
					if (ret = (SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, (LONG_PTR)m_WindowedModeExStyle) || GetLastError() == ERROR_SUCCESS) && (SetWindowLongPtrW(m_hWnd, GWL_STYLE, mode == WindowMode::Windowed ? (LONG_PTR)m_WindowedModeStyle : WS_POPUP) || GetLastError() == ERROR_SUCCESS) && SetResolution(m_Resolution, true)) {
						ShowWindow(m_hWnd, SW_SHOWNORMAL);
						if (!(ret = GetLastError() == ERROR_SUCCESS))
							m_Mode = modeCopy;
					}
				}
				return ret;
			}

			WindowMode GetMode() const { return m_Mode; }

		private:
			BOOL m_HasMenu;
			DWORD m_WindowedModeExStyle, m_WindowedModeStyle;
			WindowMode m_Mode{};
			HWND m_hWnd;
			DisplayUtils::DisplayResolution m_Resolution;
		};
	}
}