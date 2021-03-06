#include <iomanip>
#include <iostream>
#include <sstream>

#include "GPSU/Memory.hpp"
#include "GPSU/Text.hpp"

using namespace GPSU;
using namespace GPSU::Ansii;

class PoolTest : public PoolNew {};
class AutoPoolTest : public PoolNewDelete {};

enum class LogPtrMod
{
	Alloc,
	Free,
};

class MemTest
{
public:
	void Run() const
	{
		const auto arraySize = 16;
		StackPool<128> spool;
		const auto s0 = spool.Alloc(arraySize);
		auto spt = new(spool) PoolTest;
		
		Prealloc<> ppool(1);
		std::wcout << PoolSizeTest(ppool) << std::endl;
		const auto p0 = ppool.Alloc(arraySize);
		auto ppt = new(ppool) PoolTest;
		
		Dynamic<StackPool<128>> dspool;
		const auto ds0 = dspool.Alloc(arraySize);
		std::unique_ptr<AutoPoolTest> daspt(new(dspool) AutoPoolTest);
		auto dspt = new(dspool) PoolTest;
		
		Dynamic<Prealloc<ResizePolicy::NotAllowed>> dppool(256);
		auto dptest = static_cast<int*>(dppool.Alloc(sizeof(int)));
		const int testValue = 10;
		*dptest = testValue;
		
		const auto secondTest = [&](std::wostream& s)
		{
			auto t2 = LogPtr(new(dppool) PoolTest, LogPtrMod::Alloc, s, L"");
			s << " -> " << std::setw(8) << FormatBytes(dppool.Used()) << L"/" << std::setw(8) << std::left << FormatBytes(dppool.Limit()) << std::right << L" -> ";
		};
		
		std::wcout << PoolSizeTest(dppool) << std::endl;
		std::wcout << PoolSizeTest(dppool, secondTest) << std::endl;
		
		const auto dp0 = dppool.Alloc(arraySize);
		std::unique_ptr<AutoPoolTest> dappt(new(dppool) AutoPoolTest);
		auto dppt = new(dppool) PoolTest;
		
		std::wcout << std::endl;
		const auto start = dppool.AllocationStart();
		auto buffSize = static_cast<size_t>(1) << 34;

		while (true)
		{
			auto resized = dppool.ResizeBuffer(buffSize);

			if (resized)
				break;

			std::wcout << WAnsiiText(Foreground(Colors::Red), L"Resize has failed: ", FormatBytes(buffSize)) << std::endl;

			if (buffSize <= 1024)
				break;

			buffSize /= 2;
		}

		std::wcout;
		const auto newStart = dppool.AllocationStart();
		const auto ohshi = dppool.Alloc(6 * 1024 * 1024);
		const auto bufferMoved = start != newStart;

		if (bufferMoved)
		{
			std::wcout << L"Buffer start has moved." << std::endl;
			dptest = dptest - static_cast<int*>(start) + static_cast<int*>(newStart);
		}

		std::wcout << L"Test: " << std::setw(16) << start << L" == " << std::left << std::setw(16) << newStart << std::right << L" -> " << Test(!bufferMoved) << std::endl;
		std::wcout << L"Test: " << std::setw(16) << *dptest << L" == " << std::left << std::setw(16) << testValue << std::right << L" -> " << Test(*dptest == testValue) << std::endl;
		
		std::wcout << std::endl;
		std::wcout << L"Stack pool:            " << PoolStats(spool, s0, spt) << std::endl;
		std::wcout << L"Prealloc pool:         " << PoolStats(ppool, p0, ppt) << std::endl;
		std::wcout << L"Dynamic stack pool:    " << PoolStats(dspool, ds0, dspt) << std::endl;
		std::wcout << L"Dynamic prealloc pool: " << PoolStats(dppool, dp0, dppt) << std::endl;
		
		std::wcout << std::endl;
		std::wcout << L"Dynamic stack pool ptr/data sizes: " << DynamicPoolStats(dspool, arraySize, ds0, dspt, daspt.get()) << std::endl;
		std::wcout << L"Dynamic prealoc ptr/data sizes:    " << DynamicPoolStats(dppool, arraySize, dp0, dppt, dappt.get()) << std::endl;
	}

private:
	WText Test(bool passed) const
	{
		return WAnsiiText(Foreground(passed ? Colors::Green : Colors::Red), passed ? L"Passed" : L"Failed");
	}

	inline void FormatPart(std::wostream& s, size_t bytes, size_t K, size_t part, const wchar_t* name, bool forceShow = false) const
	{
		if (!forceShow && bytes % (part * K) < part)
			return;
		
		s << bytes / part % K << name;
	}

	std::wstring FormatBytes(size_t bytes) const
	{
		static const size_t K = 1 << 10;
		static const size_t B = 1;
		static const size_t KB =  B * K;
		static const size_t MB = KB * K;
		static const size_t GB = MB * K;
		static const size_t TB = GB * K;
		
		std::wostringstream ss;
		
		if (bytes > TB)
			ss << bytes / TB << L" TB";
		
		FormatPart(ss, bytes, K, GB, L"G");
		FormatPart(ss, bytes, K, MB, L"M");
		FormatPart(ss, bytes, K, KB, L"K");
		FormatPart(ss, bytes, K,  B, L"", static_cast<size_t>(ss.tellp()) == 0);
		
		ss << L"B";
		
		return ss.str();
	}

	template<class TTarget, class TPrefix = const wchar_t*, class TPostfix = std::wostream&(*)(std::wostream&)>
	inline TTarget* LogPtr(TTarget* ptr, LogPtrMod mod, std::wostream& s = std::wcout, TPostfix postfix = &std::endl, TPrefix prefix = L"") const
	{
		const auto sign = mod == LogPtrMod::Alloc ? L'+' : L'-';
		const auto color = mod == LogPtrMod::Alloc ? Colors::Blue : Colors::Magenta;

		s << prefix << WAnsiiText(Foreground(color), sign, ptr) << postfix;
		return ptr;
	}

	std::wstring PoolSizeTest(Pool& pool, const std::function<void(std::wostream&)>& beforeDelete = [](std::wostream&){}) const
	{
		std::wostringstream ss;
		
		ss << std::setw(8) << FormatBytes(pool.Used()) << L"/" << std::setw(8) << std::left << FormatBytes(pool.Limit()) << std::right << L" -> ";
		auto t = LogPtr(new(pool) AutoPoolTest, LogPtrMod::Alloc, ss, L"");
		ss << L" -> " << std::setw(8) << FormatBytes(pool.Used()) << L"/" << std::setw(8) << std::left << FormatBytes(pool.Limit()) << std::right << L" -> ";
		beforeDelete(ss);
		delete LogPtr(t, LogPtrMod::Free, ss, L"");
		ss << L" -> " << std::setw(8) << FormatBytes(pool.Used()) << L"/" << std::setw(8) << std::left << FormatBytes(pool.Limit()) << std::right;
		
		return ss.str();
	}

	std::wstring PoolStats(Pool& pool, void* ptr0, void* ptr1) const
	{
		std::wostringstream ss;
		
		ss	<<   L"ptr0: " << std::setw(16) << ptr0
			<< L", ptr0: " << std::setw(16) << ptr1
			<< L", delta: " << std::setw(4) << FormatBytes(reinterpret_cast<char*>(ptr1) - reinterpret_cast<char*>(ptr0))
			<< L", stats: " << std::setw(8) << FormatBytes(pool.Used()) << L"/" << std::setw(8) << std::left << FormatBytes(pool.Limit()) << std::right;
		
		return ss.str();
	}

	template<class TPool>
	inline std::wstring DynamicPtrSize(Dynamic<TPool>& pool, void* ptr, size_t dataSize) const
	{
		auto ptrSize = pool.PtrSize(ptr);
		return FormatBytes(ptrSize) + L"/" + FormatBytes(dataSize) + L" (d: " + FormatBytes(ptrSize - dataSize) + L")";
	}

	template<class TPool>
	inline std::wstring DynamicPoolStats(Dynamic<TPool>& pool, size_t arraySize, void* ptr0, PoolTest* pt, AutoPoolTest* apt) const
	{
		return DynamicPtrSize(pool, ptr0, arraySize) + L", "
			 + DynamicPtrSize(pool, pt, sizeof(PoolTest)) + L", "
			 + DynamicPtrSize(pool, apt, sizeof(AutoPoolTest));
	}
};

int main()
{
	MemTest().Run();
	return 0;
}