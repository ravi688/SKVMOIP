#pragma once

#include <SKVMOIP/defines.h>

#include <type_traits>

template<typename T> T& null_reference() { return *reinterpret_cast<T*>(NULL); }

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

