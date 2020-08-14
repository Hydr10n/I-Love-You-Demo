#pragma once

#pragma warning(disable:4996)

#include "pch.h"
#include "BaseWindow.h"
#include "ILoveYouDemo.h"

#define Scale(iPixels, iDPI) MulDiv((int)(iPixels), (int)(iDPI), USER_DEFAULT_SCREEN_DPI)

using namespace Hydr10n::SystemErrorHelpers;
using Hydr10n::Demos::ILoveYouDemo;

class MainWindow final : public Hydr10n::Windows::BaseWindow {
public:
	MainWindow() : BaseWindow(L"Direct2D") {
		Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, (IUnknown**)&d2dFactory));
		FLOAT dpiX, dpiY;
		d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
		const SIZE size{ Scale(InitialWindowWidth, dpiX), Scale(InitialWindowHeight, dpiY) };
		if (!Initialize(0, L"I Love You Demo ( https://github.com/Hydr10n/I-Love-You-Demo )",
			WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			(GetSystemMetrics(SM_CXSCREEN) - size.cx) / 2, (GetSystemMetrics(SM_CYSCREEN) - size.cy) / 2, size.cx, size.cy,
			NULL, NULL))
			throw GetLastError();
		HWND hWnd = GetWindowHandle();
		ThrowIfFailed(m_ILoveYouDemo.Initialize(hWnd));
		ShowWindow(hWnd, SW_SHOWNORMAL);
		UpdateWindow(hWnd);
	}

	DWORD Run() {
		MSG msg;
		do
			if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else
				m_ILoveYouDemo.Tick();
		while (msg.message != WM_QUIT);
		return (DWORD)msg.wParam;
	}

private:
	static const int InitialWindowWidth = 800, InitialWindowHeight = InitialWindowWidth;

	int m_CursorCoordinateX = 0;
	RECT m_ClientRect{};
	ILoveYouDemo m_ILoveYouDemo;

	LRESULT CALLBACK HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override {
		enum class MenuID {
			GlowPlayAnimation, GlowPauseAnimation, GlowReset,
			RotationPlayAnimation, RotationPauseAnimation, RotationReset, RotationClockwise, RotationCounterclockwise
		};
		switch (uMsg) {
		case WM_SIZE: {
			SetRect(&m_ClientRect, 0, 0, LOWORD(lParam), HIWORD(lParam));
			switch (wParam) {
			case SIZE_MINIMIZED: m_ILoveYouDemo.Pause(); break;
			case SIZE_RESTORED: m_ILoveYouDemo.Resume(); break;
			}
		}	break;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			m_ILoveYouDemo.Tick(FALSE);
			EndPaint(hWnd, &ps);
		}	break;
		case WM_LBUTTONDOWN: {
			m_CursorCoordinateX = GET_X_LPARAM(lParam);
			SetCapture(hWnd);
			RECT rect = m_ClientRect;
			MapWindowRect(hWnd, HWND_DESKTOP, &rect);
			ClipCursor(&rect);
		}	break;
		case WM_MOUSEMOVE: {
			if (wParam & MK_LBUTTON) {
				const int x = GET_X_LPARAM(lParam);
				m_ILoveYouDemo.SetForegroundRotationY(true, m_ILoveYouDemo.GetForegroundRotationY() + (x - m_CursorCoordinateX) * (360.f / m_ClientRect.right));
				m_CursorCoordinateX = x;
			}
		}	break;
		case WM_LBUTTONUP: ReleaseCapture(); break;
		case WM_CAPTURECHANGED: ClipCursor(NULL); break;
		case WM_MOUSEWHEEL: m_ILoveYouDemo.SetForegroundGlowRadiusScale(true, m_ILoveYouDemo.GetForegroundGlowRadiusScale() + 0.1f * GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA); break;
		case WM_RBUTTONDOWN: {
			const auto& animationSet = m_ILoveYouDemo.GetAnimationSet();
			const BOOL bIsPlayingGlowAnimation = animationSet.Contains(ILoveYouDemo::AnimationSet::AnimationType::Glow),
				bIsPlayingRotationAnimation = animationSet.Contains(ILoveYouDemo::AnimationSet::AnimationType::Rotation),
				bIsRotationClockwise = m_ILoveYouDemo.IsRotationClockwise();
			LPCWSTR lpcwResumeAnimation = L"Resume Animation", lpcwPauseAnimation = L"Pause Animation", lpcwReset = L"Reset";
			HMENU hMenu = CreatePopupMenu(), hMenuGlow = CreatePopupMenu(), hMenuRotation = CreatePopupMenu();
			AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuGlow, L"Glow");
			AppendMenuW(hMenuGlow, MF_STRING, (UINT_PTR)(bIsPlayingGlowAnimation ? MenuID::GlowPauseAnimation : MenuID::GlowPlayAnimation), bIsPlayingGlowAnimation ? lpcwPauseAnimation : lpcwResumeAnimation);
			AppendMenuW(hMenuGlow, MF_STRING, (UINT_PTR)MenuID::GlowReset, lpcwReset);
			AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuRotation, L"Rotation");
			AppendMenuW(hMenuRotation, MF_STRING, (UINT_PTR)(bIsPlayingRotationAnimation ? MenuID::RotationPauseAnimation : MenuID::RotationPlayAnimation), bIsPlayingRotationAnimation ? lpcwPauseAnimation : lpcwResumeAnimation);
			AppendMenuW(hMenuRotation, MF_STRING, (UINT_PTR)MenuID::RotationReset, lpcwReset);
			AppendMenuW(hMenuRotation, MF_STRING, (UINT_PTR)(bIsRotationClockwise ? MenuID::RotationCounterclockwise : MenuID::RotationClockwise), bIsRotationClockwise ? L"Counterclockwise" : L"Clockwise");
			POINT point{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ClientToScreen(hWnd, &point);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN, point.x, point.y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
		}	break;
		case WM_COMMAND: {
			switch ((MenuID)LOWORD(wParam)) {
			case MenuID::GlowPlayAnimation:
			case MenuID::GlowPauseAnimation: m_ILoveYouDemo.ReverseAnimationState(ILoveYouDemo::AnimationSet::AnimationType::Glow); break;
			case MenuID::GlowReset: m_ILoveYouDemo.SetForegroundGlowRadiusScale(TRUE); break;
			case MenuID::RotationPlayAnimation:
			case MenuID::RotationPauseAnimation: m_ILoveYouDemo.ReverseAnimationState(ILoveYouDemo::AnimationSet::AnimationType::Rotation); break;
			case MenuID::RotationReset: m_ILoveYouDemo.SetForegroundRotationY(TRUE); break;
			case MenuID::RotationClockwise:
			case MenuID::RotationCounterclockwise: m_ILoveYouDemo.ReverseRotation(); break;
			}
		}	break;
		case WM_DESTROY: PostQuitMessage(0); break;
		default: return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
		return 0;
	}
};