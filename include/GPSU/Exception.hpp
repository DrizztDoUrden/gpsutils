#pragma once

#include <string>

#include "GPSU/InternalCommon.hpp"

namespace GPSU
{
	class Exception
	{
	public:
		const std::wstring message;
		GPSU_API Exception(const std::wstring& message_);
		GPSU_API Exception(const Exception& e);
		GPSU_API Exception(const Exception&& e);
		GPSU_API ~Exception();
		GPSU_API const Exception& operator=(const Exception& e) = delete;
	};
}
