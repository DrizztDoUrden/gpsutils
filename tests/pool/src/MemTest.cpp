#include <iostream>
#include <sstream>

#include "GPSU/Memory.hpp"
#include "GPSU/Text.hpp"

using namespace GPSU;
using namespace GPSU::Ansii;

class PoolTest : public PoolNew {};
class AutoPoolTest : public PoolNewDelete {};

class MemTest
{
public:
    void Run() const
    {
		const auto arraySize = 16;
		StackPool<128> spool;
		auto s0 = spool.Alloc(arraySize);
		std::unique_ptr<PoolTest> spt(new(spool) PoolTest);
		
		Prealloc<> ppool(1);
		std::wcout << PoolSizeTest(ppool) << std::endl;
		auto p0 = ppool.Alloc(arraySize);
		std::unique_ptr<PoolTest> ppt(new(ppool) PoolTest);
		
		Dynamic<StackPool<128>> dspool;
		auto ds0 = dspool.Alloc(arraySize);
		std::unique_ptr<AutoPoolTest> daspt(new(dspool) AutoPoolTest);
		std::unique_ptr<PoolTest> dspt(new(dspool) PoolTest);
		
		Dynamic<Prealloc<ResizePolicy::NotAllowed>> dppool(256);
		auto dptest = static_cast<int*>(dppool.Alloc(sizeof(int)));
		const int testValue = 10;
		*dptest = testValue;
		
		auto secondTest = [&](std::wostream& s)
		{
			auto t2 = LogPtr(new(dppool) AutoPoolTest, L"+", s, L"");
			s << " -> " << FormatBytes(dppool.Used()) << L"/" << FormatBytes(dppool.Limit()) << L" -> ";
		};
		
		std::wcout << PoolSizeTest(dppool) << std::endl;
		std::wcout << PoolSizeTest(dppool, secondTest) << std::endl;
		
		auto dp0 = dppool.Alloc(arraySize);
		std::unique_ptr<AutoPoolTest> dappt(new(dppool) AutoPoolTest);
		std::unique_ptr<PoolTest> dppt(new(dppool) PoolTest);
		
		std::wcout << std::endl;
		auto start = dppool.AllocationStart();
		dppool.ResizeBuffer(16ul * 1024ul * 1024ul * 1024ul);
		auto newStart = dppool.AllocationStart();
		auto ohshi = dppool.Alloc(6 * 1024 * 1024);
		
		if (start != newStart)
		{
			std::wcout << L"Buffer start has moved." << std::endl;
			dptest = dptest - static_cast<int*>(start) + static_cast<int*>(newStart);
		}
		
		std::wcout << L"Test: " << *dptest << " == " << testValue << std::endl;
		
		std::wcout << std::endl;
		std::wcout << L"Stack pool:" << PoolStats(spool, s0, spt.get()) << std::endl;
		std::wcout << L"Prealloc pool:" << PoolStats(ppool, p0, ppt.get()) << std::endl;
		std::wcout << L"Dynamic stack pool:" << PoolStats(dspool, ds0, dspt.get()) << std::endl;
		std::wcout << L"Dynamic prealloc pool:" << PoolStats(dppool, dp0, dppt.get()) << std::endl;
		
		std::wcout << std::endl;
		std::wcout << L"Dynamic stack pool ptr/data sizes: " << DynamicPoolStats(dspool, arraySize, ds0, dspt.get(), daspt.get()) << std::endl;
		std::wcout << L"Dynamic prealoc ptr/data sizes: "    << DynamicPoolStats(dppool, arraySize, dp0, dppt.get(), dappt.get()) << std::endl;
    }

private:
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
		FormatPart(ss, bytes, K,  B, L"", ss.tellp() == 0);
		
		ss << L"B";
		
		return ss.str();
	}

	template<class TTarget, class TPostfix = std::wostream&(*)(std::wostream&)>
	inline TTarget* LogPtr(TTarget* ptr, const wchar_t* prefix = L"+", std::wostream& s = std::wcout, TPostfix postfix = &std::endl) const
	{
		s << prefix << ptr << postfix;
		return ptr;
	}

	std::wstring PoolSizeTest(Pool& pool, const std::function<void(std::wostream&)>& beforeDelete = [](std::wostream&){}) const
	{
		std::wostringstream ss;
		
		ss << FormatBytes(pool.Used()) << L"/" << FormatBytes(pool.Limit()) << L" -> ";
		auto t = LogPtr(new(pool) AutoPoolTest, L"+", ss, L"");
		ss << L" -> " << FormatBytes(pool.Used()) << L"/" << FormatBytes(pool.Limit()) << L" -> ";
		beforeDelete(ss);
		delete LogPtr(t, L"-", ss, L"");
		ss << L" -> " << FormatBytes(pool.Used()) << L"/" << FormatBytes(pool.Limit());
		
		return ss.str();
	}

	std::wstring PoolStats(Pool& pool, void* ptr0, void* ptr1) const
	{
		std::wostringstream ss;
		
		ss << L"                                                                                  "
			<< CSICode(CSICodes::CHA, 25) << ptr0
			<< CSICode(CSICodes::CHA, 45) << ptr1
			<< CSICode(CSICodes::CHA, 65) << L"delta: " << (reinterpret_cast<char*>(ptr1) - reinterpret_cast<char*>(ptr0))
			<< CSICode(CSICodes::CHA, 80) << L"stats: " << FormatBytes(pool.Used()) << L"/" << FormatBytes(pool.Limit());
		
		return ss.str();
	}

	template<class TPool>
	inline std::wstring DynamicPtrSize(Dynamic<TPool>& pool, void* ptr, size_t dataSize) const
	{
		auto ptrSize = pool.PtrSize(ptr);
		return FormatBytes(ptrSize) + L"/" + FormatBytes(dataSize) + L" (d: " + FormatBytes(ptrSize - dataSize) + L")";
	}

	template<class TPool>
	inline std::wstring DynamicPoolStats(Dynamic<TPool>& pool, size_t arraySize, void* ptr0, void* pt, void* apt) const
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