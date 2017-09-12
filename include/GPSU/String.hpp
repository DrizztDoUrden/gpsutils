#pragma once

#include <codecvt>
#include <locale>
#include <string>

namespace std
{
    inline std::wstring to_wstring(const std::string& str)
    {
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }
    
    inline std::wstring to_wstring(const char* str)
    {
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }
    
    inline std::wstring to_wstring(const wchar_t* str)
    {
        return str;
    }
    
    inline std::string to_string(const std::wstring& str)
    {
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(str);
    }
}
