#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include "SystemErrorHelpers.h"

namespace Hydr10n {
	namespace DirectXHelpers {
		using namespace SystemErrorHelpers;
		using Microsoft::WRL::ComPtr;

		inline HRESULT WINAPI GetDeviceContext(ID2D1RenderTarget* pD2dRenderTarget, ID2D1DeviceContext** ppD2dDeviceContext) { return pD2dRenderTarget->QueryInterface(ppD2dDeviceContext); }

		inline HRESULT WINAPI CreateHwndRenderTarget(ID2D1Factory* pD2dFactory, HWND hWnd, ID2D1HwndRenderTarget** ppD2dHwndRenderTarget) {
			RECT rc;
			GetClientRect(hWnd, &rc);
			return pD2dFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(rc.right, rc.bottom)), ppD2dHwndRenderTarget);
		}

		inline HRESULT WINAPI CreateLinearGradientBrush(ID2D1RenderTarget* pD2dRenderTarget, const D2D1_GRADIENT_STOP* pD2dGradientStops,
			UINT32 gradientStopsCount, const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES& pD2dLinearGradientBrushProperties, ID2D1LinearGradientBrush** ppLinearGradientBrush) {
			return CatchHRESULT([&] {
				ComPtr<ID2D1GradientStopCollection> d2dGradientStopCollection;
				ThrowIfFailed(pD2dRenderTarget->CreateGradientStopCollection(pD2dGradientStops, gradientStopsCount, &d2dGradientStopCollection));
				ThrowIfFailed(pD2dRenderTarget->CreateLinearGradientBrush(pD2dLinearGradientBrushProperties, d2dGradientStopCollection.Get(), ppLinearGradientBrush));
				});
		}

		HRESULT WINAPI DrawTextNormal(ID2D1RenderTarget* pD2dRenderTarget, const WCHAR* fontName, FLOAT fontSize, DWRITE_TEXT_ALIGNMENT dwriteTextAlignment, DWRITE_PARAGRAPH_ALIGNMENT dwriteParagraphAlignment, PCWSTR pcwText, const D2D1_COLOR_F& d2dColor, const D2D1_RECT_F* pD2dRect = nullptr, const WCHAR* localName = L"") {
			return CatchHRESULT([&] {
				const D2D1_SIZE_F d2dSize = pD2dRenderTarget->GetSize();
				ComPtr<IDWriteFactory> dwriteFactory;
				ComPtr<IDWriteTextFormat> dwriteTextFormat;
				ComPtr<ID2D1SolidColorBrush> d2dSolidBrush;
				ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwriteFactory.Get()), &dwriteFactory));
				ThrowIfFailed(dwriteFactory->CreateTextFormat(fontName, NULL, DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, localName, &dwriteTextFormat));
				ThrowIfFailed(pD2dRenderTarget->CreateSolidColorBrush(d2dColor, &d2dSolidBrush));
				ThrowIfFailed(dwriteTextFormat->SetTextAlignment(dwriteTextAlignment));
				ThrowIfFailed(dwriteTextFormat->SetParagraphAlignment(dwriteParagraphAlignment));
				pD2dRenderTarget->DrawTextW(pcwText, lstrlenW(pcwText), dwriteTextFormat.Get(), pD2dRect == nullptr ? D2D1::RectF(0, 0, d2dSize.width, d2dSize.height) : *pD2dRect, d2dSolidBrush.Get());
				});
		}
	}
}