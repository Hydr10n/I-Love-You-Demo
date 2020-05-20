#pragma once

#include "pch.h"
#include "BaseWindow.h"

#pragma warning(disable:4996)
#pragma warning(disable:26812)

#define Scale(iPixels, iDPI) MulDiv((INT)iPixels, (INT)iDPI, USER_DEFAULT_SCREEN_DPI)
#define CalcFrameDuration(iFPS) (1000 / iFPS)
#define CalcTransitionVelocity(iSize, iMilliseconds, iFPS) (iSize / (iMilliseconds / 1000.0f * iFPS))

using Microsoft::WRL::ComPtr;

class MainWindow : public BaseWindow<MainWindow> {
public:
	MainWindow() {
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, (IUnknown**)&m_D2dFactory)))
			throw;
		SetWndClassEx(L"Direct2D", LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW), GetStockBrush(WHITE_BRUSH));
		FLOAT m_DpiX = USER_DEFAULT_SCREEN_DPI, m_DpiY = USER_DEFAULT_SCREEN_DPI;
		m_D2dFactory->GetDesktopDpi(&m_DpiX, &m_DpiY);
		const SIZE size = { Scale(MainWindowWidth, m_DpiX), Scale(MainWindowHeight, m_DpiY) };
		if (!Initialize(0, L"I Love You Demo ( https://github.com/Hydr10n/I-Love-You-Demo )",
			WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			(GetSystemMetrics(SM_CXSCREEN) - size.cx) / 2, (GetSystemMetrics(SM_CYSCREEN) - size.cy) / 2, size.cx, size.cy,
			NULL, NULL)) {
			throw;
		}
		ShowWindow(m_hWnd, SW_SHOWNORMAL);
		UpdateWindow(m_hWnd);
	}

	DWORD Run() {
		MSG msg = {};
		while (msg.message != WM_QUIT) {
			if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else
				WaitTimer();
		}
		return (DWORD)msg.wParam;
	}

protected:
	enum class Menu_Identifiers {
		Play, Pause,
		Clockwise, Counterclockwise
	};

	static const int MainWindowWidth = 800, MainWindowHeight = MainWindowWidth,
		HeartWindowWidth = MainWindowWidth, HeartWindowHeight = MainWindowWidth,
		FPS = 60, TotalAnimationDuration = 2500, FrameDuration = CalcFrameDuration(FPS);

	BOOL m_bPlay = TRUE, m_bClockwise = TRUE;
	FLOAT m_angleYDelta = CalcTransitionVelocity(360, TotalAnimationDuration, FPS), m_AngleY = 0, m_FrameDurationCount = 0;
	HANDLE m_hTimer = NULL;
	ComPtr<ID2D1Factory> m_D2dFactory;
	ComPtr<ID2D1HwndRenderTarget> m_D2dHwndRenderTarget;
	ComPtr<ID2D1BitmapRenderTarget> m_D2dBitmapRenderTargetBackground, m_D2dBitmapRenderTargetForeground;
	ComPtr<ID2D1Bitmap> m_D2dBitmapBackground, m_D2dBitmapForeground;
	ComPtr<ID2D1DeviceContext> m_D2dDeviceContext;
	ComPtr<ID2D1Effect> m_D2d3DPerspectiveTransformEffect;

	LRESULT CALLBACK HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override {
		switch (uMsg) {
		case WM_CREATE: {
			try {
				ThrowIfFailed(CreateHwndRenderTarget(m_D2dFactory.Get(), hWnd, &m_D2dHwndRenderTarget));
				ThrowIfFailed(GetDeviceContext(m_D2dHwndRenderTarget.Get(), &m_D2dDeviceContext));
				ThrowIfFailed(m_D2dHwndRenderTarget->CreateCompatibleRenderTarget(&m_D2dBitmapRenderTargetBackground));
				ThrowIfFailed(m_D2dBitmapRenderTargetBackground->GetBitmap(&m_D2dBitmapBackground));
				ThrowIfFailed(GradientFillBackground(m_D2dBitmapRenderTargetBackground.Get(), D2D1::ColorF(0x157050), D2D1::ColorF(0x155799)));
				ThrowIfFailed(m_D2dHwndRenderTarget->CreateCompatibleRenderTarget(&m_D2dBitmapRenderTargetForeground));
				ThrowIfFailed(m_D2dBitmapRenderTargetForeground->GetBitmap(&m_D2dBitmapForeground));
				ThrowIfFailed(Create3DPerspectiveTransformEffect(m_D2dDeviceContext.Get(), &m_D2d3DPerspectiveTransformEffect));
				const D2D1_SIZE_F d2dSize = m_D2dHwndRenderTarget->GetSize();
				m_D2d3DPerspectiveTransformEffect->SetInput(0, m_D2dBitmapForeground.Get());
				m_D2d3DPerspectiveTransformEffect->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_PERSPECTIVE_ORIGIN, D2D1::Vector2F(d2dSize.width / 2, d2dSize.height / 2));
				ThrowIfFailed(DrawHeart(m_D2dBitmapRenderTargetForeground.Get()));
				ThrowIfFailed(DrawText(m_D2dBitmapRenderTargetForeground.Get(), L"Comic Sans MS", 30, DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"I LOVE YOU\nFOREVER", D2D1::ColorF(0xf0f0f0), NULL));
				ThrowIfFailed(StartTimer());
			}
			catch (HRESULT) {
				PostQuitMessage(1);
				break;
			}
		}	break;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			if (m_bPlay) {
				m_D2dDeviceContext->BeginDraw();
				m_D2dDeviceContext->DrawImage(m_D2dBitmapBackground.Get());
				m_D2dDeviceContext->DrawImage(m_D2d3DPerspectiveTransformEffect.Get());
				const D2D1_SIZE_F d2dSize = m_D2dHwndRenderTarget->GetSize();
				const D2D1_RECT_F d2dRect = D2D1::RectF(0, d2dSize.height * 0.9f, d2dSize.width, d2dSize.height);
				DrawText(m_D2dDeviceContext.Get(), L"Segoe UI", 25, DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"Copyright \xa9 Programmer-Yang_Xun@outlook.com", D2D1::ColorF(0xf0f0f0), &d2dRect, TRUE);
				m_D2dDeviceContext->EndDraw();
				if ((m_FrameDurationCount += FrameDuration) > TotalAnimationDuration)
					m_FrameDurationCount = 0;
				m_AngleY += (m_bClockwise ? -1 : 1) * m_angleYDelta;
				m_D2d3DPerspectiveTransformEffect->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION_ORIGIN, D2D1::Vector3F(m_D2dHwndRenderTarget->GetSize().width / 2, 0, 0));
				m_D2d3DPerspectiveTransformEffect->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION, D2D1::Vector3F(0, m_AngleY, 0));
			}
			EndPaint(hWnd, &ps);
		}	break;
		case WM_RBUTTONDOWN: {
			HMENU hMenu = CreatePopupMenu();
			AppendMenuW(hMenu, MF_STRING, (UINT_PTR)(m_bPlay ? Menu_Identifiers::Pause : Menu_Identifiers::Play), m_bPlay ? L"Pause" : L"Play");
			AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenuW(hMenu, MF_STRING, (UINT_PTR)(m_bClockwise ? Menu_Identifiers::Counterclockwise : Menu_Identifiers::Clockwise), m_bClockwise ? L"Counterclockwise" : L"Clockwise");
			SetForegroundWindow(hWnd);
			POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ClientToScreen(hWnd, &point);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN, point.x, point.y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
		}	break;
		case WM_COMMAND: {
			switch ((LOWORD(wParam))) {
			case (WORD)Menu_Identifiers::Play:
			case (WORD)Menu_Identifiers::Pause: {
				m_bPlay = !m_bPlay;
				if (m_bPlay)
					SetTimer(FrameDuration, TRUE);
				else
					CancelWaitableTimer(m_hTimer);
			}	break;
			case (WORD)Menu_Identifiers::Clockwise:
			case (WORD)Menu_Identifiers::Counterclockwise: m_bClockwise = !m_bClockwise; break;
			}
		}	break;
		case WM_DESTROY: {
			CloseHandle(m_hTimer);
			PostQuitMessage(0);
		}	break;
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	HRESULT StartTimer() {
		m_hTimer = CreateWaitableTimerW(NULL, FALSE, NULL);
		if (m_hTimer == NULL)
			return E_FAIL;
		HRESULT hr = SetTimer(FrameDuration, FALSE);
		if (FAILED(hr)) {
			CloseHandle(m_hTimer);
			m_hTimer = NULL;
		}
		return hr;
	}

	HRESULT SetTimer(LONG lPeriod, BOOL bResume) {
		LARGE_INTEGER li = {};
		return SetWaitableTimer(m_hTimer, &li, lPeriod, NULL, NULL, bResume) ? S_OK : E_FAIL;
	}

	void WaitTimer() {
		if (MsgWaitForMultipleObjects(1, &m_hTimer, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0)
			InvalidateRect(m_hWnd, NULL, FALSE);
	}

	static HRESULT CreateHwndRenderTarget(ID2D1Factory* pD2dFactory, HWND hWnd, ID2D1HwndRenderTarget** ppD2dHwndRenderTarget) {
		RECT rc;
		GetClientRect(hWnd, &rc);
		return pD2dFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(rc.right, rc.bottom)), ppD2dHwndRenderTarget);
	}

	static HRESULT GetDeviceContext(ID2D1RenderTarget* pD2dRenderTarget, ID2D1DeviceContext** ppD2dDeviceContext) { return pD2dRenderTarget->QueryInterface(ppD2dDeviceContext); }

	static HRESULT Create3DPerspectiveTransformEffect(ID2D1DeviceContext* pD2dDeviceContext, ID2D1Effect** ppD2dEffect) { return pD2dDeviceContext->CreateEffect(CLSID_D2D13DPerspectiveTransform, ppD2dEffect); }

	static HRESULT GradientFillBackground(ID2D1RenderTarget* pD2dRenderTarget, const D2D1_COLOR_F& d2d1ColorStart, const D2D1_COLOR_F& d2d1ColorEnd) {
		try {
			const D2D1_GRADIENT_STOP d2dGradientStops[] = {
				D2D1::GradientStop(0, d2d1ColorStart),
				D2D1::GradientStop(1, d2d1ColorEnd)
			};
			const D2D1_SIZE_F d2dSize = pD2dRenderTarget->GetSize();
			ComPtr<ID2D1GradientStopCollection> d2dGradientStopCollection;
			ComPtr<ID2D1LinearGradientBrush> d2dLinearGradientBrush;
			ThrowIfFailed(pD2dRenderTarget->CreateGradientStopCollection(d2dGradientStops, ARRAYSIZE(d2dGradientStops), &d2dGradientStopCollection));
			ThrowIfFailed(pD2dRenderTarget->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(0, 0), D2D1::Point2F(d2dSize.width, d2dSize.height)), d2dGradientStopCollection.Get(), &d2dLinearGradientBrush));
			pD2dRenderTarget->BeginDraw();
			pD2dRenderTarget->FillRectangle(D2D1::RectF(0, 0, d2dSize.width, d2dSize.height), d2dLinearGradientBrush.Get());
			ThrowIfFailed(pD2dRenderTarget->EndDraw());
		}
		catch (HRESULT hr) {
			return hr;
		}
		return S_OK;
	}

	static HRESULT DrawText(ID2D1RenderTarget* pD2dRenderTarget, PCWSTR pFontName, FLOAT fontSize, DWRITE_TEXT_ALIGNMENT textAlignment, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment, PCWSTR pText, const D2D1_COLOR_F& d2dColor, const D2D1_RECT_F* pD2dRect, BOOL bDrawBegun = FALSE) {
		try {
			ID2D1Factory* pD2dFactory;
			pD2dRenderTarget->GetFactory(&pD2dFactory);
			ComPtr<IDWriteFactory> dwriteFactory;
			ComPtr<IDWriteTextFormat> dwriteTextFormat;
			ComPtr<ID2D1SolidColorBrush> d2dSolidBrush;
			ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwriteFactory.Get()), (IUnknown**)(&dwriteFactory)));
			ThrowIfFailed(dwriteFactory->CreateTextFormat(pFontName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"", &dwriteTextFormat));
			ThrowIfFailed(pD2dRenderTarget->CreateSolidColorBrush(d2dColor, &d2dSolidBrush));
			dwriteTextFormat->SetTextAlignment(textAlignment);
			dwriteTextFormat->SetParagraphAlignment(paragraphAlignment);
			const D2D1_SIZE_F size = pD2dRenderTarget->GetSize();
			if (!bDrawBegun)
				pD2dRenderTarget->BeginDraw();
			pD2dRenderTarget->DrawTextW(pText, lstrlenW(pText), dwriteTextFormat.Get(), pD2dRect == NULL ? D2D1::RectF(0, 0, size.width, size.height) : *pD2dRect, d2dSolidBrush.Get());
			if (!bDrawBegun)
				ThrowIfFailed(pD2dRenderTarget->EndDraw());
		}
		catch (HRESULT hr) {
			return hr;
		}
		return S_OK;
	}

	static HRESULT DrawHeart(ID2D1RenderTarget* pD2dRenderTarget) {
		try {
			const D2D1_GRADIENT_STOP d2dGradientStops[] = {
				D2D1::GradientStop(0, D2D1::ColorF(0xf02864)),
				D2D1::GradientStop(1, D2D1::ColorF(0x282864))
			};
			const D2D1_SIZE_F d2dSize = pD2dRenderTarget->GetSize();
			ID2D1Factory* pD2dFactory;
			pD2dRenderTarget->GetFactory(&pD2dFactory);
			ComPtr<ID2D1GradientStopCollection> d2dGradientStopCollection;
			ComPtr<ID2D1LinearGradientBrush> d2dLinearGradientBrush;
			ComPtr<ID2D1PathGeometry> d2dPathGeometry;
			ComPtr<ID2D1GeometrySink> d2dGeometrySink;
			ThrowIfFailed(pD2dRenderTarget->CreateGradientStopCollection(d2dGradientStops, ARRAYSIZE(d2dGradientStops), &d2dGradientStopCollection));
			ThrowIfFailed(pD2dRenderTarget->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(d2dSize.width / 2, 0), D2D1::Point2F(d2dSize.width / 2, d2dSize.height)), d2dGradientStopCollection.Get(), &d2dLinearGradientBrush));
			ThrowIfFailed(pD2dFactory->CreatePathGeometry(&d2dPathGeometry));
			ThrowIfFailed(d2dPathGeometry->Open(&d2dGeometrySink));
			/*
			Math Functions:
				f(x) = sqrt(1 - (|x| - 1) ^ 2)
				g(x) = arccos(1 - |x|) - Pi
				x: [-2, 2]
			*/
			static const FLOAT pi = 3.14159265f, radius = 1, width = radius * 4, height = radius + pi, ratio = width / height;
			const D2D1_SIZE_F d2dSizeNew = { d2dSize.width * 0.8f, d2dSize.height * 0.8f };
			const FLOAT magnification = floorf((d2dSizeNew.width / d2dSizeNew.height < ratio ? d2dSizeNew.width : d2dSizeNew.height * ratio) / width),
				actualRadius = floorf(radius * magnification) * 2;
			const D2D1_POINT_2F origin = D2D1::Point2F(d2dSize.width / 2, d2dSize.height / 2 - (height / 2 - radius) * magnification),
				d2d1StartPoint = D2D1::Point2F(origin.x - actualRadius, (-acosf(1 - 1 / magnification * actualRadius) + pi) * magnification + origin.y);
			d2dGeometrySink->BeginFigure(d2d1StartPoint, D2D1_FIGURE_BEGIN_FILLED);
			for (FLOAT x = -actualRadius + 1; x <= actualRadius; x++)
				d2dGeometrySink->AddLine(D2D1::Point2F(origin.x + x, (-acosf(1 - 1 / magnification * fabsf(x)) + pi) * magnification + origin.y));
			d2dGeometrySink->AddArc(D2D1::ArcSegment(D2D1::Point2F(origin.x, origin.y), D2D1::SizeF(actualRadius / 2, actualRadius / 2), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
			d2dGeometrySink->AddArc(D2D1::ArcSegment(d2d1StartPoint, D2D1::SizeF(actualRadius / 2, actualRadius / 2), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
			d2dGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
			ThrowIfFailed(d2dGeometrySink->Close());
			pD2dRenderTarget->BeginDraw();
			pD2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White, 0));
			pD2dRenderTarget->FillGeometry(d2dPathGeometry.Get(), d2dLinearGradientBrush.Get());
			ThrowIfFailed(pD2dRenderTarget->EndDraw());
		}
		catch (HRESULT hr) {
			return hr;
		}
		return S_OK;
	}
};