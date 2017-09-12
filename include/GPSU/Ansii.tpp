namespace GPSU
{
    namespace Ansii
    {
        namespace Details
        {
            const wchar_t _wesc = 27;
            const wchar_t _wsep = L';';
            const char _esc = 27;
            const char _sep = ';';
            
            template<Modes mode> class ModeValueTypeContainer {};
            
            template<> class ModeValueTypeContainer<Modes::None> { public: using type = SGRCodes; };
            template<> class ModeValueTypeContainer<Modes::Boldness> { public: using type = BoldValues; };
            template<> class ModeValueTypeContainer<Modes::Underline> { public: using type = Underlines; };
            template<> class ModeValueTypeContainer<Modes::Fonts> { public: using type = Fonts; };
            template<> class ModeValueTypeContainer<Modes::Foreground> { public: using type = Colors; };
            template<> class ModeValueTypeContainer<Modes::Background> { public: using type = Colors; };
        }

        inline void Code::ToStream(std::wostream& os) const
        {
            if (Details::_ansiiEnabled)
            {
                os << Details::_wesc << static_cast<wchar_t>(_code);
                
                if (_code == Codes::CSI)
                {
                    const auto argsJoiner = [](const std::wstring &ss, const std::wstring &s) { return ss.empty() ? s : ss + Details::_wsep + s; };
                    const auto args = std::accumulate(_args.begin(), _args.end(), std::wstring(), argsJoiner);
                    os << args << static_cast<wchar_t>(_value);
                }
            }
        }

        inline void Code::ToStream(std::ostream& os) const
        {
            if (Details::_ansiiEnabled)
            {
                os << Details::_esc << static_cast<char>(_code);
                
                if (_code == Codes::CSI)
                {
                    const auto argsJoiner = [](const std::wstring &ss, const std::wstring &s) { return ss.empty() ? s : ss + Details::_wsep + s; };
                    const auto args = std::to_string(std::accumulate(_args.begin(), _args.end(), std::wstring(), argsJoiner));
                    os << args << static_cast<char>(_value);
                }
            }
        }
        
        inline Modes Code::Mode() const
        {
            static const std::unordered_map<unsigned int, Modes> modeIds
            {
                { static_cast<unsigned int>(Ansii::Modes::Boldness)  + static_cast<unsigned int>(Ansii::BoldValues::Bold),      Ansii::Modes::Boldness, },
                { static_cast<unsigned int>(Ansii::Modes::Boldness)  + static_cast<unsigned int>(Ansii::BoldValues::Faint),     Ansii::Modes::Boldness, },
                { static_cast<unsigned int>(Ansii::Modes::Boldness)  + static_cast<unsigned int>(Ansii::BoldValues::Off),       Ansii::Modes::Boldness, },

                { static_cast<unsigned int>(Ansii::Modes::Underline) + static_cast<unsigned int>(Ansii::Underlines::Single),    Ansii::Modes::Underline, },
                { static_cast<unsigned int>(Ansii::Modes::Underline) + static_cast<unsigned int>(Ansii::Underlines::None),      Ansii::Modes::Underline, },

                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Default),        Ansii::Modes::Fonts, },
                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Alternative0),   Ansii::Modes::Fonts, },
                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Alternative1),   Ansii::Modes::Fonts, },
                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Alternative2),   Ansii::Modes::Fonts, },
                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Alternative3),   Ansii::Modes::Fonts, },
                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Alternative4),   Ansii::Modes::Fonts, },
                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Alternative5),   Ansii::Modes::Fonts, },
                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Alternative6),   Ansii::Modes::Fonts, },
                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Alternative7),   Ansii::Modes::Fonts, },
                { static_cast<unsigned int>(Ansii::Modes::Fonts)     + static_cast<unsigned int>(Ansii::Fonts::Alternative8),   Ansii::Modes::Fonts, },

                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::Default),      Ansii::Modes::Foreground, },
                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::ExtendedSet),  Ansii::Modes::Foreground, },
                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::Black),        Ansii::Modes::Foreground, },
                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::Red),          Ansii::Modes::Foreground, },
                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::Green),        Ansii::Modes::Foreground, },
                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::Yellow),       Ansii::Modes::Foreground, },
                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::Blue),         Ansii::Modes::Foreground, },
                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::Magenta),      Ansii::Modes::Foreground, },
                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::Cyan),         Ansii::Modes::Foreground, },
                { static_cast<unsigned int>(Ansii::Modes::Foreground) + static_cast<unsigned int>(Ansii::Colors::White),        Ansii::Modes::Foreground, },

                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::Default),      Ansii::Modes::Background, },
                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::ExtendedSet),  Ansii::Modes::Background, },
                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::Black),        Ansii::Modes::Background, },
                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::Red),          Ansii::Modes::Background, },
                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::Green),        Ansii::Modes::Background, },
                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::Yellow),       Ansii::Modes::Background, },
                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::Blue),         Ansii::Modes::Background, },
                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::Magenta),      Ansii::Modes::Background, },
                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::Cyan),         Ansii::Modes::Background, },
                { static_cast<unsigned int>(Ansii::Modes::Background) + static_cast<unsigned int>(Ansii::Colors::White),        Ansii::Modes::Background, },
            };

            const auto sgrCode = SGRCode();
            const auto mode = modeIds.find(sgrCode);

            if (mode == modeIds.end())
                return Modes::None;
                
            return mode->second;
        }

        inline unsigned int Code::Id() const
        {
            auto mode = Mode();
            
            if (mode == Modes::None)
                return SGRCode();

            return static_cast<unsigned int>(mode);
        }
        
        inline bool Code::Consumes(const Code& other) const
        {
            return SGRCode() == static_cast<unsigned int>(SGRCodes::Reset) || Id() == other.Id();
        }
        
        inline void CodeList::ToStream(std::wostream& os) const
        {
            if (Details::_ansiiEnabled)
            {
                if (Content().size() == 0)
                    return;
                
                os << Details::_wesc << static_cast<wchar_t>(Content()[0].AnsiiCode());
                
                const auto codesJoiner = [](const std::wstring &ss, const Code& code)
                {
                    const auto argsJoiner = [](const std::wstring &ss, const std::wstring &s) { return ss.empty() ? s : ss + Details::_wsep + s; };
                    const auto args = std::accumulate(code.Arguments().begin(), code.Arguments().end(), std::wstring(), argsJoiner);
                    return ss.empty() ? args : ss + Details::_wsep + args;
                };
                
                const auto codes = std::accumulate(Content().begin(), Content().end(), std::wstring(), codesJoiner);
                os << codes << static_cast<wchar_t>(Content()[0].CSICode());
            }
        }

        inline void CodeList::ToStream(std::ostream& os) const
        {
            if (Details::_ansiiEnabled)
            {
                if (Content().size() == 0)
                    return;
                
                os << Details::_esc << static_cast<char>(Content()[0].AnsiiCode());
                
                const auto codesJoiner = [](const std::wstring &ss, const Code& code)
                {
                    const auto argsJoiner = [](const std::wstring &ss, const std::wstring &s) { return ss.empty() ? s : ss + Details::_wsep + s; };
                    const auto args = std::accumulate(code.Arguments().begin(), code.Arguments().end(), std::wstring(), argsJoiner);
                    return ss.empty() ? args : ss + Details::_wsep + args;
                };
                
                const auto codes = std::accumulate(Content().begin(), Content().end(), std::wstring(), codesJoiner);
                
                os << std::to_string(codes) << static_cast<char>(Content()[0].CSICode());
            }
        }
        
        inline void CodeList::Simplify()
        {
            for (auto i = 1u; i < Content().size(); i++)
            {
                const auto& code = Content()[i];
                
                for (auto j = 0u; j < i; j++)
                    if (code.Consumes(Content()[j]))
                    {
                        Remove(j);
                        j--;
                        i--;
                    }
            }
        }
    }
}
