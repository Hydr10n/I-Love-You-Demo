/*
 * Header File: ILoveYouDemo.h
 * Last Update: 2021/11/21
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
				ErrorHelpers::ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.ReleaseAndGetAddressOf()));

				OnWindowSizeChanged(outputSize);
			}

			SIZE GetOutputSize() const { return m_outputSize; }

			void Tick() {
				m_stepTimer.Tick([&] { if (m_isRunning) Update(); });

				Render();
			}

			void OnWindowSizeChanged(const SIZE& size) {
				if (m_outputSize.cx == size.cx && m_outputSize.cy == size.cy) return;

				m_outputSize = size;

				CreateResources();
			}

			void OnResuming() {
				if (!m_isRunning) {
					m_isRunning = true;

					m_stepTimer.ResetElapsedTime();
				}
			}

			void OnSuspending() { m_isRunning = false; }

			void OnMouseLeftButtonDown(WPARAM wParam, LPARAM lParam) { m_cursorCoordinate = MAKEPOINTS(lParam); }

			void OnMouseMove(WPARAM wParam, LPARAM lParam) {
				if (wParam & MK_LBUTTON) {
					const auto points = MAKEPOINTS(lParam);

					auto foregroundRotation = m_foregroundRotation;
					foregroundRotation.y += 0.25f * (points.x - m_cursorCoordinate.x);
					SetForegroundRotation(foregroundRotation);

					m_cursorCoordinate = points;
				}
			}

			void OnMouseWheel(WPARAM wParam, LPARAM lParam) { SetForegroundGlowRadiusScale(m_foregroundGlowRadiusScale + 0.1f * GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA); }

			void ShowFPS(bool visible) { m_isFPSVisible = visible; }

			bool IsFPSVisible() const { return m_isFPSVisible; }

			void ReverseAnimationState(Animation animation) {
				if (m_animations.contains(animation)) m_animations.erase(animation);
				else m_animations.insert(animation);
			}

			const std::set<Animation>& GetAnimations() const { return m_animations; }

			void ResetForegroundGlowRadiusScale() { SetForegroundGlowRadiusScale(0); }

			void ResetForegroundRotation() { SetForegroundRotation(D2D1::Vector3F()); }

			void ReverseRotation() { m_isRotationClockwise = !m_isRotationClockwise; }

			bool IsRotationClockwise() const { return m_isRotationClockwise; }

		private:
			static constexpr FLOAT TargetFPS = 60, DefaultDeviceIndependentHeight = 800, GlowAnimationDuration = 1250, RotationAnimationDuration = GlowAnimationDuration * 2;

			const HWND m_hWnd;

			bool m_isRunning{}, m_isFPSVisible{}, m_isGlowFadeIn = true, m_isRotationClockwise = true;

			FLOAT m_scale, m_maxForegroundGlowRadius, m_foregroundGlowRadiusScale{};

			POINTS m_cursorCoordinate{};

			SIZE m_outputSize;

			D2D1_SIZE_F m_d2dSize;

			D2D1_VECTOR_3F m_foregroundRotation{};

			std::set<Animation> m_animations{ Animation::Glow, Animation::Rotation };

			DX::StepTimer m_stepTimer;

			Microsoft::WRL::ComPtr<ID2D1Factory> m_d2dFactory;
			Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_d2dDeviceContext;
			Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dBackgroundBitmap;
			Microsoft::WRL::ComPtr<ID2D1Effect> m_d2dEffectShadow, m_d2dEffect3DPerspectiveTransform;

			void SetForegroundGlowRadiusScale(FLOAT scale) {
				scale = std::clamp(scale, 0.f, 1.f);

				ErrorHelpers::ThrowIfFailed(m_d2dEffectShadow->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, m_maxForegroundGlowRadius * scale));

				m_foregroundGlowRadiusScale = scale;
			}

			void SetForegroundRotation(const D2D1_VECTOR_3F& rotation) {
				const D2D1_VECTOR_3F correctedRotation = D2D1::Vector3F(rotation.x - static_cast<int>(rotation.x / 360) * 360, rotation.y - static_cast<int>(rotation.y / 360) * 360, rotation.z - static_cast<int>(rotation.z / 360) * 360);

				ErrorHelpers::ThrowIfFailed(m_d2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION, correctedRotation));

				m_foregroundRotation = correctedRotation;
			}

			void CreateResources() {
				using namespace DirectXHelpers;
				using ErrorHelpers::ThrowIfFailed;

				Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> d2dHwndRenderTarget;
				ThrowIfFailed(D2D1CreateHwndRenderTarget(m_d2dFactory.Get(), m_hWnd, &d2dHwndRenderTarget, static_cast<UINT32>(m_outputSize.cx), static_cast<UINT32>(m_outputSize.cy)));

				ThrowIfFailed(d2dHwndRenderTarget->QueryInterface(m_d2dDeviceContext.ReleaseAndGetAddressOf()));

				m_d2dSize = m_d2dDeviceContext->GetSize();

				m_scale = m_d2dSize.height / DefaultDeviceIndependentHeight;

				ThrowIfFailed(D2D1CreateBitmapBGRA(m_d2dDeviceContext.Get(), m_d2dBackgroundBitmap.ReleaseAndGetAddressOf()));

				Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dBitmap;
				ThrowIfFailed(D2D1CreateBitmapBGRA(m_d2dDeviceContext.Get(), &d2dBitmap));

				CreateEffects(d2dBitmap.Get());

				DrawImages(d2dBitmap.Get());
			}

			void CreateEffects(ID2D1Bitmap1* pD2dBitmap1) {
				using namespace D2D1;
				using ErrorHelpers::ThrowIfFailed;

				Microsoft::WRL::ComPtr<ID2D1Effect> d2dEffectComposite;
				ThrowIfFailed(m_d2dDeviceContext->CreateEffect(CLSID_D2D1Composite, &d2dEffectComposite));

				ThrowIfFailed(m_d2dDeviceContext->CreateEffect(CLSID_D2D1Shadow, m_d2dEffectShadow.ReleaseAndGetAddressOf()));

				ThrowIfFailed(m_d2dDeviceContext->CreateEffect(CLSID_D2D13DPerspectiveTransform, m_d2dEffect3DPerspectiveTransform.ReleaseAndGetAddressOf()));

				ThrowIfFailed(m_d2dEffectShadow->SetValue(D2D1_SHADOW_PROP_COLOR, Vector4F(1, 1, 1, 0.5f)));

				ThrowIfFailed(m_d2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION_ORIGIN, Vector3F(m_d2dSize.width / 2)));
				ThrowIfFailed(m_d2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_PERSPECTIVE_ORIGIN, Vector2F(m_d2dSize.width / 2, m_d2dSize.height / 2)));

				ThrowIfFailed(m_d2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP_DEPTH, 1200 * m_scale));
				m_d2dEffectShadow->SetInput(0, pD2dBitmap1);

				d2dEffectComposite->SetInputEffect(0, m_d2dEffectShadow.Get());

				d2dEffectComposite->SetInput(1, pD2dBitmap1);

				m_d2dEffect3DPerspectiveTransform->SetInputEffect(0, d2dEffectComposite.Get());
			}

			void DrawImages(ID2D1Bitmap1* pD2dBitmap1) {
				using namespace D2D1;
				using DirectXHelpers::D2D1DrawTextNormal;
				using ErrorHelpers::ThrowIfFailed;

				constexpr FLOAT HeartScale = 0.8f;

				Microsoft::WRL::ComPtr<ID2D1Image> d2dImage;
				m_d2dDeviceContext->GetTarget(&d2dImage);

				m_d2dDeviceContext->SetTarget(m_d2dBackgroundBitmap.Get());

				m_d2dDeviceContext->BeginDraw();

				DrawBackground(m_d2dDeviceContext.Get());

				const auto rect = RectF(0, m_d2dSize.height * (0.5f + HeartScale / 2), m_d2dSize.width, m_d2dSize.height);
				ThrowIfFailed(D2D1DrawTextNormal(m_d2dDeviceContext.Get(), L"Segoe UI", 24 * m_scale, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"Copyright © Programmer-Yang_Xun@outlook.com", ColorF(ColorF::White), &rect));

				m_d2dDeviceContext->SetTarget(pD2dBitmap1);

				m_d2dDeviceContext->Clear(ColorF(ColorF::White, 0));

				DrawHeart(m_d2dDeviceContext.Get(), HeartScale);

				ThrowIfFailed(D2D1DrawTextNormal(m_d2dDeviceContext.Get(), L"Comic Sans MS", 46 * m_scale, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"I LOVE YOU\nFOREVER", ColorF(ColorF::White)));

				ThrowIfFailed(m_d2dDeviceContext->EndDraw());

				m_d2dDeviceContext->SetTarget(d2dImage.Get());

				m_maxForegroundGlowRadius = m_scale * HeartScale * 50;
				SetForegroundGlowRadiusScale(m_foregroundGlowRadiusScale);

				SetForegroundRotation(m_foregroundRotation);
			}

			void DrawFPS() {
				using namespace D2D1;

				const auto fontSize = 22 * m_scale;
				const auto rect = RectF(0, 0, m_d2dSize.width, fontSize);
				ErrorHelpers::ThrowIfFailed(DirectXHelpers::D2D1DrawTextNormal(m_d2dDeviceContext.Get(), L"Segoe UI", fontSize, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, (L"FPS: " + std::to_wstring(m_stepTimer.GetFramesPerSecond())).c_str(), ColorF(ColorF::White), &rect));
			}

			void Update() {
				if (m_stepTimer.GetFrameCount() < 2) return;

				if (m_animations.contains(Animation::Glow)) {
					SetForegroundGlowRadiusScale(m_foregroundGlowRadiusScale + (m_isGlowFadeIn ? 1 : -1) * CalculateAverageVelocity(1, GlowAnimationDuration, static_cast<float>(TargetFPS)));

					if (m_foregroundGlowRadiusScale >= 1) m_isGlowFadeIn = false;
					else if (m_foregroundGlowRadiusScale <= 0) m_isGlowFadeIn = true;
				}

				if (m_animations.contains(Animation::Rotation)) {
					auto foregroundRotation = m_foregroundRotation;
					foregroundRotation.y += (m_isRotationClockwise ? -1 : 1) * CalculateAverageVelocity(360, RotationAnimationDuration, static_cast<float>(TargetFPS));
					SetForegroundRotation(foregroundRotation);
				}
			}

			void Render() {
				if (!m_stepTimer.GetFrameCount()) return;

				m_d2dDeviceContext->BeginDraw();

				m_d2dDeviceContext->DrawImage(m_d2dBackgroundBitmap.Get());

				m_d2dDeviceContext->DrawImage(m_d2dEffect3DPerspectiveTransform.Get());

				if (m_isFPSVisible) DrawFPS();

				ErrorHelpers::ThrowIfFailed(m_d2dDeviceContext->EndDraw());
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
