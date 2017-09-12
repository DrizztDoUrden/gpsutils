#pragma once

#include <string>
#include <unordered_map>

#include "GPSU/InternalCommon.hpp"

namespace GPSU
{
	class Id
	{
	public:
		inline Id() : _value(0) {}
		inline Id(size_t value) : _value(value) {}
		inline Id(const std::wstring& text) : _value(GetId(text)) {}
		inline Id(const wchar_t* text) : _value(GetId(text)) {}
		inline operator const std::wstring&() const { return ValueToText()[_value]; }
		inline const std::wstring& Text() const { return ValueToText()[_value]; }
		inline size_t Value() const { return _value; }
		inline bool IsValid() const { return _value != 0; }
		inline bool operator==(const Id& other) const { return _value == other._value; }
		inline bool operator!=(const Id& other) const { return _value != other._value; }
		inline static Id Invalid() { return Id(); }

	private:
		size_t _value;

		static GPSU_API size_t& Last();
		static GPSU_API std::unordered_map<size_t, std::wstring>& ValueToText();
		static GPSU_API std::unordered_map<std::wstring, size_t>& TextToValue();
		static GPSU_API size_t GetId(const std::wstring& text);
	};
}

namespace std
{
	template<>
	struct hash<GPSU::Id>
	{
		inline size_t operator()(const GPSU::Id& id) const { return id.Value(); }
	};
}
