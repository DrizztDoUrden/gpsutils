#include <cstddef>
#include <cstring>
#include <experimental/string_view>
#include <string>

namespace GPSU
{
    template <class T>
    constexpr std::experimental::string_view TypeName()
    {
        constexpr const char* fname = "TypeName()";
        std::experimental::string_view p =
    #ifdef __clang__
        __PRETTY_FUNCTION__;
    #elif defined(__GNUC__)
        __PRETTY_FUNCTION__;
    #elif defined(_MSC_VER)
        __FUNCSIG__;
    #endif
        p.remove_prefix(p.find('=') + 2);
        p.remove_suffix(p.size() - p.find(';'));
        return p;
    }
    
    template <class T>
    constexpr std::string TypeNameString()
    {
        return std::string(TypeName<T>().data(), TypeName<T>().size());
    }
}
