/*
 * Header File: ILoveYouDemo.h
 * Last Update: 2021/09/03
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include "StepTimer.h"

#include "DirectXHelpers.h"

#include <d2d1effects.h>

#include <string>
#include <set>

#include <algorithm>

constexpr float CalculateAverageVelocity(float value, float millionSeconds, float FPS) { return value / (millionSeconds / 1000 * FPS); }

namespace Hydr10n {
	namespace Demos {
		class ILoveYou {
		public:
			enum class Animation { Glow, Rotation };

			ILoveYou(HWND hWnd, const SIZE& outputSize) noexcept(false) : m_hWnd(hWnd) {
				ErrorHelpers::ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_D2dFactory.ReleaseAndGetAddressOf()));

				OnWindowSizeChanged(outputSize);
			}

			SIZE GetOutputSize() const { return m_OutputSize; }

			void Tick() {
				m_StepTimer.Tick([&] {
					if (m_IsRunning)
						Update();
					});

				Render();
			}

			void OnWindowSizeChanged(const SIZE& size) {
				if (m_OutputSize.cx == size.cx && m_OutputSize.cy == size.cy)
					return;

				m_OutputSize = size;

				CreateResources();
			}

			void OnResuming() {
				if (!m_IsRunning) {
					m_IsRunning = true;

					m_StepTimer.ResetElapsedTime();
				}
			}

			void OnSuspending() { m_IsRunning = false; }

			void OnMouseLeftButtonDown(WPARAM wParam, LPARAM lParam) { m_CursorCoordinate = MAKEPOINTS(lParam); }

			void OnMouseMove(WPARAM wParam, LPARAM lParam) {
				if (wParam & MK_LBUTTON) {
					const auto points = MAKEPOINTS(lParam);

					auto foregroundRotation = m_ForegroundRotation;
					foregroundRotation.y += 0.25f * (points.x - m_CursorCoordinate.x);
					SetForegroundRotation(foregroundRotation);

					m_CursorCoordinate = points;
				}
			}

			void OnMouseWheel(WPARAM wParam, LPARAM lParam) { SetForegroundGlowRadiusScale(m_ForegroundGlowRadiusScale + 0.1f * GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA); }

			void ShowFPS(bool visible) { m_IsFPSVisible = visible; }

			bool IsFPSVisible() const { return m_IsFPSVisible; }

			void ReverseAnimationState(Animation animation) {
				if (m_Animations.contains(animation))
					m_Animations.erase(animation);
				else
					m_Animations.insert(animation);
			}

			const std::set<Animation>& GetAnimations() const { return m_Animations; }

			void ResetForegroundGlowRadiusScale() { SetForegroundGlowRadiusScale(0); }

			void ResetForegroundRotation() { SetForegroundRotation(D2D1::Vector3F()); }

			void ReverseRotation() { m_IsRotationClockwise = !m_IsRotationClockwise; }

			bool IsRotationClockwise() const { return m_IsRotationClockwise; }

		private:
			static constexpr FLOAT TargetFPS = 60, DefaultDeviceIndependentHeight = 800, GlowAnimationDuration = 1250, RotationAnimationDuration = GlowAnimationDuration * 2;

			const HWND m_hWnd;

			bool m_IsRunning{}, m_IsFPSVisible{}, m_IsGlowFadeIn = true, m_IsRotationClockwise = true;

			FLOAT m_Scale, m_MaxForegroundGlowRadius, m_ForegroundGlowRadiusScale{};

			POINTS m_CursorCoordinate{};

			SIZE m_OutputSize;

			D2D1_SIZE_F m_D2dSize;

			D2D1_VECTOR_3F m_ForegroundRotation{};

			std::set<Animation> m_Animations{ Animation::Glow, Animation::Rotation };

			DX::StepTimer m_StepTimer;

			Microsoft::WRL::ComPtr<ID2D1Factory> m_D2dFactory;
			Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_D2dDeviceContext;
			Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_D2dBackgroundBitmap;
			Microsoft::WRL::ComPtr<ID2D1Effect> m_D2dEffectShadow, m_D2dEffect3DPerspectiveTransform;

			void SetForegroundGlowRadiusScale(FLOAT scale) {
				scale = std::clamp(scale, 0.f, 1.f);

				ErrorHelpers::ThrowIfFailed(m_D2dEffectShadow->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, m_MaxForegroundGlowRadius * scale));

				m_ForegroundGlowRadiusScale = scale;
			}

			void SetForegroundRotation(const D2D1_VECTOR_3F& rotation) {
				const D2D1_VECTOR_3F correctedRotation = D2D1::Vector3F(rotation.x - static_cast<int>(rotation.x / 360) * 360, rotation.y - static_cast<int>(rotation.y / 360) * 360, rotation.z - static_cast<int>(rotation.z / 360) * 360);

				ErrorHelpers::ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION, correctedRotation));

				m_ForegroundRotation = correctedRotation;
			}

			void CreateResources() {
				using namespace DirectXHelpers;
				using ErrorHelpers::ThrowIfFailed;

				Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> d2dHwndRenderTarget;
				ThrowIfFailed(D2D1CreateHwndRenderTarget(m_D2dFactory.Get(), m_hWnd, &d2dHwndRenderTarget, static_cast<UINT32>(m_OutputSize.cx), static_cast<UINT32>(m_OutputSize.cy)));

				ThrowIfFailed(d2dHwndRenderTarget->QueryInterface(m_D2dDeviceContext.ReleaseAndGetAddressOf()));

				m_D2dSize = m_D2dDeviceContext->GetSize();

				m_Scale = m_D2dSize.height / DefaultDeviceIndependentHeight;

				ThrowIfFailed(D2D1CreateBitmapBGRA(m_D2dDeviceContext.Get(), m_D2dBackgroundBitmap.ReleaseAndGetAddressOf()));

				Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dBitmap;
				ThrowIfFailed(D2D1CreateBitmapBGRA(m_D2dDeviceContext.Get(), &d2dBitmap));

				CreateEffects(d2dBitmap.Get());

				DrawImages(d2dBitmap.Get());
			}

			void CreateEffects(ID2D1Bitmap1* pD2dBitmap1) {
				using namespace D2D1;
				using ErrorHelpers::ThrowIfFailed;

				Microsoft::WRL::ComPtr<ID2D1Effect> d2dEffectComposite;
				ThrowIfFailed(m_D2dDeviceContext->CreateEffect(CLSID_D2D1Composite, &d2dEffectComposite));

				ThrowIfFailed(m_D2dDeviceContext->CreateEffect(CLSID_D2D1Shadow, m_D2dEffectShadow.ReleaseAndGetAddressOf()));

				ThrowIfFailed(m_D2dDeviceContext->CreateEffect(CLSID_D2D13DPerspectiveTransform, m_D2dEffect3DPerspectiveTransform.ReleaseAndGetAddressOf()));

				ThrowIfFailed(m_D2dEffectShadow->SetValue(D2D1_SHADOW_PROP_COLOR, Vector4F(1, 1, 1, 0.5f)));

				ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION_ORIGIN, Vector3F(m_D2dSize.width / 2)));
				ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_PERSPECTIVE_ORIGIN, Vector2F(m_D2dSize.width / 2, m_D2dSize.height / 2)));

				ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_DEPTH, 1200 * m_Scale));
				m_D2dEffectShadow->SetInput(0, pD2dBitmap1);

				d2dEffectComposite->SetInputEffect(0, m_D2dEffectShadow.Get());

				d2dEffectComposite->SetInput(1, pD2dBitmap1);

				m_D2dEffect3DPerspectiveTransform->SetInputEffect(0, d2dEffectComposite.Get());
			}

			void DrawImages(ID2D1Bitmap1* pD2dBitmap1) {
				using namespace D2D1;
				using DirectXHelpers::D2D1DrawTextNormal;
				using ErrorHelpers::ThrowIfFailed;

				constexpr FLOAT HeartScale = 0.8f;

				Microsoft::WRL::ComPtr<ID2D1Image> d2dImage;
				m_D2dDeviceContext->GetTarget(&d2dImage);

				m_D2dDeviceContext->SetTarget(m_D2dBackgroundBitmap.Get());

				m_D2dDeviceContext->BeginDraw();

				DrawBackground(m_D2dDeviceContext.Get());

				const auto rect = RectF(0, m_D2dSize.height * (0.5f + HeartScale / 2), m_D2dSize.width, m_D2dSize.height);
				ThrowIfFailed(D2D1DrawTextNormal(m_D2dDeviceContext.Get(), L"Segoe UI", 24 * m_Scale, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"Copyright © Programmer-Yang_Xun@outlook.com", ColorF(ColorF::White), &rect));
				
				m_D2dDeviceContext->SetTarget(pD2dBitmap1);

				m_D2dDeviceContext->Clear(ColorF(ColorF::White, 0));

				DrawHeart(m_D2dDeviceContext.Get(), HeartScale);

				ThrowIfFailed(D2D1DrawTextNormal(m_D2dDeviceContext.Get(), L"Comic Sans MS", 46 * m_Scale, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"I LOVE YOU\nFOREVER", ColorF(ColorF::White)));
				ThrowIfFailed(m_D2dDeviceContext->EndDraw());

				m_D2dDeviceContext->SetTarget(d2dImage.Get());

				m_MaxForegroundGlowRadius = m_Scale * HeartScale * 50;
				SetForegroundGlowRadiusScale(m_ForegroundGlowRadiusScale);

				SetForegroundRotation(m_ForegroundRotation);
			}

			void DrawFPS() {
				using namespace D2D1;

				const auto fontSize = 22 * m_Scale;
				const auto rect = RectF(0, 0, m_D2dSize.width, fontSize);
				ErrorHelpers::ThrowIfFailed(DirectXHelpers::D2D1DrawTextNormal(m_D2dDeviceContext.Get(), L"Segoe UI", fontSize, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, (L"FPS: " + std::to_wstring(m_StepTimer.GetFramesPerSecond())).c_str(), ColorF(ColorF::White), &rect));
			}

			void Update() {
				if (m_StepTimer.GetFrameCount() < 2)
					return;

				if (m_Animations.contains(Animation::Glow)) {
					SetForegroundGlowRadiusScale(m_ForegroundGlowRadiusScale + (m_IsGlowFadeIn ? 1 : -1) * CalculateAverageVelocity(1, GlowAnimationDuration, static_cast<float>(TargetFPS)));

					if (m_ForegroundGlowRadiusScale >= 1)
						m_IsGlowFadeIn = false;
					else if (m_ForegroundGlowRadiusScale <= 0)
						m_IsGlowFadeIn = true;
				}

				if (m_Animations.contains(Animation::Rotation)) {
					auto foregroundRotation = m_ForegroundRotation;
					foregroundRotation.y += (m_IsRotationClockwise ? -1 : 1) * CalculateAverageVelocity(360, RotationAnimationDuration, static_cast<float>(TargetFPS));
					SetForegroundRotation(foregroundRotation);
				}
			}

			void Render() {
				if (!m_StepTimer.GetFrameCount())
					return;

				m_D2dDeviceContext->BeginDraw();

				m_D2dDeviceContext->DrawImage(m_D2dBackgroundBitmap.Get());

				m_D2dDeviceContext->DrawImage(m_D2dEffect3DPerspectiveTransform.Get());

				if (m_IsFPSVisible)
					DrawFPS();

				ErrorHelpers::ThrowIfFailed(m_D2dDeviceContext->EndDraw());
			}

			static void DrawBackground(ID2D1RenderTarget* pD2dRenderTarget) {
				using namespace D2D1;

				const D2D1_GRADIENT_STOP d2dGradientStops[]{
					GradientStop(0, ColorF(0x157050)),
					GradientStop(1, ColorF(0x155799))
				};

				const auto size = pD2dRenderTarget->GetSize();

				Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> d2dLinearGradientBrush;
				ErrorHelpers::ThrowIfFailed(DirectXHelpers::D2D1CreateLinearGradientBrush(pD2dRenderTarget, d2dGradientStops, ARRAYSIZE(d2dGradientStops), LinearGradientBrushProperties(Point2F(), Point2F(size.width, size.height)), &d2dLinearGradientBrush));

				pD2dRenderTarget->FillRectangle(RectF(0, 0, size.width, size.height), d2dLinearGradientBrush.Get());
			}

			static void DrawHeart(ID2D1RenderTarget* pD2dRenderTarget, FLOAT scale) {
				using namespace D2D1;
				using Microsoft::WRL::ComPtr;
				using ErrorHelpers::ThrowIfFailed;

				const D2D1_GRADIENT_STOP d2dGradientStops[]{
					GradientStop(0, ColorF(0xf02864)),
					GradientStop(1, ColorF(0x282864))
				};

				const auto size = pD2dRenderTarget->GetSize();

				ComPtr<ID2D1Factory> d2dFactory;
				pD2dRenderTarget->GetFactory(&d2dFactory);

				ComPtr<ID2D1PathGeometry> d2dPathGeometry;
				ThrowIfFailed(d2dFactory->CreatePathGeometry(&d2dPathGeometry));

				ComPtr<ID2D1GeometrySink> d2dGeometrySink;
				ThrowIfFailed(d2dPathGeometry->Open(&d2dGeometrySink));

				ComPtr<ID2D1LinearGradientBrush> d2dLinearGradientBrush;
				ThrowIfFailed(DirectXHelpers::D2D1CreateLinearGradientBrush(pD2dRenderTarget, d2dGradientStops, ARRAYSIZE(d2dGradientStops), LinearGradientBrushProperties(Point2F(size.width / 2), Point2F(size.width / 2, size.height)), &d2dLinearGradientBrush));

				/*
				 * Math Functions:
				 * f(x) = sqrt(1 - (|x| - 1) ^ 2)
				 * g(x) = arccos(1 - |x|) - Pi
				 * x: [-2, 2]
				 */

				constexpr auto Pi = 3.14159265f, Radius = 1.f, Width = Radius * 4, Height = Radius + Pi, Ratio = Width / Height;
				scale *= (size.width / size.height < Ratio ? size.width : size.height * Ratio) / Width;
				const auto diameter = floorf(Radius * scale * 2);
				const auto origin = Point2F(size.width / 2, size.height / 2 - scale * (Height / 2 - Radius)), startPoint = Point2F(origin.x - diameter, origin.y + (-acosf(1 - 1 / scale * diameter) + Pi) * scale);

				d2dGeometrySink->BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

				for (auto x = -diameter + 1; x <= diameter; x++)
					d2dGeometrySink->AddLine(Point2F(origin.x + x, origin.y + (-acosf(1 - 1 / scale * fabsf(x)) + Pi) * scale));

				d2dGeometrySink->AddArc(ArcSegment(origin, SizeF(diameter / 2, diameter / 2), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

				d2dGeometrySink->AddArc(ArcSegment(startPoint, SizeF(diameter / 2, diameter / 2), 0, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

				d2dGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);

				ThrowIfFailed(d2dGeometrySink->Close());

				pD2dRenderTarget->FillGeometry(d2dPathGeometry.Get(), d2dLinearGradientBrush.Get());
			}
		};
	}
}
