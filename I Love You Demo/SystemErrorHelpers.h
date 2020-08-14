#pragma once

#include <Windows.h>

namespace Hydr10n {
	namespace SystemErrorHelpers {
		template <class T>
		HRESULT CatchHRESULT(const T& lambdaExpression) {
			try { lambdaExpression(); }
			catch (HRESULT hr) { return hr; }
			return S_OK;
		}

		inline void WINAPI ThrowIfFailed(HRESULT hr) {
			if (FAILED(hr))
				throw hr;
		}

		inline DWORD WINAPI WIN32_FROM_HRESULT(HRESULT hr) {
			if ((hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
				return HRESULT_CODE(hr);
			if (hr == S_OK)
				return ERROR_SUCCESS;
			return ERROR_CAN_NOT_COMPLETE;
		}

		class SystemErrorMessage final {
		private:
			LPWSTR m_lpwSystemErrorMessage = NULL;

		public:
			static const DWORD DefaultLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

			SystemErrorMessage(DWORD dwErrorCode, DWORD dwLanguageId = DefaultLanguageId) {
				FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, dwLanguageId, (LPWSTR)&m_lpwSystemErrorMessage, 0, NULL);
			}

			SystemErrorMessage(HRESULT hr, DWORD dwLanguageId = DefaultLanguageId) : SystemErrorMessage(WIN32_FROM_HRESULT(hr)) {}

			~SystemErrorMessage() {
				LocalFree(m_lpwSystemErrorMessage);
				m_lpwSystemErrorMessage = NULL;
			}

			operator LPCWSTR() const { return m_lpwSystemErrorMessage; }
		};
	}
}