/*
 * Header File: ILoveYouDemo.h
 * Last Update: 2020/11/11
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include "pch.h"
#include "set_helper.h"
#include "StepTimer.h"
#include <d2d1effects.h>
#include <set>

constexpr float CalculateTransitionSpeed(float value, float millionSeconds, float FPS) { return value / (millionSeconds / 1000 * FPS); }

namespace Hydr10n {
	namespace Demos {
		class ILoveYou final {
		public:
			class AnimationSet final {
			public:
				enum class AnimationType { Glow, Rotation };

				AnimationSet(const std::initializer_list<AnimationType>& animationTypes) : m_AnimationTypes(animationTypes) {}

				bool Add(AnimationType animationType) { return std_container_helpers::set_helper::modify(m_AnimationTypes, animationType, false); }

				bool Remove(AnimationType animationType) { return std_container_helpers::set_helper::modify(m_AnimationTypes, animationType, true); }

				bool Contains(AnimationType animationType) const { return m_AnimationTypes.find(animationType) != m_AnimationTypes.cend(); }

				bool IsEmpty() const { return m_AnimationTypes.empty(); }

			private:
				std::set<AnimationType> m_AnimationTypes;
			};

			ILoveYou(HWND hWnd, UINT32 pixelWidth = 0, UINT32 pixelHeight = 0) noexcept(false) : m_hWnd(hWnd) {
				SystemErrorHelpers::ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED, (IUnknown**)&m_D2dFactory));
				Resize(pixelWidth, pixelHeight);
				m_StepTimer.SetFixedTimeStep(true);
				m_StepTimer.SetTargetElapsedSeconds(1.0 / FPS);
				m_IsInitialized = true;
			}

			void Resize(UINT32 pixelWidth, UINT32 pixelHeight) {
				const bool isInitialized = m_IsInitialized;
				m_IsInitialized = false;
				CreateGraphicsResources(pixelWidth, pixelHeight);
				CreateGraphicsEffects();
				DrawImages();
				if (m_IsInitialized = isInitialized)
					Render();
			}

			void Tick(bool update = true) {
				if (!update)
					Render();
				else if (IsRunning() && !m_AnimationSet.IsEmpty()) {
					m_StepTimer.Tick([&] { Update(); });
					Render();
				}
			}

			void ShowFramesPerSecond(bool visible) {
				m_IsFramesPerSecondVisible = visible;
				if (!IsRunning() || m_AnimationSet.IsEmpty())
					Render();
			}

			bool IsFramesPerSecondVisible() const noexcept { return m_IsFramesPerSecondVisible; }

			void Pause() {
				if (m_IsRunning) {
					m_IsRunning = false;
					Render();
				}
			}

			void Resume() {
				if (!m_IsRunning) {
					m_IsRunning = true;
					m_StepTimer.ResetElapsedTime();
				}
			}

			bool IsRunning() const noexcept { return m_IsRunning; }

			void ReverseAnimationState(AnimationSet::AnimationType animationType) {
				if (m_AnimationSet.Contains(animationType)) {
					m_AnimationSet.Remove(animationType);
					if (m_AnimationSet.IsEmpty())
						Pause();
				}
				else {
					m_AnimationSet.Add(animationType);
					Resume();
				}
			}

			const AnimationSet& GetAnimationSet() const noexcept { return m_AnimationSet; }

			void SetForegroundGlowRadiusScale(FLOAT scale = 0) {
				using namespace SystemErrorHelpers;
				if (!m_D2dEffectShadow)
					throw_system_error((int)D2DERR_NOT_INITIALIZED);
				scale = min(1, max(0, scale));
				ThrowIfFailed(m_D2dEffectShadow->SetValue(D2D1_SHADOW_PROP::D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, m_MaxForegroundGlowRadius * scale));
				m_ForegroundGlowRadiusScale = scale;
				if (m_IsInitialized && !IsRunning())
					Render();
			}

			FLOAT GetForegroundGlowRadiusScale() const noexcept { return m_ForegroundGlowRadiusScale; }

			void SetForegroundRotationY(FLOAT y = 0) {
				using namespace SystemErrorHelpers;
				if (!m_D2dEffect3DPerspectiveTransform)
					throw_system_error((int)D2DERR_NOT_INITIALIZED);
				y -= int(y / 360) * 360;
				ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION, D2D1::Vector3F(0, y)));
				m_ForegroundRotationY = y;
				if (m_IsInitialized && !IsRunning())
					Render();
			}

			FLOAT GetForegroundRotationY() const noexcept { return m_ForegroundRotationY; }

			void ReverseRotation() noexcept { m_IsRotationClockwise = !m_IsRotationClockwise; }

			bool IsRotationClockwise() const noexcept { return m_IsRotationClockwise; }

		private:
			static constexpr FLOAT FPS = 60,
				DefaultDeviceIndependentHeight = 800,
				GlowAnimationDuration = 1250, RotationAnimationDuration = GlowAnimationDuration * 2;

			bool m_IsInitialized{}, m_IsRunning{ true }, m_IsFramesPerSecondVisible{ true }, m_IsGlowFadeIn{ true }, m_IsRotationClockwise{ true };
			FLOAT m_Scale, m_MaxForegroundGlowRadius, m_ForegroundGlowRadiusScale{}, m_ForegroundRotationY{};
			HWND m_hWnd;
			D2D1_SIZE_F m_D2dSize;
			AnimationSet m_AnimationSet{ AnimationSet::AnimationType::Glow, AnimationSet::AnimationType::Rotation };
			DX::StepTimer m_StepTimer;
			Microsoft::WRL::ComPtr<ID2D1Factory> m_D2dFactory;
			Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_D2dDeviceContext;
			Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_D2dBitmap1Background, m_D2dBitmap1Foreground;
			Microsoft::WRL::ComPtr<ID2D1Effect> m_D2dEffect, m_D2dEffectShadow, m_D2dEffect3DPerspectiveTransform;

			void CreateGraphicsResources(UINT32 pixelWidth, UINT32 pixelHeight) {
				using namespace DirectXHelpers;
				using SystemErrorHelpers::ThrowIfFailed;
				m_D2dDeviceContext.Reset();
				m_D2dBitmap1Background.Reset();
				m_D2dBitmap1Foreground.Reset();
				Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> d2dHwndRenderTarget;
				ThrowIfFailed(D2D1CreateHwndRenderTarget(m_D2dFactory.Get(), m_hWnd, &d2dHwndRenderTarget, pixelWidth, pixelHeight));
				ThrowIfFailed(D2D1GetDeviceContext(d2dHwndRenderTarget.Get(), &m_D2dDeviceContext));
				ThrowIfFailed(D2D1CreateBitmapBGRA(m_D2dDeviceContext.Get(), &m_D2dBitmap1Background));
				ThrowIfFailed(D2D1CreateBitmapBGRA(m_D2dDeviceContext.Get(), &m_D2dBitmap1Foreground));
				m_D2dSize = m_D2dDeviceContext->GetSize();
				m_Scale = m_D2dSize.height / DefaultDeviceIndependentHeight;
			}

			void CreateGraphicsEffects() {
				using namespace D2D1;
				using SystemErrorHelpers::ThrowIfFailed;
				m_D2dEffectShadow.Reset();
				m_D2dEffect3DPerspectiveTransform.Reset();
				m_D2dEffect.Reset();
				Microsoft::WRL::ComPtr<ID2D1Effect> d2dEffectComposite;
				ThrowIfFailed(m_D2dDeviceContext->CreateEffect(CLSID_D2D1Shadow, &m_D2dEffectShadow));
				ThrowIfFailed(m_D2dDeviceContext->CreateEffect(CLSID_D2D1Composite, &d2dEffectComposite));
				ThrowIfFailed(m_D2dDeviceContext->CreateEffect(CLSID_D2D13DPerspectiveTransform, &m_D2dEffect3DPerspectiveTransform));
				ThrowIfFailed(m_D2dEffectShadow->SetValue(D2D1_SHADOW_PROP::D2D1_SHADOW_PROP_COLOR, Vector4F(1, 1, 1, 0.5f)));
				ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION_ORIGIN, Vector3F(m_D2dSize.width / 2)));
				ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_PERSPECTIVE_ORIGIN, Vector2F(m_D2dSize.width / 2, m_D2dSize.height / 2)));
				ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_DEPTH, 1200 * m_Scale));
				m_D2dEffectShadow->SetInput(0, m_D2dBitmap1Foreground.Get());
				d2dEffectComposite->SetInputEffect(0, m_D2dEffectShadow.Get());
				d2dEffectComposite->SetInput(1, m_D2dBitmap1Foreground.Get());
				m_D2dEffect3DPerspectiveTransform->SetInputEffect(0, d2dEffectComposite.Get());
				m_D2dEffect = m_D2dEffect3DPerspectiveTransform;
			}

			void DrawImages() {
				using namespace D2D1;
				using DirectXHelpers::D2D1DrawTextNormal;
				using SystemErrorHelpers::ThrowIfFailed;
				constexpr FLOAT HeartScale = 0.8f;
				Microsoft::WRL::ComPtr<ID2D1Image> d2dImageOldTarget;
				m_D2dDeviceContext->GetTarget(&d2dImageOldTarget);
				m_D2dDeviceContext->SetTarget(m_D2dBitmap1Background.Get());
				m_D2dDeviceContext->BeginDraw();
				DrawBackground(m_D2dDeviceContext.Get());
				ThrowIfFailed(D2D1DrawTextNormal(m_D2dDeviceContext.Get(), L"Segoe UI", 24 * m_Scale, DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"Copyright © Programmer-Yang_Xun@outlook.com", ColorF(ColorF::White), &RectF(0, m_D2dSize.height * (0.5f + HeartScale / 2), m_D2dSize.width, m_D2dSize.height)));
				ThrowIfFailed(m_D2dDeviceContext->EndDraw());
				m_D2dDeviceContext->SetTarget(m_D2dBitmap1Foreground.Get());
				m_D2dDeviceContext->BeginDraw();
				m_D2dDeviceContext->Clear(ColorF(ColorF::White, 0));
				DrawHeart(m_D2dDeviceContext.Get(), HeartScale);
				ThrowIfFailed(D2D1DrawTextNormal(m_D2dDeviceContext.Get(), L"Comic Sans MS", 46 * m_Scale, DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"I LOVE YOU\nFOREVER", ColorF(ColorF::White)));
				ThrowIfFailed(m_D2dDeviceContext->EndDraw());
				m_D2dDeviceContext->SetTarget(d2dImageOldTarget.Get());
				m_MaxForegroundGlowRadius = m_Scale * HeartScale * 50;
				SetForegroundGlowRadiusScale(m_ForegroundGlowRadiusScale);
				SetForegroundRotationY(m_ForegroundRotationY);
			}

			void DrawFramesPerSecond() {
				using namespace D2D1;
				const FLOAT fontSize = 22 * m_Scale;
				SystemErrorHelpers::ThrowIfFailed(DirectXHelpers::D2D1DrawTextNormal(m_D2dDeviceContext.Get(), L"Segoe UI", fontSize, DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER, (L"FPS: " + std::to_wstring(!IsRunning() || m_AnimationSet.IsEmpty() ? 0 : m_StepTimer.GetFramesPerSecond())).c_str(), ColorF(ColorF::White), &RectF(0, 0, m_D2dSize.width, fontSize)));
			}

			void Update() {
				if (m_StepTimer.GetFrameCount() > 1) {
					if (m_AnimationSet.Contains(AnimationSet::AnimationType::Glow)) {
						SetForegroundGlowRadiusScale(m_ForegroundGlowRadiusScale + (m_IsGlowFadeIn ? 1 : -1) * CalculateTransitionSpeed(1, GlowAnimationDuration, FPS));
						if (m_ForegroundGlowRadiusScale >= 1)
							m_IsGlowFadeIn = false;
						else if (m_ForegroundGlowRadiusScale <= 0)
							m_IsGlowFadeIn = true;
					}
					if (m_AnimationSet.Contains(AnimationSet::AnimationType::Rotation))
						SetForegroundRotationY(m_ForegroundRotationY + (m_IsRotationClockwise ? -1 : 1) * CalculateTransitionSpeed(360, RotationAnimationDuration, FPS));
				}
			}

			void Render() {
				using namespace SystemErrorHelpers;
				if (!m_IsInitialized)
					throw_system_error((int)D2DERR_NOT_INITIALIZED);
				m_D2dDeviceContext->BeginDraw();
				m_D2dDeviceContext->DrawImage(m_D2dBitmap1Background.Get());
				m_D2dDeviceContext->DrawImage(m_D2dEffect.Get());
				if (m_IsFramesPerSecondVisible)
					DrawFramesPerSecond();
				ThrowIfFailed(m_D2dDeviceContext->EndDraw());
			}

			static void DrawBackground(ID2D1RenderTarget* pD2dRenderTarget) {
				using namespace D2D1;
				const D2D1_GRADIENT_STOP d2dGradientStops[]{
					GradientStop(0, ColorF(0x157050)),
					GradientStop(1, ColorF(0x155799))
				};
				const D2D1_SIZE_F d2dSize = pD2dRenderTarget->GetSize();
				Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush> d2dLinearGradientBrush;
				SystemErrorHelpers::ThrowIfFailed(DirectXHelpers::D2D1CreateLinearGradientBrush(pD2dRenderTarget, d2dGradientStops, ARRAYSIZE(d2dGradientStops), LinearGradientBrushProperties(Point2F(), Point2F(d2dSize.width, d2dSize.height)), &d2dLinearGradientBrush));
				pD2dRenderTarget->FillRectangle(RectF(0, 0, d2dSize.width, d2dSize.height), d2dLinearGradientBrush.Get());
			}

			static void DrawHeart(ID2D1RenderTarget* pD2dRenderTarget, FLOAT scale) {
				using namespace D2D1;
				using Microsoft::WRL::ComPtr;
				using SystemErrorHelpers::ThrowIfFailed;
				const D2D1_GRADIENT_STOP d2dGradientStops[]{
					GradientStop(0, ColorF(0xf02864)),
					GradientStop(1, ColorF(0x282864))
				};
				const D2D1_SIZE_F size = pD2dRenderTarget->GetSize();
				ComPtr<ID2D1Factory> d2dFactory;
				ComPtr<ID2D1PathGeometry> d2dPathGeometry;
				ComPtr<ID2D1GeometrySink> d2dGeometrySink;
				ComPtr<ID2D1LinearGradientBrush> d2dLinearGradientBrush;
				pD2dRenderTarget->GetFactory(&d2dFactory);
				ThrowIfFailed(d2dFactory->CreatePathGeometry(&d2dPathGeometry));
				ThrowIfFailed(d2dPathGeometry->Open(&d2dGeometrySink));
				ThrowIfFailed(DirectXHelpers::D2D1CreateLinearGradientBrush(pD2dRenderTarget, d2dGradientStops, ARRAYSIZE(d2dGradientStops), LinearGradientBrushProperties(Point2F(size.width / 2), Point2F(size.width / 2, size.height)), &d2dLinearGradientBrush));
				/*
				 * Math Functions:
				 * f(x) = sqrt(1 - (|x| - 1) ^ 2)
				 * g(x) = arccos(1 - |x|) - Pi
				 * x: [-2, 2]
				 */
				constexpr FLOAT Pi = 3.14159265f, Radius = 1, Width = Radius * 4, Height = Radius + Pi, Ratio = Width / Height;
				scale *= ((size.width / size.height < Ratio ? size.width : size.height * Ratio) / Width);
				const FLOAT diameter = floorf(Radius * scale * 2);
				const D2D1_POINT_2F origin = Point2F(size.width / 2, size.height / 2 - scale * (Height / 2 - Radius)),
					startPoint = Point2F(origin.x - diameter, origin.y + (-acosf(1 - 1 / scale * diameter) + Pi) * scale);
				d2dGeometrySink->BeginFigure(startPoint, D2D1_FIGURE_BEGIN::D2D1_FIGURE_BEGIN_FILLED);
				for (FLOAT x = -diameter + 1; x <= diameter; x++)
					d2dGeometrySink->AddLine(Point2F(origin.x + x, origin.y + (-acosf(1 - 1 / scale * fabsf(x)) + Pi) * scale));
				d2dGeometrySink->AddArc(ArcSegment(origin, SizeF(diameter / 2, diameter / 2), 0, D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE::D2D1_ARC_SIZE_SMALL));
				d2dGeometrySink->AddArc(ArcSegment(startPoint, SizeF(diameter / 2, diameter / 2), 0, D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE::D2D1_ARC_SIZE_SMALL));
				d2dGeometrySink->EndFigure(D2D1_FIGURE_END::D2D1_FIGURE_END_CLOSED);
				ThrowIfFailed(d2dGeometrySink->Close());
				pD2dRenderTarget->FillGeometry(d2dPathGeometry.Get(), d2dLinearGradientBrush.Get());
			}
		};
	}
}