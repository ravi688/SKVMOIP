#include <SKVMOIP/assert.h>
#undef _ASSERT
#include <spdlog/spdlog.h>
#include <common/third_party/debug_break.h>

namespace SKVMOIP
{
	void __skvmoip_assert(bool condition, const std::string_view description)
	{
		if(!condition)
		{
			spdlog::critical("Assertion Failure: {}", description);
			debug_break();
		}
	}
	void __skvmoip_assert_wrn(bool condition, const std::string_view description)
	{
		if(!condition)
			spdlog::warn("Assertion Failure: {}", description);
	}
}