#include "GPSU/SharedLib.hpp"
#include "GPSU/SystemError.hpp"
#include <locale>
#include <codecvt>
#include <string>

#ifdef WINDOWS
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

namespace GPSU
{
	GPSU_API LibraryLoadException::LibraryLoadException(const std::wstring& msg) : Exception(msg) {}
	GPSU_API LibraryLoadException::LibraryLoadException(const LibraryLoadException& e) : Exception(e) {}
	GPSU_API LibraryLoadException::LibraryLoadException(const LibraryLoadException&& e) : Exception(e) {}
	GPSU_API LibraryLoadException::~LibraryLoadException() {}
	
	GPSU_API SharedLib::SharedLib(const std::wstring& path)
#ifdef WINDOWS
		: _handle(LoadLibrary(path.c_str()))
#else
		: _handle(dlopen(converter.to_bytes(path).c_str(), RTLD_NOW | RTLD_GLOBAL))
#endif
	{
		if (_handle == nullptr)
		{
#ifdef WINDOWS
			CheckSystemError<LibraryLoadException>();
#else
			throw LibraryLoadException(converter.from_bytes(dlerror()) + L"\n");
#endif
		}
	}

#pragma warning(disable: 4290)
	GPSU_API SharedLib::~SharedLib() throw(LibraryLoadException)
#pragma warning(default: 4290)
	{
		if (_handle == nullptr)
			return;

#ifdef WINDOWS
		if (!FreeLibrary((HMODULE)_handle)) // TODO: check FreeLibrary return values
			CheckSystemError<LibraryLoadException>();
#else
		if (dlclose(_handle))
			throw LibraryLoadException(converter.from_bytes(dlerror()));
#endif
	}
}
