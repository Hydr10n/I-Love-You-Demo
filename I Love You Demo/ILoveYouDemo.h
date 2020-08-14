#pragma once

#include "pch.h"
#include "MessageTimer.h"
#include <d2d1effects.h>
#include <unordered_set>

#define CalcFrameDuration(iFPS) (1000.f / (int)(iFPS))
#define CalcTransitionVelocity(iValue, iMilliseconds, iFPS) ((int)(iValue) / ((int)(iMilliseconds) / 1000.f * (int)(iFPS)))

namespace Hydr10n {
	namespace Demos {
		using namespace DirectXHelpers;
		using Timers::MessageTimer;
		using Microsoft::WRL::ComPtr;

		class ILoveYouDemo final {
		public:
			class AnimationSet final {
			public:
				enum class AnimationType { Glow, Rotation };

				AnimationSet(const std::initializer_list<AnimationType>& animationTypes) : m_AnimationTypes(animationTypes) {}

				void Add(AnimationType animationType) { m_AnimationTypes.insert(animationType); }

				void Remove(AnimationType animationType) { m_AnimationTypes.erase(animationType); }

				BOOL Contains(AnimationType animationType) const { return m_AnimationTypes.find(animationType) != m_AnimationTypes.end(); }

				BOOL IsEmpty() const { return m_AnimationTypes.empty(); }

			private:
				std::unordered_set<AnimationType> m_AnimationTypes;
			};

			HRESULT Initialize(HWND hWnd) {
				if (m_Initialized)
					return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
				return CatchHRESULT([&] {
					ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, (IUnknown**)&m_D2dFactory));
					ThrowIfFailed(CreateGraphicsResources(hWnd));
					ThrowIfFailed(CreateGraphicsEffects());
					ThrowIfFailed(DrawImages());
					ThrowIfFailed(m_MessageTimer.Start(FrameDuration) ? S_OK : HRESULT_FROM_WIN32(GetLastError()));
					m_Initialized = TRUE;
					});
			}

			HRESULT Tick(BOOL bUpdate = TRUE) {
				return CatchHRESULT([&] {
					if (!bUpdate)
						ThrowIfFailed(Render(TRUE));
					else if (IsRunning()) {
						if (m_MessageTimer.Wait())
							ThrowIfFailed(Update());
						ThrowIfFailed(Render());
					}
					});
			}

			BOOL Pause() { return m_MessageTimer.Pause(); }

			BOOL Resume() { return m_MessageTimer.Resume(); }

			BOOL IsRunning() const { return m_MessageTimer.GetState() == MessageTimer::State::Running; }

			BOOL ReverseAnimationState(AnimationSet::AnimationType animationType) {
				if (m_AnimationSet.Contains(animationType)) {
					m_AnimationSet.Remove(animationType);
					if (m_AnimationSet.IsEmpty())
						return Pause();
					return TRUE;
				}
				else {
					m_AnimationSet.Add(animationType);
					return Resume();
				}
			}

			const AnimationSet& GetAnimationSet() const { return m_AnimationSet; }

			HRESULT SetForegroundGlowRadiusScale(BOOL bRender, FLOAT scale = 0) {
				return CatchHRESULT([&] {
					scale = min(1, max(0, scale));
					if (m_Initialized)
						ThrowIfFailed(m_D2dEffectShadow->SetValue(D2D1_SHADOW_PROP::D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, m_MaxForegroundGlowRadius * scale));
					if (bRender)
						ThrowIfFailed(Render(TRUE));
					m_ForegroundGlowRadiusScale = scale;
					});
			}

			FLOAT GetForegroundGlowRadiusScale() const { return m_ForegroundGlowRadiusScale; }

			HRESULT SetForegroundRotationY(BOOL bRender, FLOAT y = 0) {
				return CatchHRESULT([&] {
					if (m_Initialized)
						ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION, D2D1::Vector3F(0, y)));
					if (bRender)
						ThrowIfFailed(Render(TRUE));
					m_ForegroundRotationY = y;
					});
			}

			FLOAT GetForegroundRotationY() const { return m_ForegroundRotationY; }

			void ReverseRotation() { m_IsRotationClockwise = !m_IsRotationClockwise; }

			BOOL IsRotationClockwise() const { return m_IsRotationClockwise; }

		private:
			static const int FPS = 60, FrameDuration = (int)CalcFrameDuration(FPS),
				GlowAnimationDuration = 2500, RotationAnimationDuration = GlowAnimationDuration;

			BOOL m_Initialized = FALSE, m_IsGlowFadeIn = TRUE, m_IsRotationClockwise = TRUE;
			FLOAT m_MaxForegroundGlowRadius = 0, m_ForegroundGlowRadiusScale = 0, m_ForegroundRotationY = 0;
			ComPtr<ID2D1Factory> m_D2dFactory;
			ComPtr<ID2D1DeviceContext> m_D2dDeviceContext;
			ComPtr<ID2D1BitmapRenderTarget> m_D2dBitmapRenderTargetBackground, m_D2dBitmapRenderTargetForeground;
			ComPtr<ID2D1Bitmap> m_D2dBitmapBackground, m_D2dBitmapForeground;
			ComPtr<ID2D1Effect> m_D2dEffect, m_D2dEffectShadow, m_D2dEffect3DPerspectiveTransform;
			MessageTimer m_MessageTimer;
			AnimationSet m_AnimationSet{ AnimationSet::AnimationType::Glow, AnimationSet::AnimationType::Rotation };

			HRESULT CreateGraphicsResources(HWND hWnd) {
				return CatchHRESULT([&] {
					ComPtr<ID2D1HwndRenderTarget> m_D2dHwndRenderTarget;
					ThrowIfFailed(CreateHwndRenderTarget(m_D2dFactory.Get(), hWnd, &m_D2dHwndRenderTarget));
					ThrowIfFailed(GetDeviceContext(m_D2dHwndRenderTarget.Get(), &m_D2dDeviceContext));
					ThrowIfFailed(m_D2dHwndRenderTarget->CreateCompatibleRenderTarget(&m_D2dBitmapRenderTargetBackground));
					ThrowIfFailed(m_D2dHwndRenderTarget->CreateCompatibleRenderTarget(&m_D2dBitmapRenderTargetForeground));
					ThrowIfFailed(m_D2dBitmapRenderTargetBackground->GetBitmap(&m_D2dBitmapBackground));
					ThrowIfFailed(m_D2dBitmapRenderTargetForeground->GetBitmap(&m_D2dBitmapForeground));
					});
			}

			HRESULT CreateGraphicsEffects() {
				return CatchHRESULT([&] {
					const D2D1_SIZE_F d2dSize = m_D2dDeviceContext->GetSize();
					ComPtr<ID2D1Effect> d2dCompositeEffect;
					ThrowIfFailed(m_D2dDeviceContext->CreateEffect(CLSID_D2D1Shadow, &m_D2dEffectShadow));
					ThrowIfFailed(m_D2dDeviceContext->CreateEffect(CLSID_D2D1Composite, &d2dCompositeEffect));
					ThrowIfFailed(m_D2dDeviceContext->CreateEffect(CLSID_D2D13DPerspectiveTransform, &m_D2dEffect3DPerspectiveTransform));
					ThrowIfFailed(m_D2dEffectShadow->SetValue(D2D1_SHADOW_PROP::D2D1_SHADOW_PROP_COLOR, D2D1::Vector4F(1, 1, 1, 0.5f)));
					ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_ROTATION_ORIGIN, D2D1::Vector3F(d2dSize.width / 2)));
					ThrowIfFailed(m_D2dEffect3DPerspectiveTransform->SetValue(D2D1_3DPERSPECTIVETRANSFORM_PROP::D2D1_3DPERSPECTIVETRANSFORM_PROP_PERSPECTIVE_ORIGIN, D2D1::Vector2F(d2dSize.width / 2, d2dSize.height / 2)));
					m_D2dEffectShadow->SetInput(0, m_D2dBitmapForeground.Get());
					d2dCompositeEffect->SetInputEffect(0, m_D2dEffectShadow.Get());
					d2dCompositeEffect->SetInput(1, m_D2dBitmapForeground.Get());
					m_D2dEffect3DPerspectiveTransform->SetInputEffect(0, d2dCompositeEffect.Get());
					m_D2dEffect = m_D2dEffect3DPerspectiveTransform;
					});
			}

			HRESULT DrawImages() {
				return CatchHRESULT([&] {
					const D2D1_SIZE_F d2dSize = m_D2dDeviceContext->GetSize();
					const FLOAT scale = d2dSize.height / 800, heartScale = 0.8f;
					m_D2dBitmapRenderTargetBackground->BeginDraw();
					ThrowIfFailed(DrawBackground(m_D2dBitmapRenderTargetBackground.Get()));
					ThrowIfFailed(DrawTextNormal(m_D2dBitmapRenderTargetBackground.Get(), L"Segoe UI", 24 * scale, DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"Copyright \xa9 Programmer-Yang_Xun@outlook.com", D2D1::ColorF(0xf0f0f0), &D2D1::RectF(0, d2dSize.height * 0.9f, d2dSize.width, d2dSize.height)));
					ThrowIfFailed(m_D2dBitmapRenderTargetBackground->EndDraw());
					m_D2dBitmapRenderTargetForeground->BeginDraw();
					ThrowIfFailed(DrawHeart(m_D2dBitmapRenderTargetForeground.Get(), heartScale));
					ThrowIfFailed(DrawTextNormal(m_D2dBitmapRenderTargetForeground.Get(), L"Comic Sans MS", 40 * scale, DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER, L"I LOVE YOU\nFOREVER", D2D1::ColorF(0xf0f0f0)));
					ThrowIfFailed(m_D2dBitmapRenderTargetForeground->EndDraw());
					m_MaxForegroundGlowRadius = 50 * scale * heartScale;
					ThrowIfFailed(SetForegroundGlowRadiusScale(FALSE));
					ThrowIfFailed(SetForegroundRotationY(FALSE));
					});
			}

			HRESULT Update() {
				return CatchHRESULT([&] {
					if (m_MessageTimer.GetWaitCount() > 1) {
						if (m_AnimationSet.Contains(AnimationSet::AnimationType::Glow)) {
							ThrowIfFailed(SetForegroundGlowRadiusScale(FALSE, m_ForegroundGlowRadiusScale + (m_IsGlowFadeIn ? 1 : -1) * 2 * CalcTransitionVelocity(1, GlowAnimationDuration, FPS)));
							if (m_ForegroundGlowRadiusScale >= 1)
								m_IsGlowFadeIn = FALSE;
							else if (m_ForegroundGlowRadiusScale <= 0)
								m_IsGlowFadeIn = TRUE;
						}
						if (m_AnimationSet.Contains(AnimationSet::AnimationType::Rotation))
							ThrowIfFailed(SetForegroundRotationY(FALSE, m_ForegroundRotationY + (m_IsRotationClockwise ? -1 : 1) * CalcTransitionVelocity(360, RotationAnimationDuration, FPS)));
					}
					});
			}

			HRESULT Render(BOOL bForce = FALSE) {
				if (!m_Initialized)
					return D2DERR_NOT_INITIALIZED;
				if (!bForce && !m_MessageTimer.GetWaitCount())
					return E_FAIL;
				m_D2dDeviceContext->BeginDraw();
				m_D2dDeviceContext->DrawImage(m_D2dBitmapBackground.Get());
				m_D2dDeviceContext->DrawImage(m_D2dEffect.Get());
				return m_D2dDeviceContext->EndDraw();
			}

			static HRESULT DrawBackground(ID2D1RenderTarget* pD2dRenderTarget) {
				return CatchHRESULT([&] {
					const D2D1_GRADIENT_STOP d2dGradientStops[]{
						D2D1::GradientStop(0, D2D1::ColorF(0x157050)),
						D2D1::GradientStop(1, D2D1::ColorF(0x155799))
					};
					const D2D1_SIZE_F d2dSize = pD2dRenderTarget->GetSize();
					ComPtr<ID2D1LinearGradientBrush> d2dLinearGradientBrush;
					ThrowIfFailed(CreateLinearGradientBrush(pD2dRenderTarget, d2dGradientStops, ARRAYSIZE(d2dGradientStops), D2D1::LinearGradientBrushProperties(D2D1::Point2F(), D2D1::Point2F(d2dSize.width, d2dSize.height)), &d2dLinearGradientBrush));
					pD2dRenderTarget->FillRectangle(D2D1::RectF(0, 0, d2dSize.width, d2dSize.height), d2dLinearGradientBrush.Get());
					});
			}

			static HRESULT DrawHeart(ID2D1RenderTarget* pD2dRenderTarget, FLOAT scale) {
				return CatchHRESULT([&] {
					const D2D1_GRADIENT_STOP d2dGradientStops[]{
						D2D1::GradientStop(0, D2D1::ColorF(0xf02864)),
						D2D1::GradientStop(1, D2D1::ColorF(0x282864))
					};
					const D2D1_SIZE_F size = pD2dRenderTarget->GetSize();
					ComPtr<ID2D1Factory> d2dFactory;
					ComPtr<ID2D1LinearGradientBrush> d2dLinearGradientBrush;
					ComPtr<ID2D1PathGeometry> d2dPathGeometry;
					ComPtr<ID2D1GeometrySink> d2dGeometrySink;
					pD2dRenderTarget->GetFactory(&d2dFactory);
					ThrowIfFailed(CreateLinearGradientBrush(pD2dRenderTarget, d2dGradientStops, ARRAYSIZE(d2dGradientStops), D2D1::LinearGradientBrushProperties(D2D1::Point2F(size.width / 2), D2D1::Point2F(size.width / 2, size.height)), &d2dLinearGradientBrush));
					ThrowIfFailed(d2dFactory->CreatePathGeometry(&d2dPathGeometry));
					ThrowIfFailed(d2dPathGeometry->Open(&d2dGeometrySink));
					/*
					 * Math Functions:
					 * f(x) = sqrt(1 - (|x| - 1) ^ 2)
					 * g(x) = arccos(1 - |x|) - Pi
					 * x: [-2, 2]
					 */
					static const FLOAT Pi = 3.14159265f, Radius = 1, Width = Radius * 4, Height = Radius + Pi, Ratio = Width / Height;
					scale *= ((size.width / size.height < Ratio ? size.width : size.height * Ratio) / Width);
					const FLOAT diameter = floorf(Radius * scale * 2);
					const D2D1_POINT_2F origin = D2D1::Point2F(size.width / 2, size.height / 2 - scale * (Height / 2 - Radius)),
						startPoint = D2D1::Point2F(origin.x - diameter, origin.y + (-acosf(1 - 1 / scale * diameter) + Pi) * scale);
					d2dGeometrySink->BeginFigure(startPoint, D2D1_FIGURE_BEGIN::D2D1_FIGURE_BEGIN_FILLED);
					for (FLOAT x = -diameter + 1; x <= diameter; x++)
						d2dGeometrySink->AddLine(D2D1::Point2F(origin.x + x, origin.y + (-acosf(1 - 1 / scale * fabsf(x)) + Pi) * scale));
					d2dGeometrySink->AddArc(D2D1::ArcSegment(origin, D2D1::SizeF(diameter / 2, diameter / 2), 0, D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE::D2D1_ARC_SIZE_SMALL));
					d2dGeometrySink->AddArc(D2D1::ArcSegment(startPoint, D2D1::SizeF(diameter / 2, diameter / 2), 0, D2D1_SWEEP_DIRECTION::D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE::D2D1_ARC_SIZE_SMALL));
					d2dGeometrySink->EndFigure(D2D1_FIGURE_END::D2D1_FIGURE_END_CLOSED);
					ThrowIfFailed(d2dGeometrySink->Close());
					pD2dRenderTarget->FillGeometry(d2dPathGeometry.Get(), d2dLinearGradientBrush.Get());
					});
			}
		};
	}
}