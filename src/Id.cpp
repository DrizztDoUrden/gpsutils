#include "GPSU/Id.hpp"

namespace GPSU
{
	size_t& Id::Last()
	{
		static size_t v = 0;
		return v;
	}

	std::unordered_map<size_t, std::wstring>& Id::ValueToText()
	{
		static std::unordered_map<size_t, std::wstring> v;
		return v;
	}
	
	std::unordered_map<std::wstring, size_t>& Id::TextToValue()
	{
		static std::unordered_map<std::wstring, size_t> v;
		return v;
	}

	size_t Id::GetId(const std::wstring& text)
	{
		const auto existing = TextToValue().find(text);

		if (existing != TextToValue().end())
			return existing->second;

		const auto value = ++Last();

		ValueToText()[value] = text;
		TextToValue()[text] = value;

		return value;
	}
}
