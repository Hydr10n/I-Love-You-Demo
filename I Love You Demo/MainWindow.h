#pragma once

#pragma warning(disable:4996)

#include "pch.h"
#include "BaseWindow.h"
#include "ILoveYouDemo.h"
#include "MyAppSettingsData.h"
#include <memory>

#define Scale(iPixels, iDPI) MulDiv((int)(iPixels), (int)(iDPI), USER_DEFAULT_SCREEN_DPI)

class MainWindow final : public Hydr10n::Windows::BaseWindow {
public:
	MainWindow() noexcept(false) : BaseWindow(L"Direct2D") {
		using namespace Hydr10n::WindowHelpers;
		using Hydr10n::SystemErrorHelpers::ThrowIfFailed;
		ThrowIfFailed(Initialize(0, L"I Love You Demo", 0, 0, 0, 0, 0, NULL, NULL));
		Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED, (IUnknown**)&d2dFactory));
		Hydr10n::DisplayUtils::DisplayResolution displayResolution;
		if (MyAppSettingsData::Load(displayResolution)) {
			const auto& maxDisplayResolution = m_SystemDisplayResolutionSet.GetMaxDisplayResolution();
			if (displayResolution > maxDisplayResolution)
				displayResolution = maxDisplayResolution;
		}
		else {
			FLOAT dpiX, dpiY;
			d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
			displayResolution = { (DWORD)Scale(DefaultDeviceIndependentClientSize.cx, dpiX), (DWORD)Scale(DefaultDeviceIndependentClientSize.cy, dpiY) };
		}
		HWND hWnd = GetWindowHandle();
		m_ILoveYouDemo.reset(new Hydr10n::Demos::ILoveYouDemo(hWnd, displayResolution.PixelWidth, displayResolution.PixelHeight));
		bool showFramesPerSecond;
		if (MyAppSettingsData::Load(MyAppSettingsData::Key_bool::ShowFramesPerSecond, showFramesPerSecond))
			m_ILoveYouDemo->ShowFramesPerSecond(showFramesPerSecond);
		WindowMode windowMode;
		MyAppSettingsData::Load(windowMode);
		m_WindowModeHelper.reset(new WindowModeUtil(hWnd, windowMode, 0, DefaultWindowedModeStyle));
		m_WindowModeHelper->SetResolution(displayResolution, false);
		ThrowIfFailed(m_WindowModeHelper->SetMode(windowMode));
		m_HasTitleBar = windowMode == WindowMode::Windowed;
		bool showHelpAtStatup;
		if (!MyAppSettingsData::Load(MyAppSettingsData::Key_bool::ShowHelpAtStartup, showHelpAtStatup) || showHelpAtStatup) {
			MessageBoxW(hWnd, L"Window mode, resolution, FPS visibility and animations can be controlled in the context menu; glow & rotation of the heart image can be controlled with mouse. Pressing [Alt + Enter] toggles between windowed/borderless and full-screen mode.", L"Help", MB_OK);
			MyAppSettingsData::Save(MyAppSettingsData::Key_bool::ShowHelpAtStartup, false);
		}
	}

	DWORD Run() {
		MSG msg;
		do
			if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else
				m_ILoveYouDemo->Tick();
		while (msg.message != WM_QUIT);
		return (DWORD)msg.wParam;
	}

private:
	static const DWORD DefaultWindowedModeStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	static constexpr SIZE DefaultDeviceIndependentClientSize{ 650, 650 };
	static constexpr Hydr10n::DisplayUtils::DisplayResolution MinDisplayResolution{ 800, 600 };

	const Hydr10n::DisplayUtils::SystemDisplayResolutionSet m_SystemDisplayResolutionSet;

	bool m_HasTitleBar;
	int m_CursorCoordinateX{};
	RECT m_ClientRect{};
	std::unique_ptr<Hydr10n::WindowHelpers::WindowModeUtil> m_WindowModeHelper;
	std::unique_ptr<Hydr10n::Demos::ILoveYouDemo> m_ILoveYouDemo;

	LRESULT CALLBACK HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override {
		using Hydr10n::Demos::ILoveYouDemo;
		using Hydr10n::WindowHelpers::WindowMode;
		enum class MenuID {
			WindowModeWindowed, WindowModeBorderless, WindowModeFullScreen,
			ShowFramesPerSecond, HideFramesPerSecond,
			GlowPlayAnimation, GlowPauseAnimation, GlowReset,
			RotationPlayAnimation, RotationPauseAnimation, RotationReset, RotationClockwise, RotationCounterclockwise,
			ViewSourceCodeOnGitHub,
			Exit,
			FirstResolution
		};
		switch (uMsg) {
		case WM_SIZE: {
			m_ClientRect = { 0, 0, (int)LOWORD(lParam), (int)HIWORD(lParam) };
			switch (wParam) {
			case SIZE_MINIMIZED: m_ILoveYouDemo->Pause(); break;
			case SIZE_RESTORED: m_ILoveYouDemo->Resume(); break;
			}
		}	break;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			m_ILoveYouDemo->Tick(false);
			EndPaint(hWnd, &ps);
		}	break;
		case WM_SYSKEYDOWN: {
			if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000) {
				const auto mode = m_WindowModeHelper->GetMode() == WindowMode::FullScreen ? (m_HasTitleBar ? WindowMode::Windowed : WindowMode::Borderless) : WindowMode::FullScreen;
				if (m_WindowModeHelper->SetMode(mode))
					MyAppSettingsData::Save(mode);
			}
		}	break;
		case WM_MENUCHAR: return MAKELRESULT(0, MNC_CLOSE);
		case WM_LBUTTONDOWN: {
			m_CursorCoordinateX = GET_X_LPARAM(lParam);
			SetCapture(hWnd);
			RECT rc = m_ClientRect;
			MapWindowRect(hWnd, HWND_DESKTOP, &rc);
			ClipCursor(&rc);
		}	break;
		case WM_MOUSEMOVE: {
			if (wParam & MK_LBUTTON) {
				const int x = GET_X_LPARAM(lParam);
				m_ILoveYouDemo->SetForegroundRotationY(m_ILoveYouDemo->GetForegroundRotationY() + (x - m_CursorCoordinateX) * (0.4f * m_WindowModeHelper->GetResolution().PixelWidth / (m_ClientRect.right - m_ClientRect.left)));
				m_CursorCoordinateX = x;
			}
		}	break;
		case WM_LBUTTONUP: ReleaseCapture(); break;
		case WM_CAPTURECHANGED: ClipCursor(NULL); break;
		case WM_MOUSEWHEEL: m_ILoveYouDemo->SetForegroundGlowRadiusScale(m_ILoveYouDemo->GetForegroundGlowRadiusScale() + 0.1f * GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA); break;
		case WM_CONTEXTMENU: {
			const auto mode = m_WindowModeHelper->GetMode();
			const auto& resolution = m_WindowModeHelper->GetResolution();
			const auto& animationSet = m_ILoveYouDemo->GetAnimationSet();
			const bool isFramesPerSecondVisible = m_ILoveYouDemo->IsFramesPerSecondVisible(),
				isPlayingGlowAnimation = animationSet.Contains(ILoveYouDemo::AnimationSet::AnimationType::Glow),
				isPlayingRotationAnimation = animationSet.Contains(ILoveYouDemo::AnimationSet::AnimationType::Rotation),
				isRotationClockwise = m_ILoveYouDemo->IsRotationClockwise();
			const LPCWSTR lpcwPlayAnimation = L"Play Animation", lpcwReset = L"Reset";
			HMENU hMenu = CreatePopupMenu(), hMenuWindowMode = CreatePopupMenu(), hMenuResolution = CreatePopupMenu(), hMenuGlow = CreatePopupMenu(), hMenuRotation = CreatePopupMenu();
			AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuWindowMode, L"Window Mode");
			AppendMenuW(hMenuWindowMode, MF_STRING | (mode == WindowMode::Windowed ? MF_CHECKED : MF_UNCHECKED), (UINT_PTR)MenuID::WindowModeWindowed, L"Windowed");
			AppendMenuW(hMenuWindowMode, MF_STRING | (mode == WindowMode::Borderless ? MF_CHECKED : MF_UNCHECKED), (UINT_PTR)MenuID::WindowModeBorderless, L"Borderless");
			AppendMenuW(hMenuWindowMode, MF_STRING | (mode == WindowMode::FullScreen ? MF_CHECKED : MF_UNCHECKED), (UINT_PTR)MenuID::WindowModeFullScreen, L"Full-Screen");
			AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuResolution, L"Resolution");
			for (size_t i = 0; i != m_SystemDisplayResolutionSet.Count(); i++) {
				const auto& systemDisplayResolution = m_SystemDisplayResolutionSet[i];
				if (systemDisplayResolution >= MinDisplayResolution)
					AppendMenuW(hMenuResolution, MF_STRING | (resolution == systemDisplayResolution ? MF_CHECKED : MF_UNCHECKED), UINT_PTR((size_t)MenuID::FirstResolution + i), (std::to_wstring(systemDisplayResolution.PixelWidth) + L" × " + std::to_wstring(systemDisplayResolution.PixelHeight)).c_str());
			}
			AppendMenuW(hMenu, MF_STRING | (isFramesPerSecondVisible ? MF_CHECKED : MF_UNCHECKED), UINT_PTR(isFramesPerSecondVisible ? MenuID::HideFramesPerSecond : MenuID::ShowFramesPerSecond), L"Show FPS");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuGlow, L"Glow");
			AppendMenuW(hMenuGlow, MF_STRING | (isPlayingGlowAnimation ? MF_CHECKED : MF_UNCHECKED), UINT_PTR(isPlayingGlowAnimation ? MenuID::GlowPauseAnimation : MenuID::GlowPlayAnimation), lpcwPlayAnimation);
			AppendMenuW(hMenuGlow, MF_STRING, (UINT_PTR)MenuID::GlowReset, lpcwReset);
			AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuRotation, L"Rotation");
			AppendMenuW(hMenuRotation, MF_STRING | (isPlayingRotationAnimation ? MF_CHECKED : MF_UNCHECKED), UINT_PTR(isPlayingRotationAnimation ? MenuID::RotationPauseAnimation : MenuID::RotationPlayAnimation), lpcwPlayAnimation);
			AppendMenuW(hMenuRotation, MF_STRING, (UINT_PTR)MenuID::RotationReset, lpcwReset);
			AppendMenuW(hMenuRotation, MF_STRING | (isRotationClockwise ? MF_CHECKED : MF_UNCHECKED), UINT_PTR(isRotationClockwise ? MenuID::RotationCounterclockwise : MenuID::RotationClockwise), L"Clockwise");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuW(hMenu, MF_STRING, (UINT_PTR)MenuID::ViewSourceCodeOnGitHub, L"View Source Code on GitHub");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)MenuID::Exit, L"Exit");
			POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (pt.x == -1 && pt.y == -1)
				GetCursorPos(&pt);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
		}	break;
		case WM_COMMAND: {
			const MenuID menuId = (MenuID)wParam;
			switch (menuId) {
			case MenuID::WindowModeWindowed: {
				if (m_WindowModeHelper->GetMode() != WindowMode::Windowed && m_WindowModeHelper->SetMode(WindowMode::Windowed)) {
					m_HasTitleBar = true;
					MyAppSettingsData::Save(WindowMode::Windowed);
				}
			}	break;
			case MenuID::WindowModeBorderless: {
				if (m_WindowModeHelper->GetMode() != WindowMode::Borderless && m_WindowModeHelper->SetMode(WindowMode::Borderless)) {
					m_HasTitleBar = false;
					MyAppSettingsData::Save(WindowMode::Borderless);
				}
			}	break;
			case MenuID::WindowModeFullScreen: {
				if (m_WindowModeHelper->GetMode() != WindowMode::FullScreen && m_WindowModeHelper->SetMode(WindowMode::FullScreen))
					MyAppSettingsData::Save(WindowMode::FullScreen);
			}	break;
			case MenuID::ShowFramesPerSecond:
			case MenuID::HideFramesPerSecond: {
				const bool isFramesPerSecondVisible = !m_ILoveYouDemo->IsFramesPerSecondVisible();
				m_ILoveYouDemo->ShowFramesPerSecond(isFramesPerSecondVisible);
				MyAppSettingsData::Save(MyAppSettingsData::Key_bool::ShowFramesPerSecond, isFramesPerSecondVisible);
			}	break;
			case MenuID::GlowPlayAnimation:
			case MenuID::GlowPauseAnimation: m_ILoveYouDemo->ReverseAnimationState(ILoveYouDemo::AnimationSet::AnimationType::Glow); break;
			case MenuID::GlowReset: m_ILoveYouDemo->SetForegroundGlowRadiusScale(); break;
			case MenuID::RotationPlayAnimation:
			case MenuID::RotationPauseAnimation: m_ILoveYouDemo->ReverseAnimationState(ILoveYouDemo::AnimationSet::AnimationType::Rotation); break;
			case MenuID::RotationReset: m_ILoveYouDemo->SetForegroundRotationY(); break;
			case MenuID::RotationClockwise:
			case MenuID::RotationCounterclockwise: m_ILoveYouDemo->ReverseRotation(); break;
			case MenuID::ViewSourceCodeOnGitHub: ShellExecuteW(NULL, L"open", L"https://github.com/Hydr10n/I-Love-You-Demo", NULL, NULL, SW_SHOW); break;
			case MenuID::Exit: SendMessageW(hWnd, WM_CLOSE, 0, 0); break;
			default: {
				const auto& systemDisplayResolution = m_SystemDisplayResolutionSet[(size_t)menuId - (size_t)MenuID::FirstResolution];
				if (systemDisplayResolution != m_WindowModeHelper->GetResolution() && m_WindowModeHelper->SetResolution(systemDisplayResolution, m_WindowModeHelper->GetMode() != WindowMode::FullScreen)) {
					m_ILoveYouDemo->Resize((UINT32)systemDisplayResolution.PixelWidth, (UINT32)systemDisplayResolution.PixelHeight);
					MyAppSettingsData::Save(systemDisplayResolution);
				}
			}	break;
			}
		}	break;
		case WM_DESTROY: PostQuitMessage(0); break;
		default: return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
		return 0;
	}
};