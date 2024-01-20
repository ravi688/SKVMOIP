#pragma once

#include <SKVMOIP/defines.h>

#include <type_traits>

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
	T& operator*() { return m_value; }
	T* operator->() { return &m_value; }
};
