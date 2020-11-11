//
// StepTimer.h - A simple timer that provides elapsed time information
//

#pragma once

#include <Windows.h>
#include "../SystemErrorHelpers/SystemErrorHelpers.h"

namespace DX {
	// Helper class for animation and simulation timing.
	class StepTimer final {
	public:
		StepTimer() noexcept(false) {
			using namespace Hydr10n::SystemErrorHelpers;
			ThrowIfFailed(QueryPerformanceFrequency(&m_QpcFrequency));
			ThrowIfFailed(QueryPerformanceCounter(&m_QpcLastTime));

			// Initialize max delta to 1/10 of a second.
			m_QpcMaxDelta = static_cast<DWORD64>(m_QpcFrequency.QuadPart / 10);
		}

		// Get elapsed time since the previous Update call.
		DWORD64 GetElapsedTicks() const noexcept { return m_ElapsedTicks; }
		double GetElapsedSeconds() const noexcept { return TicksToSeconds(m_ElapsedTicks); }

		// Get total time since the start of the program.
		DWORD64 GetTotalTicks() const noexcept { return m_TotalTicks; }
		double GetTotalSeconds() const noexcept { return TicksToSeconds(m_TotalTicks); }

		// Get total number of updates since start of the program.
		DWORD GetFrameCount() const noexcept { return m_FrameCount; }

		// Get the current framerate.
		DWORD GetFramesPerSecond() const noexcept { return m_FramesPerSecond; }

		// Set whether to use fixed or variable timestep mode.
		void SetFixedTimeStep(bool isFixedTimestep) noexcept { m_IsFixedTimeStep = isFixedTimestep; }

		// Set how often to call Update when in fixed timestep mode.
		void SetTargetElapsedTicks(DWORD64 targetElapsed) noexcept { m_TargetElapsedTicks = targetElapsed; }
		void SetTargetElapsedSeconds(double targetElapsed) noexcept { m_TargetElapsedTicks = SecondsToTicks(targetElapsed); }

		// Integer format represents time using 10,000,000 ticks per second.
		static const DWORD64 TicksPerSecond = 10000000;

		static constexpr double TicksToSeconds(DWORD64 ticks) noexcept { return static_cast<double>(ticks) / TicksPerSecond; }
		static constexpr DWORD64 SecondsToTicks(double seconds) noexcept { return static_cast<DWORD64>(seconds * TicksPerSecond); }

		// After an intentional timing discontinuity (for instance a blocking IO operation)
		// call this to avoid having the fixed timestep logic attempt a set of catch-up
		// Update calls.
		void ResetElapsedTime() {
			Hydr10n::SystemErrorHelpers::ThrowIfFailed(QueryPerformanceCounter(&m_QpcLastTime));

			m_LeftOverTicks = 0;
			m_FramesPerSecond = 0;
			m_FramesThisSecond = 0;
			m_QpcSecondCounter = 0;
		}

		// Update timer state, calling the specified Update function the appropriate number of times.
		template<typename TUpdate>
		void Tick(const TUpdate& update) {
			// Query the current time.
			LARGE_INTEGER currentTime;

			Hydr10n::SystemErrorHelpers::ThrowIfFailed(QueryPerformanceCounter(&currentTime));

			DWORD64 timeDelta = static_cast<DWORD64>(currentTime.QuadPart - m_QpcLastTime.QuadPart);

			m_QpcLastTime = currentTime;
			m_QpcSecondCounter += timeDelta;

			// Clamp excessively large time deltas (e.g. after paused in the debugger).
			if (timeDelta > m_QpcMaxDelta)
				timeDelta = m_QpcMaxDelta;

			// Convert QPC units into a canonical tick format. This cannot overflow due to the previous clamp.
			timeDelta *= TicksPerSecond;
			timeDelta /= static_cast<DWORD64>(m_QpcFrequency.QuadPart);

			const DWORD lastFrameCount = m_FrameCount;

			if (m_IsFixedTimeStep) {
				// Fixed timestep update logic

				// If the app is running very close to the target elapsed time (within 1/4 of a millisecond) just clamp
				// the clock to exactly match the target value. This prevents tiny and irrelevant errors
				// from accumulating over time. Without this clamping, a game that requested a 60 fps
				// fixed update, running with vsync enabled on a 59.94 NTSC display, would eventually
				// accumulate enough tiny errors that it would drop a frame. It is better to just round
				// small deviations down to zero to leave things running smoothly.

				if (static_cast<DWORD64>(abs(static_cast<INT64>(timeDelta - m_TargetElapsedTicks))) < TicksPerSecond / 4000)
					timeDelta = m_TargetElapsedTicks;

				m_LeftOverTicks += timeDelta;

				while (m_LeftOverTicks >= m_TargetElapsedTicks) {
					m_ElapsedTicks = m_TargetElapsedTicks;
					m_TotalTicks += m_TargetElapsedTicks;
					m_LeftOverTicks -= m_TargetElapsedTicks;
					m_FrameCount++;

					update();
				}
			}
			else {
				// Variable timestep update logic.
				m_ElapsedTicks = timeDelta;
				m_TotalTicks += timeDelta;
				m_LeftOverTicks = 0;
				m_FrameCount++;

				update();
			}

			// Track the current framerate.
			if (m_FrameCount != lastFrameCount)
				m_FramesThisSecond++;

			if (m_QpcSecondCounter >= static_cast<DWORD64>(m_QpcFrequency.QuadPart)) {
				m_FramesPerSecond = m_FramesThisSecond;
				m_FramesThisSecond = 0;
				m_QpcSecondCounter %= static_cast<DWORD64>(m_QpcFrequency.QuadPart);
			}
		}

	private:
		// Source timing data uses QPC units.
		LARGE_INTEGER m_QpcFrequency{}, m_QpcLastTime{};
		DWORD64 m_QpcMaxDelta{};

		// Derived timing data uses a canonical tick format.
		DWORD64 m_ElapsedTicks{}, m_TotalTicks{}, m_LeftOverTicks{};

		// Members for tracking the framerate.
		DWORD m_FrameCount{}, m_FramesPerSecond{}, m_FramesThisSecond{};
		DWORD64 m_QpcSecondCounter{};

		// Members for configuring fixed timestep mode.
		bool m_IsFixedTimeStep{};
		DWORD64 m_TargetElapsedTicks{ TicksPerSecond / 60 };
	};
}