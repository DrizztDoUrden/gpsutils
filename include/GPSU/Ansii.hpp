#pragma once

#include <functional>
#include <numeric>
#include <string>
#include <vector>

#include <GPSU/String.hpp>

namespace GPSU
{
	namespace Ansii
	{
		enum class Codes : unsigned int
		{
			// Single Shift Two
			SS2 = 'N',
			// Single Shift Three
			SS3 = 'O',
			// Device Control String
			DSC = 'P',
			// String Terminator
			ST = '\\',
			// Operating System Command
			OSC = ']',
			// Start of String
			SOS = 'X',
			// Privacy Message
			PM = '^',
			// Application Program Command
			APC = '_',
			// Reset to Initial State
			RIS = 'c',
			// Control Sequence Introducer 
			CSI = '[',
		};

		enum class CSICodes : unsigned int
		{
			None = 0,
			// Cursor Up
			CUU = 'A',
			// Cursor Down
			CUD = 'B',
			// Cursor Forward
			CUF = 'C',
			// Cursor Back
			CUB = 'D',
			// Cursor Next Line
			CNL = 'E',
			// Cursor Previous Line
			CPL = 'F',
			// Cursor Horizontal Absolute
			CHA = 'G',
			// Cursor Position
			CUP = 'H',
			// Erase in Display
			ED = 'J',
			// Erase in Line
			EL = 'K',
			// Scroll Up
			SU = 'S',
			// Scroll Down
			SD = 'T',
			// Horizontal and Vertical Position
			HVP = 'f',
			// Save Cursor Position
			SCP = 's',
			// Restore Cursor Position
			RCP = 'u',
			// Select Graphic Rendition
			SGR = 'm',
			// Device Status Report
			DSR = 'n',
			// AUX Port On/Off
			AUX = 'i',
		};

		enum class SGRCodes : unsigned int
		{
			Reset = 0,
			ItalicOrInverse = 3,
			UnderlineSingle = 4,
			Inverse = 7,
		};

		enum class Colors : unsigned int
		{
			Black = 0,
			Red = 1,
			Green = 2,
			Yellow = 3,
			Blue = 4,
			Magenta = 5,
			Cyan = 6,
			White = 7,
			ExtendedSet = 8,
			Default = 9,
		};

		enum class BoldValues : unsigned int
		{
			Bold = 0,
			Faint = 1,
			Off = 21,
		};

		enum class Fonts : unsigned int
		{
			Default = 0,
			Alternative0 = 1,
			Alternative1 = 2,
			Alternative2 = 3,
			Alternative3 = 4,
			Alternative4 = 5,
			Alternative5 = 6,
			Alternative6 = 7,
			Alternative7 = 8,
			Alternative8 = 9,
		};

		enum class Underlines : unsigned int
		{
			Single = 0,
			None = 20,
		};

		enum class Modes : unsigned int
		{
			None = 0,
			Boldness = 1,
			Underline = 4,
			Fonts = 10,
			Foreground = 30,
			Background = 40,
		};

		namespace Details
		{
			bool _ansiiEnabled = true;
			template<Modes mode>
			class ModeValueTypeContainer;
			template<Modes mode>
			using ModeValueType = typename ModeValueTypeContainer<mode>::type;
		}

		inline void Disable() { Details::_ansiiEnabled = false; }

		// Source of implemented codes: https://en.wikipedia.org/wiki/ANSI_escape_code
		class Code
		{
		public:
			inline unsigned int SGRCode() const { return std::wcstol(_args[0].c_str(), nullptr, 10); }
			inline Codes AnsiiCode() const { return _code; }
			inline CSICodes CSICode() const { return _value; }
			template<class... TArgs>
			inline Code(Codes code, CSICodes value = CSICodes::None, TArgs... args) : _code(code), _value(value), _args({ std::to_wstring(args)... }) {}
			inline const std::vector<std::wstring>& Arguments() const { return _args; }
			inline void ToStream(std::wostream& os) const;
			inline void ToStream(std::ostream& os) const;
			inline Modes Mode() const;
			inline unsigned int Id() const;
			inline bool Consumes(const Code&) const;

		private:
			Codes _code;
			CSICodes _value;
			std::vector<std::wstring> _args;
		};

		// All codes other than CIS SGR should not be stacked.
		class CodeList
		{
		public:
			template<class... TArgs>
			inline CodeList(const TArgs&... args) : _content({ args... }) {}
			inline const CodeList& operator+=(const Code& code) { _content.push_back(code); return *this; }
			inline const CodeList& operator+=(const CodeList& other) { _content.insert(_content.end(), other.Content().begin(), other.Content().end()); return *this; }
			inline const std::vector<Code>& Content() const { return _content; }
			inline const Code& operator[](size_t index) const { return _content[index]; }
			inline void Remove(int index) { _content.erase(_content.begin() + index); }
			inline void ToStream(std::wostream& os) const;
			inline void ToStream(std::ostream& os) const;
			inline void Simplify();

		private:
			std::vector<Code> _content;
		};

		template<class... TArgs>
		inline Code CSICode(TArgs... args) { return Code(Codes::CSI, args...); }

		template<Modes mode, class... TArgs>
		inline Code ModeCode(Details::ModeValueType<mode> value, TArgs... args)
		{
			return Code(Codes::CSI, CSICodes::SGR, static_cast<unsigned int>(mode) + static_cast<unsigned int>(value), args...);
		}

		template<Modes mode>
		inline Code Color256Code(unsigned char color)
		{
			return ModeCode<mode>(Colors::ExtendedSet, 5, static_cast<int>(color));
		}

		template<Modes mode>
		inline Code RGBColorCode(unsigned char r, unsigned char g, unsigned char b)
		{
			return ModeCode<mode>(Colors::ExtendedSet, 2, static_cast<int>(r), static_cast<int>(g), static_cast<int>(b));
		}
	}
}

#include "GPSU/Ansii.tpp"

namespace GPSU
{
	namespace Ansii
	{
		inline Code Unmoded(SGRCodes codes) { return ModeCode<Modes::None>(codes); }
		inline Code Boldness(BoldValues bold) { return ModeCode<Modes::Boldness>(bold); }
		inline Code Underline(Underlines ul) { return ModeCode<Modes::Underline>(ul); }
		inline Code Font(Fonts font) { return ModeCode<Modes::Fonts>(font); }
		inline Code Foreground(Colors color) { return ModeCode<Modes::Foreground>(color); }
		inline Code Background(Colors color) { return ModeCode<Modes::Background>(color); }
		inline Code Foreground(unsigned char color) { return Color256Code<Modes::Foreground>(color); }
		inline Code Background(unsigned char color) { return Color256Code<Modes::Background>(color); }
		inline Code Foreground(unsigned char r, unsigned char g, unsigned char b) { return RGBColorCode<Modes::Foreground>(r, g, b); }
		inline Code Background(unsigned char r, unsigned char g, unsigned char b) { return RGBColorCode<Modes::Background>(r, g, b); }

		const Code Reset = Unmoded(SGRCodes::Reset);

		const Code Bold = Boldness(BoldValues::Bold);
		const Code Faint = Boldness(BoldValues::Faint);
		const Code BoldOff = Boldness(BoldValues::Off);

		const Code UnderlineSingle = Underline(Underlines::Single);
		const Code UnderlineOff = Underline(Underlines::None);
	}
}

namespace std
{
	inline wostream& operator<< (std::wostream& os, const GPSU::Ansii::Code& code)
	{
		code.ToStream(os);
		return os;
	}
	
	inline ostream& operator<< (std::ostream& os, const GPSU::Ansii::Code& code)
	{
		code.ToStream(os);
		return os;
	}
	
	inline wostream& operator<< (std::wostream& os, const GPSU::Ansii::CodeList& codes)
	{
		codes.ToStream(os);
		return os;
	}
	
	inline ostream& operator<< (std::ostream& os, const GPSU::Ansii::CodeList& codes)
	{
		codes.ToStream(os);
		return os;
	}
	
	inline std::wstring to_wstring(GPSU::Ansii::SGRCodes code)
	{
		return to_wstring(static_cast<unsigned int>(code));
	}
	
	inline std::string to_string(GPSU::Ansii::SGRCodes code)
	{
		return to_string(static_cast<unsigned int>(code));
	}
}
