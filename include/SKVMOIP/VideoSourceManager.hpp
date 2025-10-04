#pragma once

#include <SKVMOIP/VideoSource.hpp> // for IVideoSource
#include <common/defines.hpp>
#include <memory> // for std::unique_ptr<>
#include <optional> // for std::optional<>
#include <vector> // for std::vector<>
#include <concepts> // for std::derived_from<>

namespace SKVMOIP
{ 
	template<typename T> requires(std::derived_from<T, IVideoSource>)
	class IVideoSourceManager
	{
	public:
		IVideoSourceManager() = default;
		virtual ~IVideoSourceManager() = default;

		virtual u32 getNumVideoSources() = 0;

		virtual std::optional<std::unique_ptr<T>> acquireVideoSource(IVideoSource::DeviceID deviceID,
																		IVideoSource::Usage usage,
																		const std::vector<std::tuple<u32, u32, u32>>& resPrefList) = 0;
		virtual void releaseVideoSource(std::unique_ptr<T>& videoSource) = 0;
	};
}
