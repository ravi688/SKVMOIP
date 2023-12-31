#include <SKVMOIP/Event.hpp>
#include <SKVMOIP/assert.h>

namespace SKVMOIP
{
	Event::SubscriptionHandle Event::subscribe(Event::EventHandler handler, EventHandlerData handlerData)
	{
		Event::SubscriptionHandle handle;
		if(m_danglingHandles.size() > 0)
		{
			handle = m_danglingHandles.back();
			m_danglingHandles.pop_back();
		}
		else
			handle = m_counter++;
		m_handlers.insert(std::pair<SubscriptionHandle, std::pair<EventHandler, EventHandlerData>> { handle, { handler, handlerData } });
		return handle;
	}

	bool Event::unsubscribe(Event::SubscriptionHandle handle)
	{
		bool isRemoved = m_handlers.erase(handle) == 1;
		if(isRemoved)
			m_danglingHandles.push_back(handle);
		return isRemoved;
	}

	void Event::publish(void* eventData)
	{
		for(std::pair<SubscriptionHandle, std::pair<EventHandler, EventHandlerData>>&& keyValuePair : m_handlers)
			keyValuePair.second.first(eventData, keyValuePair.second.second);
	}
}
