#include <iostream>

#include "GPSU/Exception.hpp"

namespace GPSU
{
    GPSU_API Exception::Exception(const std::wstring& message_) : message(message_) { std::wcout << message << std::endl; }
	GPSU_API Exception::Exception(const Exception& e) : message(e.message) {}
	GPSU_API Exception::Exception(const Exception&& e) : message(e.message) {}
	GPSU_API Exception::~Exception() {}
}
