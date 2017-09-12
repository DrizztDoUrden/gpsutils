#ifdef WINDOWS
#	include <conio.h>
#   include <Windows.h>
#else
#	include <lx-conio.h>
#   include <unistd.h>
#endif

#include <iostream>
#include <sstream>

#include "GPSU/Text.hpp"

#ifndef WINDOWS
	inline void Sleep(int time) { sleep((time + 999) / 1000); }
#endif

using namespace GPSU;
using namespace GPSU::Ansii;

class TextTest
{
public:
	void Run() const
	{
		std::wcout << L"[" << WAnsiiText(Ansii::Foreground(Colors::Green), L"Started") << L"] Test" << std::endl;
		std::wcout << CSICode(CSICodes::DSR, 6);
		
		std::string str;
		char c;
		
		do
		{
			str += (c = _getch());
		} while (c != 'R');
		
		char* ptr;
		auto rows = strtol(str.c_str() + 2, &ptr, 0);
		auto cols = strtol(ptr + 1, &ptr, 0);
		
		std::wcout << L"Cursor: (" << cols << L";" << rows << ")" << std::endl;
		
		std::wstring start = L"This line should be removed in ";
		std::wstring timerStr = start + L"X seconds.";
		std::wcout << timerStr << std::endl;
		
		for (auto i = 3; i > 0; i--)
		{
			std::wcout << CSICode(CSICodes::CPL) << CSICode(CSICodes::CHA, start.size() + 1) << i << CSICode(CSICodes::CNL);
			std::wcout.flush();
			Sleep(1000);
		}
		
		std::wcout << CSICode(CSICodes::CPL) << CSICode(CSICodes::EL);
		std::wcout.flush();
	}
};

int main()
{
	TextTest().Run();
	return 0;
}