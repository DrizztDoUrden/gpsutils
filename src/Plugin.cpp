#include <algorithm>
#include <iostream>

#ifdef VISUAL_STUDIO
	#include <filesystem>
#else
	#include <experimental/filesystem>
#endif

#include "GPSU/Plugin.hpp"

#define fs std::experimental::filesystem

namespace GPSU
{
	GPSU_API Plugin::Plugin(const std::wstring& name)
		: _name(name)
		, _lib(nullptr)
	{}

	static std::wstring LibFilename(const std::wstring& name)
	{
#if WINDOWS
		return name + L".dll";
#else
		return L"lib" + name + L".so";
#endif
	}

	GPSU_API void Plugin::Load(const std::wstring& root, const Version& minVersion)
	{
		assert(!_lib.IsValid() && "Attempt to load a loaded plugin.");

		const auto modPath = root + L"/" + _name;
		const auto pathEnd = L"/" + LibFilename(_name);
		auto found = false;

		for (const auto& child : fs::directory_iterator(modPath))
		{
			if (!fs::is_directory(child))
				continue;

			Version version;
			
			if (!version.Parse(child.path().filename().wstring()))
				continue;

			const auto path = child.path().wstring() + pathEnd;
			
			if (!fs::exists(path))
				continue;

			_available.emplace_back(version, path);
		}

		std::sort(_available.begin(), _available.end(),
			[](const Info& l, const Info& r) { return l.Version() > r.Version(); });

		for (auto id = 0; id < _available.size(); id++)
		{
			const auto& info = _available[id];

			if (info.Version() < minVersion)
				return;

			try
			{
				_lib = SharedLib(info.Path()).Unlock();
				_id = id;
				return;
			}
			catch (const LibraryLoadException&) {}
		}
	}
}
