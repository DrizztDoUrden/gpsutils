#pragma once

#include <functional>

namespace GPSU
{
	enum class AccessMode
	{
		Read = 1,
		Write = 2,
		ReadWrite = Read | Write,
	};

	template<typename TGetValue, typename TSetValue, AccessMode mode>
	class Property {};

	template<typename TGetValue>
	class Property<TGetValue*, void, AccessMode::Read>
	{
	public:
		using TGetter = std::function<TGetValue*()>;

		inline Property(const TGetter& getter)
			: _getter(getter)
		{}

		inline operator TGetValue*() const { return _getter(); }
		inline operator TGetValue&() const { return *_getter(); }
		inline TGetValue& operator*() const { return *_getter(); }
		inline TGetValue* operator->() const { return _getter(); }
		Property(Property&) = delete;
		Property& operator =(Property&) = delete;

		template<typename TRet, typename... TArgs>
		inline TRet operator()(TArgs... args) { return (**this)(args...); }

		template<typename... TArgs>
		inline void operator()(TArgs... args) { (**this)(args...); }

		template<typename TRet, typename... TArgs>
		inline TRet operator[](TArgs... args) { return (*this)->operator [](args...); }

		template<typename... TArgs>
		inline void operator[](TArgs... args) { (*this)->operator [](args...); }

	private:
		const TGetter _getter;
	};

	template<typename TGetValue>
	class Property<TGetValue, std::enable_if_t<!std::is_pointer<TGetValue>::value, void>, AccessMode::Read>
	{
	public:
		using TGetter = std::function<TGetValue()>;
		inline Property(const TGetter& getter) : _getter(getter) {}
		inline operator TGetValue() const { return _getter(); }
		inline TGetValue operator*() const { return _getter(); }
		inline std::remove_reference_t<TGetValue>* operator->() const { return &TGetValue(_getter()); }
		Property(Property&) = delete;
		Property& operator =(Property&) = delete;

		template<typename TRet, typename... TArgs>
		inline TRet operator()(TArgs... args) { return (**this)(args...); }

		template<typename... TArgs>
		inline void operator()(TArgs... args) { (**this)(args...); }

		template<typename TRet, typename... TArgs>
		inline TRet operator[](TArgs... args) { return (*this)->operator [](args...); }

		template<typename... TArgs>
		inline void operator[](TArgs... args) { (*this)->operator [](args...); }

	private:
		const TGetter _getter;
	};

	template<typename TGetValue>
	using ReadOnlyProperty = Property<TGetValue, void, AccessMode::Read>;

	template<typename TSetValue>
	class Property<void, TSetValue, AccessMode::Write>
	{
	public:
		using TSetter = std::function<void(TSetValue)>;
		inline Property(const TSetter& setter) : _setter(setter) {}
		inline void operator=(TSetValue value) { _setter(value); }
		Property(Property&) = delete;
		Property& operator =(Property&) = delete;

	private:
		const TSetter _setter;
	};

	template<typename TSetValue>
	using WriteOnlyProperty = Property<void, TSetValue, AccessMode::Write>;

	template<typename TGetValue, typename TSetValue>
	class Property<TGetValue, TSetValue, AccessMode::ReadWrite>
		: public ReadOnlyProperty<TGetValue>
		, public WriteOnlyProperty<TSetValue>
	{
	public:
		inline Property(const std::function<TGetValue()>& getter, const std::function<void(TSetValue)>& setter)
			: ReadOnlyProperty<TGetValue>(getter)
			, WriteOnlyProperty<TSetValue>(setter)
		{}
		Property(Property&) = delete;
		Property& operator =(Property&) = delete;

		inline void operator=(TSetValue value) { WriteOnlyProperty<TSetValue>::operator=(value); }
	};

	template<typename TGetValue, typename TSetValue = TGetValue>
	using ReadWriteProperty = Property<TGetValue, TSetValue, AccessMode::ReadWrite>;

	template<typename TValue, typename TGetValue = TValue>
	class GetterProperty : public ReadOnlyProperty<TGetValue>
	{
	public:
		template<typename... TArgs>
		GetterProperty(TArgs... args)
			: ReadOnlyProperty<TGetValue>([this]() -> TGetValue { return _container; })
			, _container(args...)
		{}
		GetterProperty(GetterProperty&) = delete;
		GetterProperty& operator =(GetterProperty&) = delete;

	private:
		TValue _container;
	};

	template<typename TValue>
	using RefGetterProperty = GetterProperty<TValue, TValue&>;

	template<typename TValue>
	using ConstRefGetterProperty = GetterProperty<TValue, const TValue&>;

	template<typename TValue, typename TGetValue = TValue, typename TSetValue = TValue>
	class AutoProperty : public ReadWriteProperty<TGetValue, TSetValue>
	{
	public:
		template<typename... TArgs>
		AutoProperty(TArgs... args)
			: ReadWriteProperty<TGetValue, TSetValue>(
				[this]() -> TGetValue { return _container; },
				[this](TSetValue value) { _container = value; })
			, _container(args...)
		{}
		AutoProperty(AutoProperty&) = delete;
		AutoProperty& operator =(AutoProperty&) = delete;

		inline void operator=(TSetValue value) { WriteOnlyProperty<TGetValue>::operator=(value); }

	private:
		TValue _container;
	};
}
