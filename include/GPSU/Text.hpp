#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "GPSU/Ansii.hpp"

namespace GPSU
{
    namespace Details
    {
        class TextArgContainer;
        struct TextStateModification;
        inline TextStateModification Push(const Ansii::CodeList& code);
        inline TextStateModification Pop(const Ansii::CodeList& code);
    }
    
    template<class TChar>
    class TextBase
    {
    public:
        template<class... TArgs>
        inline TextBase(const TArgs&... args);
        inline void ToStream(std::basic_ostream<TChar>& os) const;
        
    private:
        std::vector<std::shared_ptr<Details::TextArgContainer>> _args;
        inline static Ansii::CodeList Revert(const Ansii::Code& code, const std::unordered_map<unsigned int, std::reference_wrapper<const Ansii::Code>>& state);
    };
    
    using Text = TextBase<char>;
    using WText = TextBase<wchar_t>;
    
    // Only CSI SGR codes supported.
    template<class... TArgs>
    inline Text AnsiiText(const Ansii::CodeList& code, TArgs... args) { return Text(Details::Push(code), args..., Details::Pop(code)); }
    
    // Only CSI SGR codes supported.
    template<class... TArgs>
    inline WText WAnsiiText(const Ansii::CodeList& code, TArgs... args) { return WText(Details::Push(code), args..., Details::Pop(code)); }
}

namespace std
{
    inline wostream& operator<< (std::wostream& os, const GPSU::WText& text)
    {
        text.ToStream(os);
        return os;
    }
    
    inline ostream& operator<< (std::ostream& os, const GPSU::Text& text)
    {
        text.ToStream(os);
        return os;
    }
}

#define Trace "[" \
    << GPSU::AnsiiText(GPSU::Ansii::Foreground(GPSU::Ansii::Colors::Magenta), __FILE__) << ":" \
    << GPSU::AnsiiText(GPSU::Ansii::Foreground(GPSU::Ansii::Colors::Magenta), __LINE__) << "]"

#define WTrace "[" \
    << GPSU::WAnsiiText(GPSU::Ansii::Foreground(GPSU::Ansii::Colors::Magenta), __FILE__) << L":" \
    << GPSU::WAnsiiText(GPSU::Ansii::Foreground(GPSU::Ansii::Colors::Magenta), __LINE__) << L"]"

#include "GPSU/Text.tpp"
