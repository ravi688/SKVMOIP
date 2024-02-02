
#pragma once

#include <SKVMOIP/defines.h>
#include <unordered_map>
#include <vector>

namespace SKVMOIP
{
	class SKVMOIP_API Event
	{
	public:
		typedef u32 SubscriptionHandle;
		static constexpr SubscriptionHandle GetInvalidSubscriptionHandle() { return U32_MAX; }
		typedef void* EventHandlerData;
		typedef void (*EventHandler)(void* eventData, EventHandlerData handlerData);

	private:
		u32 m_counter;
		std::vector<SubscriptionHandle> m_danglingHandles;
		typedef std::unordered_map<SubscriptionHandle, std::pair<EventHandler, EventHandlerData>> EventHandlerMap;
		EventHandlerMap m_handlers;

	public:

		Event() : m_counter(0) { }
		~Event() = default;

		SubscriptionHandle subscribe(EventHandler handler, void* handlerData = NULL);
		bool unsubscribe(SubscriptionHandle handle);

		void publish(void* eventData = NULL);
	};
}
