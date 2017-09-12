#pragma once

#include <vector>

#include "GPSU/SharedLib.hpp"
#include "GPSU/Version.hpp"

namespace GPSU
{
	class Plugin
	{
	public:
		class Info;

		inline Plugin(Plugin& other)
			: _lib(other._lib.Unlock())
			, _id(other._id)
		{}

		inline const Plugin& operator=(Plugin& other)
		{
			_lib = other._lib.Unlock();
			_id = other._id;
			return *this;
		}

		GPSU_API Plugin(const std::wstring& name);
		inline bool IsValid() const { return _lib.IsValid(); }
		inline const std::vector<Info>& Available() const { return _available; }
		inline const Info& Details() const { return _available[_id]; }
		GPSU_API void Load(const std::wstring& root, const Version& minVersion = Version(0, 0, 0));

	private:
		std::wstring _name;
		SharedLib _lib;
		int _id;
		std::vector<Info> _available;
	
	public:
		class Info
		{
		public:
			inline Info() {}
			
			inline Info(const GPSU::Version& version, const std::wstring& path)
				: _version(version)
				, _path(path)
			{}

			inline GPSU::Version& Version() { return _version; }
			inline const GPSU::Version& Version() const { return _version; }
			inline std::wstring& Path() { return _path; }
			inline const std::wstring& Path() const { return _path; }

		private:
			GPSU::Version _version;
			std::wstring _path;
		};
	};
}
