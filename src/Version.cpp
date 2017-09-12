#include "GPSU/Version.hpp"

namespace GPSU
{
	GPSU_API bool Version::Parse(const std::wstring& str)
	{
		size_t ldx;
		const auto major = std::stoul(str, &ldx);

		if (ldx < 0 || str[ldx++] != L'.')
			return false;

		auto strcp = str.substr(ldx);
		const auto minor = std::stoul(strcp, &ldx);

		if (ldx < 0 || strcp[ldx++] != L'.')
			return false;

		strcp = strcp.substr(ldx);
		const auto patch = std::stoul(strcp, &ldx);

		if (ldx < 0 || ldx != strcp.size() && strcp[ldx++] != L'-')
			return false;

		if (ldx != strcp.size())
			_label = strcp.substr(ldx);

		_major = major;
		_minor = minor;
		_patch = patch;
		return true;
	}
}
