#pragma once

#include <SKVMOIP/defines.h>

#include <type_traits>
#include <utility>

template<typename T> T& null_reference() { return *reinterpret_cast<T*>(NULL); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-local-addr"
	template<typename T>
	T& garbage_reference()
	{
		char bytes[sizeof(T)];
		return *reinterpret_cast<T*>(bytes);
	}
#pragma GCC diagnostic pop

template<typename EnumClassType>
constexpr typename std::underlying_type<EnumClassType>::type EnumClassToInt(EnumClassType value)
{
	return static_cast<typename std::underlying_type<EnumClassType>::type>(value);
}

template<typename EnumClassType, typename IntegerType>
constexpr EnumClassType IntToEnumClass(IntegerType intValue)
{
	static_assert(std::is_same<typename std::underlying_type<EnumClassType>::type,IntegerType>::value);
	return static_cast<EnumClassType>(intValue);
}

template<typename T>
class OptionalReference
{
private:
	T& m_value;
	bool m_hasValue;

public:
	OptionalReference() : m_value(garbage_reference<T>()), m_hasValue(false) { }
	OptionalReference(T& value) : m_value(value), m_hasValue(true) { }
	
	OptionalReference(OptionalReference& ref) : m_value(ref.m_value), m_hasValue(ref.m_hasValue) { }
	OptionalReference& operator=(T& value) { m_value = value; m_hasValue = true; return *this; }

	T& value() { return m_value; }
	operator bool() const noexcept { return m_hasValue; }
	bool hasValue() const noexcept { return m_hasValue; }
	bool has_value() const noexcept { return hasValue(); }
	T& operator*() { return m_value; }
	T* operator->() { return &m_value; }
};

template<typename T>
class Optional
{
private:
	T m_value;
	bool m_hasValue;

public:
	Optional() : m_hasValue(false) { }
	template<typename... Args>
	Optional(Args... args) : m_value(std::forward(args)...), m_hasValue(true) { }
	Optional(T&& value) : m_value(std::move(value)), m_hasValue(true) { }
	Optional(T& value) : m_value(value), m_hasValue(true) { }

	Optional(Optional&& optional) : m_value(std::move(optional.m_value)), m_hasValue(optional.m_hasValue) { }
	Optional(Optional& optional) : m_value(optional.m_value), m_hasValue(optional.m_hasValue) { }
	Optional& operator=(Optional& optional)
	{
		m_hasValue = optional.m_hasValue;
		if(m_hasValue)
			m_value = optional.m_value;
		return *this; 
	}
	Optional& operator=(Optional&& optional)
	{
		m_hasValue = optional.m_hasValue;
		if(m_hasValue)
			m_value = std::move(optional.m_value);
		return *this;
	}
	Optional& operator=(T& value)
	{
		m_value = value;
		m_hasValue = true;
	}
	Optional& operator=(T&& value)
	{
		m_value = std::move(value);
		m_hasValue = true;
	}

	T& value() noexcept { return m_value; }
	operator bool() const noexcept { return m_hasValue; }
	bool has_value() const noexcept { return m_hasValue; }
	T& operator*() { return m_value; }
	T* operator->() { return &m_value; }
};

template<typename T1, typename T2>
class Pair
{
public:
	T1 first;
	T2 second;

	Pair() = default;
	Pair(T1& _first, T2& _second) : first(_first), second(_second) { }
	Pair(T1&& _first, T2&& _second) : first(std::move(_first)), second(std::move(_second)) { }

	Pair(Pair& pair) : first(pair.first), second(pair.second) { }
	Pair(Pair&& pair) : first(std::move(pair.first)), second(std::move(pair.second)) { }

	Pair& operator=(Pair& pair)
	{
		first = pair.first;
		second = pair.second;
	}

	Pair& operator=(Pair&& pair)
	{
		first = std::move(pair.first);
		second = std::move(pair.second);
	}
};
