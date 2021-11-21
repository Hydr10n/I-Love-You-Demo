#pragma once

#include "BaseWindow.h"

#include "ILoveYouDemo.h"

#include "DisplayHelpers.h"

#include "MyAppData.h"

#include <memory>

#define Scale(PixelCount, DPI) MulDiv(static_cast<int>(PixelCount), static_cast<int>(DPI), USER_DEFAULT_SCREEN_DPI)

class MainWindow : public Windows::BaseWindow {
public:
	MainWindow() noexcept(false) : BaseWindow(WNDCLASSEXW{ .hIcon = LoadIcon(nullptr, IDI_APPLICATION), .lpszClassName = L"Direct2D" }) {
		using ErrorHelpers::ThrowIfFailed;
		using SettingsData = MyAppData::Settings;

		ThrowIfFailed(Create(DefaultTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr));

		ThrowIfFailed(DisplayHelpers::GetDisplayResolutions(m_displayResolutions));

		SIZE outputSize;
		if (SettingsData::Load(outputSize)) {
			const auto& maxDisplayResolution = *max_element(m_displayResolutions.cbegin(), m_displayResolutions.cend());
			if (outputSize > maxDisplayResolution) outputSize = maxDisplayResolution;
		}
		else {
			Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
			ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.GetAddressOf()));

			FLOAT dpiX, dpiY;
#pragma warning(suppress: 4996)
			d2dFactory->GetDesktopDpi(&dpiX, &dpiY);

			outputSize = { Scale(650, dpiX), Scale(650, dpiY) };
		}
		m_iLoveYouDemo = std::make_unique<decltype(m_iLoveYouDemo)::element_type>(GetWindow(), outputSize);

		bool showFPS = true;
		SettingsData::Load(SettingsData::Key_bool::ShowFPS, showFPS);
		m_iLoveYouDemo->ShowFPS(showFPS);
	}

	WPARAM Run() {
		using SettingsData = MyAppData::Settings;

		const auto window = GetWindow();

		m_windowModeHelper = std::make_unique<decltype(m_windowModeHelper)::element_type>(window, m_iLoveYouDemo->GetOutputSize());
		WindowHelpers::WindowModeHelper::Mode windowMode;
		if (SettingsData::Load(windowMode)) m_windowModeHelper->SetMode(windowMode);

		UpdateWindow(window);

		m_iLoveYouDemo->Tick();

		bool showHelpAtStatup{};
		if (!SettingsData::Load(SettingsData::Key_bool::ShowHelpAtStartup, showHelpAtStatup) || showHelpAtStatup) {
			MessageBoxW(nullptr, L"Window mode, resolution, FPS visibility and animations can be controlled in the context menu; glow & rotation of the heart image can be controlled with mouse. Pressing [Alt + Enter] toggles between windowed/borderless and full-screen mode.", L"Help", MB_OK);
			if (!showHelpAtStatup) SettingsData::Save(SettingsData::Key_bool::ShowHelpAtStartup, false);
		}
		MSG msg;
		do
			if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else
				m_iLoveYouDemo->Tick();
		while (msg.message != WM_QUIT);

		return msg.wParam;
	}

private:
	static constexpr LPCWSTR DefaultTitle = L"I Love You";

	std::unique_ptr<Hydr10n::Demos::ILoveYou> m_iLoveYouDemo;

	std::unique_ptr<WindowHelpers::WindowModeHelper> m_windowModeHelper;

	std::vector<DisplayHelpers::Resolution> m_displayResolutions;

	LRESULT CALLBACK OnMessageReceived(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override {
		using Hydr10n::Demos::ILoveYou;
		using SettingsData = MyAppData::Settings;
		using WindowMode = WindowHelpers::WindowModeHelper::Mode;

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
			const auto menu = CreatePopupMenu(), hMenuWindowMode = CreatePopupMenu(), hMenuResolution = CreatePopupMenu(), hMenuGlow = CreatePopupMenu(), hMenuRotation = CreatePopupMenu();

			const auto mode = m_windowModeHelper->GetMode();
			AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuWindowMode), L"Window Mode");
			AppendMenuW(hMenuWindowMode, MF_STRING | (mode == WindowMode::Windowed ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(MenuID::WindowModeWindowed), L"Windowed");
			AppendMenuW(hMenuWindowMode, MF_STRING | (mode == WindowMode::Borderless ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(MenuID::WindowModeBorderless), L"Borderless");
			AppendMenuW(hMenuWindowMode, MF_STRING | (mode == WindowMode::Fullscreen ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(MenuID::WindowModeFullscreen), L"Fullscreen");
			AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuResolution), L"Resolution");
			int i = 0;

			const auto outputSize = m_windowModeHelper->GetOutputSize();
			for (const auto& resolution : m_displayResolutions) AppendMenuW(hMenuResolution, MF_STRING | (outputSize == resolution ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(static_cast<size_t>(MenuID::FirstResolution) + i++), (std::to_wstring(resolution.cx) + L" × " + std::to_wstring(resolution.cy)).c_str());

			const auto isFPSVisible = m_iLoveYouDemo->IsFPSVisible();
			AppendMenuW(menu, MF_STRING | (isFPSVisible ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(isFPSVisible ? MenuID::HideFPS : MenuID::ShowFPS), L"Show FPS");

			AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

			const auto& animations = m_iLoveYouDemo->GetAnimations();
			const auto playAnimation = L"Play Animation", reset = L"Reset";

			const auto isPlayingGlowAnimation = animations.contains(ILoveYou::Animation::Glow);
			AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuGlow), L"Glow");
			AppendMenuW(hMenuGlow, MF_STRING | (isPlayingGlowAnimation ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(isPlayingGlowAnimation ? MenuID::GlowPauseAnimation : MenuID::GlowPlayAnimation), playAnimation);
			AppendMenuW(hMenuGlow, MF_STRING, static_cast<UINT_PTR>(MenuID::GlowReset), reset);

			const auto isPlayingRotationAnimation = animations.contains(ILoveYou::Animation::Rotation), isRotationClockwise = m_iLoveYouDemo->IsRotationClockwise();
			AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuRotation), L"Rotation");
			AppendMenuW(hMenuRotation, MF_STRING | (isPlayingRotationAnimation ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(isPlayingRotationAnimation ? MenuID::RotationPauseAnimation : MenuID::RotationPlayAnimation), playAnimation);
			AppendMenuW(hMenuRotation, MF_STRING, static_cast<UINT_PTR>(MenuID::RotationReset), reset);
			AppendMenuW(hMenuRotation, MF_STRING | (isRotationClockwise ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(isRotationClockwise ? MenuID::RotationCounterclockwise : MenuID::RotationClockwise), L"Clockwise");

			AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

			AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(MenuID::ViewSourceCodeOnGitHub), L"View Source Code on GitHub");

			AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

			AppendMenuW(menu, MF_POPUP, static_cast<UINT_PTR>(MenuID::Exit), L"Exit");

			RECT rc{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (rc.left == -1 && rc.top == -1) {
				GetClientRect(hWnd, &rc);
				MapWindowRect(hWnd, HWND_DESKTOP, &rc);
			}
			TrackPopupMenu(menu, TPM_LEFTALIGN, static_cast<int>(rc.left), static_cast<int>(rc.top), 0, hWnd, nullptr);

			DestroyMenu(menu);
		}	break;

		case WM_COMMAND: {
			const auto menuID = static_cast<MenuID>(LOWORD(wParam));
			switch (menuID) {
			case MenuID::WindowModeWindowed: {
				constexpr WindowMode WindowMode = WindowMode::Windowed;
				if (m_windowModeHelper->GetMode() != WindowMode && m_windowModeHelper->SetMode(WindowMode)) SettingsData::Save(WindowMode);
			}	break;

			case MenuID::WindowModeBorderless: {
				constexpr WindowMode WindowMode = WindowMode::Borderless;
				if (m_windowModeHelper->GetMode() != WindowMode && m_windowModeHelper->SetMode(WindowMode)) SettingsData::Save(WindowMode);
			}	break;

			case MenuID::WindowModeFullscreen: {
				constexpr WindowMode WindowMode = WindowMode::Fullscreen;
				if (m_windowModeHelper->GetMode() != WindowMode && m_windowModeHelper->SetMode(WindowMode)) SettingsData::Save(WindowMode);
			}	break;

			case MenuID::ShowFPS:
			case MenuID::HideFPS: {
				const bool isFPSVisible = !m_iLoveYouDemo->IsFPSVisible();
				m_iLoveYouDemo->ShowFPS(isFPSVisible);
				SettingsData::Save(SettingsData::Key_bool::ShowFPS, isFPSVisible);
			}	break;

			case MenuID::GlowPlayAnimation:
			case MenuID::GlowPauseAnimation: m_iLoveYouDemo->ReverseAnimationState(ILoveYou::Animation::Glow); break;

			case MenuID::GlowReset: m_iLoveYouDemo->ResetForegroundGlowRadiusScale(); break;

			case MenuID::RotationPlayAnimation:
			case MenuID::RotationPauseAnimation: m_iLoveYouDemo->ReverseAnimationState(ILoveYou::Animation::Rotation); break;

			case MenuID::RotationReset: m_iLoveYouDemo->ResetForegroundRotation(); break;

			case MenuID::RotationClockwise:
			case MenuID::RotationCounterclockwise: m_iLoveYouDemo->ReverseRotation(); break;

			case MenuID::ViewSourceCodeOnGitHub: ShellExecuteW(nullptr, L"open", L"https://github.com/Hydr10n/I-Love-You-Demo", nullptr, nullptr, SW_SHOW); break;

			case MenuID::Exit: SendMessageW(hWnd, WM_CLOSE, 0, 0); break;

			default: {
				const auto& resolution = m_displayResolutions[static_cast<size_t>(menuID) - static_cast<size_t>(MenuID::FirstResolution)];
				const auto outputSize = m_windowModeHelper->GetOutputSize();
				if (resolution != outputSize && m_windowModeHelper->SetOutputSize(resolution)) {
					m_iLoveYouDemo->OnWindowSizeChanged(resolution);

					SettingsData::Save(resolution);
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

			m_iLoveYouDemo->OnMouseLeftButtonDown(wParam, lParam);
		}	break;

		case WM_LBUTTONUP: ReleaseCapture(); break;

		case WM_CAPTURECHANGED: ClipCursor(nullptr); break;

		case WM_MOUSEMOVE: m_iLoveYouDemo->OnMouseMove(wParam, lParam); break;

		case WM_MOUSEWHEEL: m_iLoveYouDemo->OnMouseWheel(wParam, lParam); break;

		case WM_MOVING:
		case WM_SIZING: m_iLoveYouDemo->Tick(); break;

		case WM_SIZE: {
			switch (wParam) {
			case SIZE_MINIMIZED: m_iLoveYouDemo->OnSuspending(); break;
			case SIZE_RESTORED: m_iLoveYouDemo->OnResuming(); break;
			}
		}	break;

		case WM_SYSKEYDOWN: {
			if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000 && m_windowModeHelper->ToggleMode()) SettingsData::Save(m_windowModeHelper->GetMode());
		}	break;

		case WM_MENUCHAR: return MAKELRESULT(0, MNC_CLOSE);

		case WM_DESTROY: PostQuitMessage(ERROR_SUCCESS); break;

		default: return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
		return 0;
	}
};
