#pragma once

#include <string>
#include <assert.h>

#include "GPSU/Exception.hpp"
#include "GPSU/InternalCommon.hpp"

namespace GPSU
{
	class LibraryLoadException : public Exception
	{
	public:
		GPSU_API LibraryLoadException(const std::wstring& msg);
		GPSU_API LibraryLoadException(const LibraryLoadException& e);
		GPSU_API LibraryLoadException(const LibraryLoadException&& e);
		GPSU_API ~LibraryLoadException();
		GPSU_API const LibraryLoadException& operator=(const Exception& e) = delete;
	};

	class SharedLib
	{
	public:
		inline SharedLib() : _handle(nullptr) {}
		inline SharedLib(void* handle) : _handle(handle) {}
		GPSU_API SharedLib(const std::wstring& path);
#pragma warning(disable: 4290)
		GPSU_API ~SharedLib() throw(LibraryLoadException);
#pragma warning(default: 4290)
		SharedLib(SharedLib&) = delete;
		const SharedLib& operator=(SharedLib&) = delete;
		bool IsValid() const { return _handle != nullptr; }

		inline void* Unlock()
		{
			auto handle = _handle;
			_handle = nullptr;
			return handle;
		}

		inline const SharedLib& operator=(void* handle)
		{
			assert(_handle == nullptr && "Resetting locked libraries is not allowed");
			_handle = handle;
			return *this;
		}

	private:
		void* _handle;
	};
}
