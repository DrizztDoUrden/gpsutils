#pragma once

#ifdef WINDOWS
	#include "Windows.h"
#endif

#include "GPSU/InternalCommon.hpp"

namespace GPSU
{
#ifdef WINDOWS
	template<class TException>
	class CheckSystemError
	{
	public:
		GPSU_API CheckSystemError()
		{
			const auto error = GetLastError();

			if (error == 0)
				return;

			LPTSTR message;

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&message,
				0, NULL);

			throw TException(message);
		}
	};
#endif
}
