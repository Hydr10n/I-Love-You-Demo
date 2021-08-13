#pragma once

#include "BaseWindow.h"
#include "ILoveYouDemo.h"
#include "DisplayHelpers.h"
#include "MyAppSettingsData.h"
#include <memory>

#define Scale(PixelCount, DPI) MulDiv(static_cast<int>(PixelCount), static_cast<int>(DPI), USER_DEFAULT_SCREEN_DPI)

class MainWindow : public Hydr10n::Windows::BaseWindow {
public:
	MainWindow() noexcept(false) : BaseWindow(L"Direct2D") {
		using ErrorHelpers::ThrowIfFailed;
		ThrowIfFailed(Create(DefaultTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr));
		SIZE outputSize;
		if (MyAppSettingsData::Load(outputSize)) {
			const auto& maxDisplayResolution = *max_element(m_DisplayResolutions.cbegin(), m_DisplayResolutions.cend());
			if (outputSize > maxDisplayResolution)
				outputSize = maxDisplayResolution;
		}
		else {
			Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
			ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.GetAddressOf()));
			FLOAT dpiX, dpiY;
#pragma warning(suppress: 4996)
			d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
			outputSize = { Scale(650, dpiX), Scale(650, dpiY) };
		}
		const HWND hWnd = GetWindow();
		Hydr10n::WindowHelpers::WindowMode windowMode;
		MyAppSettingsData::Load(windowMode);
		m_WindowModeHelper = std::make_unique<decltype(m_WindowModeHelper)::element_type>(hWnd, outputSize, windowMode, 0, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
		m_ILoveYouDemo = std::make_unique<decltype(m_ILoveYouDemo)::element_type>(hWnd, outputSize);
		bool showFPS;
		MyAppSettingsData::Load(MyAppSettingsData::Key_bool::ShowFPS, showFPS);
		m_ILoveYouDemo->ShowFPS(showFPS);
	}

	WPARAM Run() {
		ShowWindow(GetWindow(), SW_SHOW);
		m_WindowModeHelper->SetMode(m_WindowModeHelper->GetMode());
		m_ILoveYouDemo->Tick();
		bool showHelpAtStatup;
		if (!MyAppSettingsData::Load(MyAppSettingsData::Key_bool::ShowHelpAtStartup, showHelpAtStatup) || showHelpAtStatup) {
			MessageBoxW(nullptr, L"Window mode, resolution, FPS visibility and animations can be controlled in the context menu; glow & rotation of the heart image can be controlled with mouse. Pressing [Alt + Enter] toggles between windowed/borderless and full-screen mode.", L"Help", MB_OK);
			if (!showHelpAtStatup)
				MyAppSettingsData::Save(MyAppSettingsData::Key_bool::ShowHelpAtStartup, false);
		}
		MSG msg;
		do
			if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else
				m_ILoveYouDemo->Tick();
		while (msg.message != WM_QUIT);
		return msg.wParam;
	}

private:
	static constexpr LPCWSTR DefaultTitle = L"I Love You";

	const std::vector<Hydr10n::DisplayHelpers::Resolution> m_DisplayResolutions = Hydr10n::DisplayHelpers::GetDisplayResolutions();

	std::unique_ptr<Hydr10n::WindowHelpers::WindowModeHelper> m_WindowModeHelper;
	std::unique_ptr<Hydr10n::Demos::ILoveYou> m_ILoveYouDemo;

	LRESULT CALLBACK OnMessageReceived(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override {
		using Hydr10n::Demos::ILoveYou;
		using Hydr10n::WindowHelpers::WindowMode;
		enum class MenuID {
			WindowModeWindowed, WindowModeBorderless, WindowModeFullscreen,
			ShowFPS, HideFPS,
			GlowPlayAnimation, GlowPauseAnimation, GlowReset,
			RotationPlayAnimation, RotationPauseAnimation, RotationReset, RotationClockwise, RotationCounterclockwise,
			ViewSourceCodeOnGitHub,
			Exit,
			FirstResolution
		};
		switch (uMsg) {
		case WM_CONTEXTMENU: {
			const auto mode = m_WindowModeHelper->GetMode();
			const auto outputSize = m_WindowModeHelper->GetOutputSize();
			const auto& animations = m_ILoveYouDemo->GetAnimations();
			const bool isFPSVisible = m_ILoveYouDemo->IsFPSVisible(),
				isPlayingGlowAnimation = animations.contains(ILoveYou::Animation::Glow),
				isPlayingRotationAnimation = animations.contains(ILoveYou::Animation::Rotation),
				isRotationClockwise = m_ILoveYouDemo->IsRotationClockwise();
			const LPCWSTR lpcPlayAnimation = L"Play Animation", lpcReset = L"Reset";
			const HMENU hMenu = CreatePopupMenu(), hMenuWindowMode = CreatePopupMenu(), hMenuResolution = CreatePopupMenu(), hMenuGlow = CreatePopupMenu(), hMenuRotation = CreatePopupMenu();
			AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuWindowMode), L"Window Mode");
			AppendMenuW(hMenuWindowMode, MF_STRING | (mode == WindowMode::Windowed ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(MenuID::WindowModeWindowed), L"Windowed");
			AppendMenuW(hMenuWindowMode, MF_STRING | (mode == WindowMode::Borderless ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(MenuID::WindowModeBorderless), L"Borderless");
			AppendMenuW(hMenuWindowMode, MF_STRING | (mode == WindowMode::Fullscreen ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(MenuID::WindowModeFullscreen), L"Fullscreen");
			AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuResolution), L"Resolution");
			int i = 0;
			for (const auto& resolution : m_DisplayResolutions)
				AppendMenuW(hMenuResolution, MF_STRING | (outputSize == resolution ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(static_cast<size_t>(MenuID::FirstResolution) + i++), (std::to_wstring(resolution.cx) + L" × " + std::to_wstring(resolution.cy)).c_str());
			AppendMenuW(hMenu, MF_STRING | (isFPSVisible ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(isFPSVisible ? MenuID::HideFPS : MenuID::ShowFPS), L"Show FPS");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
			AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuGlow), L"Glow");
			AppendMenuW(hMenuGlow, MF_STRING | (isPlayingGlowAnimation ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(isPlayingGlowAnimation ? MenuID::GlowPauseAnimation : MenuID::GlowPlayAnimation), lpcPlayAnimation);
			AppendMenuW(hMenuGlow, MF_STRING, static_cast<UINT_PTR>(MenuID::GlowReset), lpcReset);
			AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuRotation), L"Rotation");
			AppendMenuW(hMenuRotation, MF_STRING | (isPlayingRotationAnimation ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(isPlayingRotationAnimation ? MenuID::RotationPauseAnimation : MenuID::RotationPlayAnimation), lpcPlayAnimation);
			AppendMenuW(hMenuRotation, MF_STRING, static_cast<UINT_PTR>(MenuID::RotationReset), lpcReset);
			AppendMenuW(hMenuRotation, MF_STRING | (isRotationClockwise ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(isRotationClockwise ? MenuID::RotationCounterclockwise : MenuID::RotationClockwise), L"Clockwise");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
			AppendMenuW(hMenu, MF_STRING, static_cast<UINT_PTR>(MenuID::ViewSourceCodeOnGitHub), L"View Source Code on GitHub");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
			AppendMenuW(hMenu, MF_POPUP, static_cast<UINT_PTR>(MenuID::Exit), L"Exit");
			POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (pt.x == -1 && pt.y == -1)
				GetCursorPos(&pt);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, nullptr);
			DestroyMenu(hMenu);
		}	break;
		case WM_COMMAND: {
			const auto menuId = static_cast<MenuID>(wParam);
			switch (menuId) {
			case MenuID::WindowModeWindowed: {
				constexpr WindowMode WindowMode = WindowMode::Windowed;
				if (m_WindowModeHelper->GetMode() != WindowMode && m_WindowModeHelper->SetMode(WindowMode))
					MyAppSettingsData::Save(WindowMode);
			}	break;
			case MenuID::WindowModeBorderless: {
				constexpr WindowMode WindowMode = WindowMode::Borderless;
				if (m_WindowModeHelper->GetMode() != WindowMode && m_WindowModeHelper->SetMode(WindowMode))
					MyAppSettingsData::Save(WindowMode);
			}	break;
			case MenuID::WindowModeFullscreen: {
				constexpr WindowMode WindowMode = WindowMode::Fullscreen;
				if (m_WindowModeHelper->GetMode() != WindowMode && m_WindowModeHelper->SetMode(WindowMode))
					MyAppSettingsData::Save(WindowMode);
			}	break;
			case MenuID::ShowFPS:
			case MenuID::HideFPS: {
				const bool isFPSVisible = !m_ILoveYouDemo->IsFPSVisible();
				m_ILoveYouDemo->ShowFPS(isFPSVisible);
				MyAppSettingsData::Save(MyAppSettingsData::Key_bool::ShowFPS, isFPSVisible);
			}	break;
			case MenuID::GlowPlayAnimation:
			case MenuID::GlowPauseAnimation: m_ILoveYouDemo->ReverseAnimationState(ILoveYou::Animation::Glow); break;
			case MenuID::GlowReset: m_ILoveYouDemo->ResetForegroundGlowRadiusScale(); break;
			case MenuID::RotationPlayAnimation:
			case MenuID::RotationPauseAnimation: m_ILoveYouDemo->ReverseAnimationState(ILoveYou::Animation::Rotation); break;
			case MenuID::RotationReset: m_ILoveYouDemo->ResetForegroundRotation(); break;
			case MenuID::RotationClockwise:
			case MenuID::RotationCounterclockwise: m_ILoveYouDemo->ReverseRotation(); break;
			case MenuID::ViewSourceCodeOnGitHub: ShellExecuteW(nullptr, L"open", L"https://github.com/Hydr10n/I-Love-You-Demo", nullptr, nullptr, SW_SHOW); break;
			case MenuID::Exit: SendMessageW(hWnd, WM_CLOSE, 0, 0); break;
			default: {
				const auto& resolution = m_DisplayResolutions[static_cast<size_t>(menuId) - static_cast<size_t>(MenuID::FirstResolution)];
				const auto outputSize = m_WindowModeHelper->GetOutputSize();
				if (resolution != outputSize && m_WindowModeHelper->SetOutputSize(resolution)) {
					m_ILoveYouDemo->OnWindowSizeChanged(resolution);
					MyAppSettingsData::Save(resolution);
				}
			}	break;
			}
		}	break;
		case WM_LBUTTONDOWN: {
			SetCapture(hWnd);
			RECT rc;
			GetClientRect(hWnd, &rc);
			MapWindowRect(hWnd, HWND_DESKTOP, &rc);
			ClipCursor(&rc);
			m_ILoveYouDemo->OnMouseLeftButtonDown(wParam, lParam);
		}	break;
		case WM_LBUTTONUP: ReleaseCapture(); break;
		case WM_CAPTURECHANGED: ClipCursor(nullptr); break;
		case WM_MOUSEMOVE: m_ILoveYouDemo->OnMouseMove(wParam, lParam); break;
		case WM_MOUSEWHEEL: m_ILoveYouDemo->OnMouseWheel(wParam, lParam); break;
		case WM_MOVING: case WM_SIZING: m_ILoveYouDemo->Tick(); break;
		case WM_SIZE: {
			switch (wParam) {
			case SIZE_MINIMIZED: m_ILoveYouDemo->OnSuspending(); break;
			case SIZE_RESTORED: m_ILoveYouDemo->OnResuming(); break;
			}
		}	break;
		case WM_SYSKEYDOWN: {
			if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000 && m_WindowModeHelper->ToggleMode())
				MyAppSettingsData::Save(m_WindowModeHelper->GetMode());
		}	break;
		case WM_MENUCHAR: return MAKELRESULT(0, MNC_CLOSE);
		case WM_DESTROY: PostQuitMessage(0); break;
		default: return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
		return 0;
	}
};