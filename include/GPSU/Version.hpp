#pragma once

#include <string>

#include "GPSU/InternalCommon.hpp"

namespace GPSU
{
	class Version
	{
	public:
		inline Version(
			unsigned long major = 1,
			unsigned long minor = 0,
			unsigned long patch = 0,
			const std::wstring& label = L"")
			: _major(major)
			, _minor(minor)
			, _patch(patch)
			, _label(label)
		{

		}

		GPSU_API bool Parse(const std::wstring& str);

		inline unsigned long Major() const { return _major; }
		inline unsigned long Minor() const { return _minor; }
		inline unsigned long Patch() const { return _patch; }
		inline const std::wstring& Label() const { return _label; }
		inline operator std::wstring() const { return ToString(); }
		inline bool operator !=(const Version& ot) const { return !(*this == ot); }
		inline bool operator >=(const Version& ot) const { return *this == ot || *this > ot; }
		inline bool operator <=(const Version& ot) const { return ot >= *this; }
		inline bool operator <(const Version& ot) const { return ot > *this; }

		inline bool operator ==(const Version& ot) const
		{
			return _major == ot._major && _minor == ot._minor && _patch == ot._patch;
		}

		inline bool operator >(const Version& ot) const
		{
			return (_major > ot._major)
				|| (_major == ot._major && _minor > ot._minor)
				|| (_major == ot._major && _minor == ot._minor && _patch > ot._patch);
		}

		inline std::wstring ToString() const
		{
			const auto labelStr = _label.empty()
				? L""
				: L"-" + _label;

			return
				std::to_wstring(_major) + L"." +
				std::to_wstring(_minor) + L"." +
				std::to_wstring(_patch) +
				labelStr;
		}

	private:
		unsigned long _major;
		unsigned long _minor;
		unsigned long _patch;
		std::wstring _label;
	};
}
