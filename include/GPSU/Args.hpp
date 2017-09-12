#pragma once
#include <exception>
#include <unordered_map>
#include <string>
#include <vector>

#include "Ansii.hpp"

class Arg
{
public:
    template<class... TArgs>
    inline Arg(bool flag, const std::string& name, TArgs... aliases) : Arg(flag, name, {name, aliases...}) {}
    inline Arg(bool flag, const std::string& name, const std::vector<std::string>& aliases) : _flag(flag), _name(name), _aliases(aliases) {}
    inline const std::vector<std::string>& Aliases() const { return _aliases; }
    inline const std::string& Name() const { return _name; }
    inline bool Flag() const { return _flag; }

private:
    bool _flag;
    std::string _name;
    std::vector<std::string> _aliases;
};

class DuplicatedAliasException : public std::exception
{
public:
    inline DuplicatedAliasException(const std::string& name) : _msg(std::string("Duplicate alias: ") + name) {}
    inline virtual const char* what() const noexcept override { return _msg.c_str(); }

private:
    std::string _msg;
};

class ArgsParser
{
public:
    using ResultType = std::unordered_map<std::string, std::vector<std::string>>;

    inline ArgsParser(const std::vector<Arg>& args) : _args(args) { BuildAliases(); }
    ArgsParser(ArgsParser&) = delete;
    ArgsParser& operator=(ArgsParser&) = delete;

    inline void Parse(int argsn, char** cargs, ResultType& result)
    {
        const Arg* lastArg = nullptr;
        result.clear();

        for (auto i = 1; i < argsn; i++)
        {
            const std::string arg(cargs[i]);

            if (arg[0] == '-')
            {
                const auto key = arg.substr(1);
                const auto found = _aliases.find(key);

                if (found == _aliases.end())
                {
                    std::cerr << "[" << Ansii::Colorize(Colors::Yellow, "Warning") << "] Key skipped: <" << key << "> because it is unknown" << std::endl;
                    continue;
                }

                lastArg = found->second;
                result[lastArg->Name()];
                continue;
            }

            if (lastArg == nullptr)
            {
                std::cerr << "[" << Ansii::Colorize(Colors::Yellow, "Warning") << "] Value skipped: <" << arg << ">" << std::endl;
                continue;
            }

            if (lastArg->Flag())
            {
                std::cerr << "[" << Ansii::Colorize(Colors::Yellow, "Warning") << "] Value skipped: <" << arg << "> because last key is a flag" << std::endl;
                continue;
            }

            result[lastArg->Name()].push_back(arg);
        }
    }

private:
    std::vector<Arg> _args;
    std::unordered_map<std::string, const Arg*> _aliases;

    inline void BuildAliases()
    {
        for (const auto& arg : _args)
            for (const auto& alias : arg.Aliases())
            {
                if (_aliases.find(alias) != _aliases.end())
                    throw DuplicatedAliasException(alias);
                _aliases[alias] = &arg;
            }
    }
};
