#pragma once

#include <Windows.h>

namespace Hydr10n {
	namespace Timers {
		class MessageTimer final {
		public:
			enum class State { NotStarted, Running, Paused, Waiting, Stopped };

			~MessageTimer() { Stop(); }

			State GetState() const { return m_State; }

			BOOL Start(LONG lPeriod) {
				if (m_hTimer) {
					SetLastError(ERROR_ALREADY_INITIALIZED);
					return FALSE;
				}
				if ((m_hTimer = CreateWaitableTimerW(NULL, FALSE, NULL)) == NULL)
					return FALSE;
				m_Period = lPeriod;
				const BOOL ret = SetTimer(FALSE);
				if (ret)
					m_State = State::Running;
				else {
					CloseHandle(m_hTimer);
					m_hTimer = NULL;
				}
				return ret;
			}

			BOOL Pause() {
				const BOOL ret = CancelWaitableTimer(m_hTimer);
				if (ret)
					m_State = State::Paused;
				return ret;
			}

			BOOL Resume() {
				const BOOL ret = SetTimer(TRUE);
				if (ret)
					m_State = State::Running;
				return ret;
			}

			BOOL Wait() {
				const State state = m_State;
				m_State = State::Waiting;
				const BOOL ret = MsgWaitForMultipleObjects(1, &m_hTimer, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0;
				if (ret) {
					m_State = State::Running;
					m_WaitCount++;
				}
				else
					m_State = state;
				return ret;
			}

			BOOL Stop() {
				const BOOL ret = CloseHandle(m_hTimer);
				if (ret) {
					m_State = State::Stopped;
					m_hTimer = NULL;
				}
				return ret;
			}

			DWORD64 GetWaitCount() const { return m_WaitCount; }

		private:
			State m_State = State::NotStarted;
			LONG m_Period = 0;
			DWORD64 m_WaitCount = 0;
			HANDLE m_hTimer = NULL;

			BOOL SetTimer(BOOL bResume) { return SetWaitableTimer(m_hTimer, &LARGE_INTEGER(), m_Period, NULL, NULL, bResume); }
		};
	}
}