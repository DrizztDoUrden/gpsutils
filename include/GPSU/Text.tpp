#pragma once

#include <stack>

namespace GPSU
{
    namespace Details
    {
        struct TextStateModification
        {
            enum class Actions
            {
                Push,
                Pop,
            };
            
            Actions action;
            Ansii::CodeList codes;
            bool isFake;
            
            inline bool IsPush() const { return action == Actions::Push; }
            inline bool IsPop() const { return action == Actions::Pop; }
        };
        
        inline TextStateModification Push(const Ansii::CodeList& code) { return TextStateModification { TextStateModification::Actions::Push, code, false, }; }
        inline TextStateModification Pop(const Ansii::CodeList& code) { return TextStateModification { TextStateModification::Actions::Pop, code, false, }; }
        
        class TextArgContainer
        {
        public:
            inline virtual void ToStream(std::wostream& os) const { std::abort(); }
            inline virtual void ToStream(std::ostream& os) const { std::abort(); }
            inline virtual void ToText(const TextBase<char>*& text) const { text = nullptr; }
            inline virtual void ToText(const TextBase<wchar_t>*& text) const { text = nullptr; }
            inline virtual TextStateModification* ToModification() { return nullptr; }
            inline virtual const TextStateModification* ToModification() const { return nullptr; }
        };
        
        template<class TArg>
        class TextTypedArgContainer : public TextArgContainer
        {
        public:
            inline TextTypedArgContainer(const TArg& data) : _data(data) {}
            inline virtual void ToStream(std::wostream& os) const override { os << _data; }
            inline virtual void ToStream(std::ostream& os) const override { os << _data; }
            
        protected:
            TArg _data;
        };
        
        template<>
        class TextTypedArgContainer<Text> : public TextArgContainer
        {
        public:
            inline virtual void ToStream(std::wostream& os) const { std::abort(); }
            inline TextTypedArgContainer(const Text& data) : _data(data) {}
            virtual void ToText(const TextBase<char>*& text) const { text = &_data; }
            virtual void ToText(const TextBase<wchar_t>*& text) const { text = nullptr; }
            inline virtual void ToStream(std::ostream& os) const override { os << _data; }
            
        protected:
            Text _data;
        };
        
        template<>
        class TextTypedArgContainer<WText> : public TextArgContainer
        {
        public:
            inline virtual void ToStream(std::ostream& os) const { std::abort(); }
            inline TextTypedArgContainer(const WText& data) : _data(data) {}
            virtual void ToText(const TextBase<char>*& text) const { text = nullptr; }
            virtual void ToText(const TextBase<wchar_t>*& text) const { text = &_data; }
            inline virtual void ToStream(std::wostream& os) const override { os << _data; }
            
        protected:
            WText _data;
        };
        
        template<>
        class TextTypedArgContainer<std::wstring> : public TextArgContainer
        {
        public:
            inline TextTypedArgContainer(const std::wstring& data) : _data(data) {}
            inline virtual void ToStream(std::wostream& os) const override { os << _data; }
            inline virtual void ToStream(std::ostream& os) const override { os << std::to_string(_data); }
            
        protected:
            std::wstring _data;
        };
        
        template<>
        class TextTypedArgContainer<std::string> : public TextArgContainer
        {
        public:
            inline TextTypedArgContainer(const std::string& data) : _data(data) {}
            inline virtual void ToStream(std::wostream& os) const override { os << std::to_wstring(_data); }
            inline virtual void ToStream(std::ostream& os) const override { os << _data; }
            
        protected:
            std::string _data;
        };
        
        template<size_t size>
        class TextTypedArgContainer<wchar_t[size]> : public TextArgContainer
        {
        public:
            inline TextTypedArgContainer(const std::wstring& data) : _data(data) {}
            inline virtual void ToStream(std::wostream& os) const override { os << _data; }
            inline virtual void ToStream(std::ostream& os) const override { os << std::to_string(_data); }
            
        protected:
            std::wstring _data;
        };
        
        template<size_t size>
        class TextTypedArgContainer<char[size]> : public TextArgContainer
        {
        public:
            inline TextTypedArgContainer(const std::string& data) : _data(data) {}
            inline virtual void ToStream(std::wostream& os) const override { os << std::to_wstring(_data); }
            inline virtual void ToStream(std::ostream& os) const override { os << _data; }
            
        protected:
            std::string _data;
        };
        
        template<>
        class TextTypedArgContainer<TextStateModification> : public TextArgContainer
        {
        public:
            inline TextTypedArgContainer(const TextStateModification& data) : _data(data) {}
            virtual const TextStateModification* ToModification() const { return &_data; }
            virtual TextStateModification* ToModification() { return &_data; }
            
        protected:
            TextStateModification _data;
        };
        
        inline void AllocateTextArgs(std::vector<std::shared_ptr<Details::TextArgContainer>>& container) {}
        
        template<class... TArgs, class TArg>
        inline void AllocateTextArgs(std::vector<std::shared_ptr<Details::TextArgContainer>>& container, const TArg& arg, const TArgs&... args)
        {
            container.push_back(std::make_shared<Details::TextTypedArgContainer<TArg>>(arg));
            AllocateTextArgs(container, args...);
        }
    }
    
    template<class TChar>
    template<class... TArgs>
    inline TextBase<TChar>::TextBase(const TArgs&... args)
    {
        Details::AllocateTextArgs(_args, args...);
        
        for (auto i = 0; i < _args.size(); i++)
        {
            auto& arg = *_args[i];
            const TextBase* text;
            arg.ToText(text);
            
            if (text == nullptr)
                continue;
            
            _args.insert(_args.begin() + i + 1,
                text->_args.begin(),
                text->_args.end());
            
            _args.erase(_args.begin() + i);
            i--;
        }
        
        for (auto i = 0; i < _args.size(); i++)
        {
            auto& arg = *_args[i];
            auto mod = arg.ToModification();
            
            if (mod == nullptr || mod->codes.Content().size() <= 1)
                continue;
            
            auto j = 1;
            
            for (const auto& code : mod->codes.Content())
                _args.insert(_args.begin() + i + j++,
                    std::make_shared<Details::TextTypedArgContainer<Details::TextStateModification>>(
                        Details::TextStateModification { mod->action, code, false }));
            
            _args.erase(_args.begin() + i);
            i--;
        }
        
        std::vector<Details::TextStateModification*> prevs;
        
        for (auto arg : _args)
        {
            auto mod = arg->ToModification();
            
            if (mod == nullptr || prevs.size() > 0 && mod->action != prevs[prevs.size() - 1]->action)
            {
                prevs.clear();
                continue;
            }
            
            for (auto prev : prevs)
                prev->isFake = true;
            
            if (prevs.size() > 0)
            {
                auto& last = *prevs[prevs.size() - 1];
                
                for (auto j = 0; j < last.codes.Content().size(); j++)
                {
                    auto found = false;
                    
                    for (const auto modCode : mod->codes.Content())
                        if (modCode.Consumes(last.codes[j]))
                        {
                            found = true;
                            break;
                        }
                    
                    if (!found)
                    {
                        mod->codes += last.codes[j];
                        last.codes.Remove(j);
                    }
                }
            }
            
            prevs.push_back(mod);
        }
    }
    
    template<class TChar>
    inline void TextBase<TChar>::ToStream(std::basic_ostream<TChar>& os) const
    {
        std::unordered_map<unsigned int, std::reference_wrapper<const Ansii::Code>> state;
        std::stack<std::unordered_map<unsigned int, std::reference_wrapper<const Ansii::Code>>> states;
        
        for (const auto& arg : _args)
        {
            const auto mod = arg->ToModification();
            
            if (mod == nullptr)
            {
                arg->ToStream(os);
                continue;
            }
            
            if (mod->IsPush())
            {
                if (!mod->isFake)
                    os << mod->codes;
                
                for (const auto& code : mod->codes.Content())
                {
                    states.push(state);
                    
                    const auto id = code.Id();
                    const auto found = state.find(id);
                    
                    if (found != state.end())
                        state.erase(found);
                    
                    state.emplace(id, std::cref(code));
                }
                
                continue;
            }
            
            if (mod->IsPop())
            {
                Ansii::CodeList revert;
                
                for (const auto& code : mod->codes.Content())
                {
                    state = states.top();
                    states.pop();
                    
                    if (!mod->isFake)
                        revert += Revert(code, state);
                }
                
                revert.Simplify();
                os << revert;
                continue;
            }
            
            std::abort();
        }
    }
    
    template<class TChar>
    inline Ansii::CodeList TextBase<TChar>::Revert(const Ansii::Code& code, const std::unordered_map<unsigned int, std::reference_wrapper<const Ansii::Code>>& state)
    {
        const auto stateValue = state.find(code.Id());
        if (stateValue != state.end())
            return stateValue->second.get();
        
        Ansii::CodeList revCode = Ansii::Reset;
        for (const auto& pair : state)
            revCode += pair.second.get();
        return revCode;
    }
}
