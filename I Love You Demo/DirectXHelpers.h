#pragma once

#include <Windows.h>

#include <wrl.h>

#include <d2d1_1.h>
#include <dwrite.h>

#include "ErrorHelpers.h"

namespace DirectXHelpers {
	inline HRESULT WINAPI D2D1CreateHwndRenderTarget(ID2D1Factory* pD2dFactory, HWND hWnd, ID2D1HwndRenderTarget** ppD2dHwndRenderTarget, UINT32 width = 0, UINT32 height = 0) {
		D2D1_SIZE_U d2dSize;
		if (width && height) d2dSize = D2D1::SizeU(width, height);
		else {
			RECT rc;
			if (!GetClientRect(hWnd, &rc))
				return HRESULT_FROM_WIN32(GetLastError());

			d2dSize = D2D1::SizeU(static_cast<UINT32>(rc.right), static_cast<UINT32>(rc.bottom));
		}

		return pD2dFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, d2dSize), ppD2dHwndRenderTarget);
	}

	inline HRESULT WINAPI D2D1CreateBitmapBGRA(ID2D1DeviceContext* pD2dRenderTarget, ID2D1Bitmap1** ppD2dBitmap1, UINT32 width = 0, UINT32 height = 0, D2D1_BITMAP_OPTIONS d2dBitmapOptions = D2D1_BITMAP_OPTIONS_TARGET) {
		FLOAT dpiX, dpiY;
		pD2dRenderTarget->GetDpi(&dpiX, &dpiY);
		return pD2dRenderTarget->CreateBitmap(width && height ? D2D1::SizeU(width, height) : pD2dRenderTarget->GetPixelSize(), nullptr, 0, D2D1::BitmapProperties1(d2dBitmapOptions, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), dpiX, dpiY), ppD2dBitmap1);
	}

	inline HRESULT WINAPI D2D1CreateLinearGradientBrush(ID2D1RenderTarget* pD2dRenderTarget, const D2D1_GRADIENT_STOP* pD2dGradientStops, UINT32 gradientStopsCount, const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES& pD2dLinearGradientBrushProperties, ID2D1LinearGradientBrush** ppD2dLinearGradientBrush) {
		using ErrorHelpers::ThrowIfFailed;
		using Microsoft::WRL::ComPtr;

		try {
			ComPtr<ID2D1GradientStopCollection> d2dGradientStopCollection;

			ThrowIfFailed(pD2dRenderTarget->CreateGradientStopCollection(pD2dGradientStops, gradientStopsCount, &d2dGradientStopCollection));

			ThrowIfFailed(pD2dRenderTarget->CreateLinearGradientBrush(pD2dLinearGradientBrushProperties, d2dGradientStopCollection.Get(), ppD2dLinearGradientBrush));
		}

		catch (const std::system_error& e) { return static_cast<HRESULT>(e.code().value()); }
		return S_OK;
	}

	inline HRESULT WINAPI D2D1CreateRadialGradientBrush(ID2D1RenderTarget* pD2dRenderTarget, const D2D1_GRADIENT_STOP* pD2dGradientStops, UINT32 gradientStopsCount, const D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES& pD2dRadialGradientBrushProperties, ID2D1RadialGradientBrush** ppD2dRadialGradientBrush) {
		using ErrorHelpers::ThrowIfFailed;
		using Microsoft::WRL::ComPtr;

		try {
			ComPtr<ID2D1GradientStopCollection> d2dGradientStopCollection;

			ThrowIfFailed(pD2dRenderTarget->CreateGradientStopCollection(pD2dGradientStops, gradientStopsCount, &d2dGradientStopCollection));

			ThrowIfFailed(pD2dRenderTarget->CreateRadialGradientBrush(pD2dRadialGradientBrushProperties, d2dGradientStopCollection.Get(), ppD2dRadialGradientBrush));
		}
		catch (const std::system_error& e) { return static_cast<HRESULT>(e.code().value()); }
		return S_OK;
	}

	inline HRESULT WINAPI D2D1DrawTextNormal(ID2D1RenderTarget* pD2dRenderTarget, const WCHAR* fontName, FLOAT fontSize, DWRITE_TEXT_ALIGNMENT dwriteTextAlignment, DWRITE_PARAGRAPH_ALIGNMENT dwriteParagraphAlignment, PCWSTR pcwText, const D2D1_COLOR_F& d2dColor, const D2D1_RECT_F* pD2dRect = nullptr, const WCHAR* localName = L"") {
		using ErrorHelpers::ThrowIfFailed;
		using Microsoft::WRL::ComPtr;
		try {
			const D2D1_SIZE_F d2dSize = pD2dRenderTarget->GetSize();

			ComPtr<IDWriteFactory> dwriteFactory;
			ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwriteFactory.Get()), &dwriteFactory));

			ComPtr<IDWriteTextFormat> dwriteTextFormat;
			ThrowIfFailed(dwriteFactory->CreateTextFormat(fontName, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, localName, &dwriteTextFormat));

			ComPtr<ID2D1SolidColorBrush> d2dSolidBrush;
			ThrowIfFailed(pD2dRenderTarget->CreateSolidColorBrush(d2dColor, &d2dSolidBrush));

			ThrowIfFailed(dwriteTextFormat->SetTextAlignment(dwriteTextAlignment));

			ThrowIfFailed(dwriteTextFormat->SetParagraphAlignment(dwriteParagraphAlignment));

			pD2dRenderTarget->DrawTextW(pcwText, lstrlenW(pcwText), dwriteTextFormat.Get(), pD2dRect ? *pD2dRect : D2D1::RectF(0, 0, d2dSize.width, d2dSize.height), d2dSolidBrush.Get());
		}
		catch (const std::system_error& e) { return static_cast<HRESULT>(e.code().value()); }
		return S_OK;
	}
}
