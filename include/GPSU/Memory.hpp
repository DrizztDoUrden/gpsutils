#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <memory>
#include <set>

namespace GPSU
{
    class Pool
    {
    public:
        virtual bool Resize(size_t) = 0;
        virtual void* Alloc(size_t) = 0;
        template<class TData>
        inline TData* Alloc() { return static_cast<TData*>(Alloc(sizeof(TData))); }
        template<class TData>
        inline TData* Alloc(size_t count) { return static_cast<TData*>(Alloc(count * sizeof(TData))); }
        virtual size_t Limit() const = 0;
        virtual size_t Used() const = 0;
        virtual size_t Left() const = 0;
        virtual void Free(void*) {}
        virtual void* Realloc(void*, size_t) { return nullptr; }
        template<class TData>
        inline TData* Realloc(TData* ptr) { return static_cast<TData*>(Realloc(ptr, sizeof(TData))); }
        template<class TData>
        inline TData* Realloc(TData* ptr, size_t size) { return static_cast<TData*>(Realloc(ptr, size)); }
        virtual void* AllocationStart() = 0;
        
    protected:
        virtual void* NextAllocation() = 0;
        virtual void NextAllocation(void*) = 0;
    };
    
    class PoolNew
    {
    public:
        inline void* operator new(size_t size) = delete;
        inline void* operator new(size_t size, Pool& pool) { return pool.Alloc(size); }
        inline void operator delete(void*) {}
        inline void operator delete(void*, Pool& pool) {}
    };
    
    class PoolNewDelete
    {
    public:
        inline void* operator new(size_t size, Pool& pool);
        inline void operator delete(void* ptr);
        inline void operator delete(void*, Pool& pool);
        
    private:
        Pool* _pool;
    };
    
    struct DefaultBlockSearch
    {
        static inline std::set<void*>::const_iterator Find(const std::set<void*>& blocks, size_t size, const std::function<size_t(void*)>& sizeGetter);
    };
    
    template<class TPool, class TBlockSearch = DefaultBlockSearch>
    class Dynamic : public TPool
    {
    public:
        template<class... TArgs>
        inline Dynamic(TArgs... args) : TPool(args...) {}
        inline virtual void* Alloc(size_t size) override;
        inline virtual size_t Used() const override;
        inline virtual void Free(void*) override;
        inline virtual void* Realloc(void*, size_t) override;
        inline size_t PtrSize(void* ptr) const { return Info(FromOutterPtr(ptr)).size; }
        
    protected:
        std::set<void*> _freed;
        struct MemInfo { size_t size; };
        inline MemInfo Info(void* ptr) const { return *static_cast<MemInfo*>(ptr); }
        inline MemInfo& Info(void* ptr) { return *static_cast<MemInfo*>(ptr); }
        inline size_t& Size(void* ptr) { return Info(ptr).size; }
        inline size_t Size(void* ptr) const { return Info(ptr).size; }
        inline void* ToOutterPtr(void* ptr) const { return ptr != nullptr ? static_cast<MemInfo*>(ptr) + 1 : nullptr; }
        inline void* FromOutterPtr(void* ptr) const { return ptr != nullptr ? static_cast<MemInfo*>(ptr) - 1 : nullptr; }
        inline void* Next(void* ptr) const { return static_cast<char*>(ptr) + Info(ptr).size; }
        inline bool Stitch(void* ptr, size_t delta);
        inline bool StitchBack(void*& ptr, size_t delta);
    };
    
    template<size_t limit>
    class StackPool : public virtual Pool
    {
    public:
        inline virtual bool Resize(size_t) override { return false; }
        inline virtual void* Alloc(size_t size) override;
        inline virtual size_t Limit() const override { return limit; }
        inline virtual size_t Used() const override { return _pos; }
        inline virtual size_t Left() const override { return limit - _pos; }
        inline virtual void* AllocationStart() override { return static_cast<void*>(_data.data()); }
        
    protected:
        inline virtual void* NextAllocation() override { return _data.data() + _pos; }
        inline virtual void NextAllocation(void* ptr) override { _pos = static_cast<size_t>(static_cast<char*>(ptr) - _data.data()); }
        
    private:
        std::array<char, limit> _data;
        size_t _pos = 0;
    };
    
    namespace ResizePolicy
    {
        struct NotAllowed
        {
            inline static bool Allowed() { return false; }
            inline static size_t CalculateNewSize(size_t, size_t) { return 0; }
        };
        
        struct Default
        {
            inline static bool Allowed() { return true; }
            inline static size_t CalculateNewSize(size_t oldSize, size_t allocSize) { return 4 * oldSize + 16 * allocSize; }
        };
    }
    
    template<class resizePolicy = ResizePolicy::Default, decltype(&malloc) alloc = &malloc, decltype(&free) free_ = &free, decltype(&realloc) realloc_ = &realloc>
    class Prealloc : public virtual Pool
    {
    public:
        inline Prealloc(size_t initialLimit) : _ptr(static_cast<char*>(malloc(initialLimit)), free_), _pos(0), _limit(initialLimit) {}
        inline virtual bool Resize(size_t) override;
        inline virtual void* Alloc(size_t size) override;
        inline virtual size_t Limit() const override { return _limit; }
        inline virtual size_t Used() const override { return _pos; }
        inline virtual size_t Left() const override { return _limit - Used(); }
        inline bool ResizeBuffer(size_t size);
        inline virtual void* AllocationStart() override { return _ptr.get(); }
        
    protected:
        inline virtual void* NextAllocation() { return _ptr.get() + _pos; }
        inline virtual void NextAllocation(void* ptr) override { _pos = static_cast<size_t>(static_cast<char*>(ptr) - _ptr.get()); }
        
    private:
        std::unique_ptr<char, decltype(&free)> _ptr;
        size_t _pos, _limit;
    };
    
    void* PoolNewDelete::operator new(size_t size, Pool& pool)
    {
        auto ptr = pool.Alloc(size);
        static_cast<PoolNewDelete*>(ptr)->_pool = &pool;
        return ptr;
    }
    
    void PoolNewDelete::operator delete(void* ptr)
    {
        static_cast<PoolNewDelete*>(ptr)->_pool->Free(ptr);
    }

    void PoolNewDelete::operator delete(void* ptr, Pool& pool)
    {
        pool.Free(ptr);
    }
    
    std::set<void*>::const_iterator DefaultBlockSearch::Find(const std::set<void*>& blocks, size_t size, const std::function<size_t(void*)>& sizeGetter)
    {
        std::set<void*>::const_iterator it;
        std::set<void*>::const_iterator best = blocks.cend();
        size_t bestMetric = -1;
        bool bestIsAliquot = false;
        
        for (it = blocks.cbegin(); it != blocks.cend(); it++)
        {
            auto block = *it;
            const auto bSize = sizeGetter(block);
            
            if (bSize == size)
            {
                best = it;
                break;
            }
            
            if (bSize < size)
                continue;
            
            size_t metric = bSize;
            auto aliquot = (bSize % size == 0);
            
            if (metric > bestMetric && (aliquot || !bestIsAliquot) || aliquot && !bestIsAliquot)
            {
                best = it;
                bestMetric = metric;
                bestIsAliquot = aliquot;
            }
        }
        
        return best;
    }
    
    template<class TPool, class TBlockSearch>
    void* Dynamic<TPool, TBlockSearch>::Alloc(size_t size)
    {
        size += sizeof(MemInfo);
        
        auto it = _freed.empty() ? _freed.cend() : TBlockSearch::Find(_freed, size, [&](void* ptr) -> size_t { return Size(ptr); });
        void* ptr;
        
        if (it != _freed.end())
        {
            ptr = *it;
            const auto emplaceRequired = size != Size(ptr);
            Size(ptr) = size;
            _freed.erase(it);
            
            if (emplaceRequired)
                _freed.emplace(Next(ptr));
        }
        else
            ptr = TPool::Alloc(size);
        
        if (ptr != nullptr)
            Info(ptr).size = size;
        
        return ToOutterPtr(ptr);
    }
    
    template<class TPool, class TBlockSearch>
    size_t Dynamic<TPool, TBlockSearch>::Used() const
    {
        size_t freedSum = 0;
        
        for (auto freed : _freed)
            freedSum += Size(freed);
        
        return TPool::Used() - freedSum;
    }
    
    template<class TPool, class TBlockSearch>
    void Dynamic<TPool, TBlockSearch>::Free(void* ptr)
    {
        ptr = FromOutterPtr(ptr);
        auto it = _freed.lower_bound(ptr);
        decltype(it) prevIt = it;
        auto isIn = prevIt != _freed.begin() && Next(*--prevIt) == ptr;
        
        if (isIn)
        {
            Size(*prevIt) += Size(ptr);
            ptr = *prevIt;
        }
        
        if (it != _freed.end() && Next(ptr) == *it)
        {
            Size(ptr) += Size(*it);
            _freed.erase(it);
        }
        
        if (Next(ptr) == this->NextAllocation())
        {
            if (isIn)
                _freed.erase(_freed.find(ptr));
            
            this->NextAllocation(ptr);
            return;
        }
        
        if (!isIn)
            _freed.emplace(ptr);
    }
    
    template<class TPool, class TBlockSearch>
    void* Dynamic<TPool, TBlockSearch>::Realloc(void* ptr, size_t size)
    {
        ptr = FromOutterPtr(ptr);
        const auto oldSize = Size(ptr);
        const long delta = static_cast<long>(size - oldSize);
        
        if (delta == 0)
            return ToOutterPtr(ptr);
        
        if (Next(ptr) == this->NextAllocation())
        {
            this->NextAllocation(static_cast<char*>(ptr) + delta);
            Size(ptr) = size;
            return ToOutterPtr(ptr);
        }
        
        if (delta < 0)
        {
            Size(ptr) = size;
            Size(Next(ptr)) = -delta;
            _freed.emplace(Next(ptr));
            return ToOutterPtr(ptr);
        }
        
        if (Stitch(ptr, delta) || StitchBack(ptr, delta))
            return ToOutterPtr(ptr);
        
        auto newPtr = Alloc(size);
        std::memcpy(newPtr, ptr, oldSize);
        Free(ptr);
        return ToOutterPtr(newPtr);
    }
    
    template<class TPool, class TBlockSearch>
    bool Dynamic<TPool, TBlockSearch>::Stitch(void* ptr, size_t delta)
    {
        auto next = Next(ptr);
        auto nextIt = _freed.find(next);
        
        if (nextIt != _freed.end())
        {
            auto nextSize = Size(next);
            
            if (nextSize >= delta)
            {
                Size(ptr) += delta;
                _freed.erase(nextIt);
                
                if (nextSize > delta)
                {
                    Size(Next(ptr)) = nextSize - delta;
                    _freed.emplace(Next(ptr));
                }
                
                return true;
            }
        }
        
        return false;
    }
    
    template<class TPool, class TBlockSearch>
    bool Dynamic<TPool, TBlockSearch>::StitchBack(void*& ptr, size_t size)
    {
        const auto delta = size - Size(ptr);
        auto prevIt = _freed.lower_bound(ptr);
        
        if (prevIt != _freed.begin() && Next(*--prevIt) == ptr)
        {
            auto prev = *prevIt;
            auto prevSize = Size(prev);
            
            if (prevSize >= delta)
            {
                std::memmove(prev, ptr, size);
                ptr = prev;
                Size(ptr) = size;
                _freed.erase(prevIt);
                
                if (prevSize > delta)
                {
                    Size(Next(ptr)) = prevSize - delta;
                    _freed.emplace(Next(ptr));
                }
                
                return true;
            }
        }
        
        return false;
    }
    
    template<size_t limit>
    void* StackPool<limit>::Alloc(size_t size)
    {
        if (_pos + size >= limit)
            return nullptr;
        
        auto ptr = NextAllocation();
        _pos += size;
        return ptr;
    }
    
    template<class resizePolicy, decltype(&malloc) alloc, decltype(&free) free_, decltype(&realloc) realloc_>
    bool Prealloc<resizePolicy, alloc, free_, realloc_>::Resize(size_t newLimit)
    {
        if (!resizePolicy::Allowed())
            return false;
        
        auto newPtr = static_cast<char*>(realloc_(_ptr.get(), newLimit));
        
        if (newPtr == nullptr)
            return false;
        
        _ptr.release();
        _ptr.reset(newPtr);
        _limit = newLimit;
        return true;
    }
    
    template<class resizePolicy, decltype(&malloc) alloc, decltype(&free) free_, decltype(&realloc) realloc_>
    void* Prealloc<resizePolicy, alloc, free_, realloc_>::Alloc(size_t size)
    {
        if (_pos + size >= _limit && !Resize(resizePolicy::CalculateNewSize(_limit, size)))
            return nullptr;
        
        auto ptr = NextAllocation();
        _pos += size;
        return ptr;
    }
    
    template<class resizePolicy, decltype(&malloc) alloc, decltype(&free) free_, decltype(&realloc) realloc_>
    bool Prealloc<resizePolicy, alloc, free_, realloc_>::ResizeBuffer(size_t size)
    {
        if (size < _pos)
            return false;
        
        Resize(size);
        _limit = size;
        return true;
    }
}