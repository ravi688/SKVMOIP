#pragma once

#include <SKVMOIP/defines.hpp>

#define skvmoip_assert(condition) SKVMOIP::__skvmoip_assert(condition, #condition)
#define skvmoip_assert_wrn(condition) SKVMOIP::__skvmoip_assert_wrn(condition, #condition)

#ifdef SKVMOIP_DEBUG
#	define skvmoip_debug_assert(condition) SKVMOIP::__skvmoip_assert(condition, #condition)
#	define skvmoip_debug_assert_wrn(condition) SKVMOIP::__skvmoip_assert_wrn(condition, #condition)
#else
#	define skvmoip_debug_assert(condition)
#	define skvmoip_debug_assert_wrn(condition)
#endif // skvmoip_DEBUG

#include <string_view>

namespace SKVMOIP
{
	void __skvmoip_assert(bool condition, const std::string_view description);
	void __skvmoip_assert_wrn(bool condition, const std::string_view description);
}

