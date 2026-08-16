#include <string>

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

EventSystem* g_eventSystem = nullptr;

EventSystem::EventSystem( EventSystemConfig const& config )
	: m_config( config )
{
}

EventSystem::~EventSystem()
{
}

void EventSystem::Startup()
{
}

void EventSystem::Shutdown()
{
}

void EventSystem::BeginFrame()
{
}

void EventSystem::EndFrame()
{
}

void EventSystem::SubscribeEventCallBackFunc( std::string const& eventName, void(*legacyFunctionPtr)(), int numberOfArgs, std::string formatting )
{
	UNUSED( numberOfArgs );
	UNUSED( formatting );
	m_subscriptionMutex.lock();
	for (EventSubscription& registeredFunction : m_subscriptionListByName[eventName])
	{
		if (registeredFunction.legacyFunctionPtr == nullptr &&
			registeredFunction.functionPtr == nullptr)
		{
			registeredFunction.legacyFunctionPtr = legacyFunctionPtr;
			registeredFunction.isLegacy = true;
// 			subIndex.numberOfArgs = numberOfArgs;
// 			subIndex.formatting = formatting;
			m_subscriptionMutex.unlock();
			return;
		}
	}
	EventSubscription newEvent;
	newEvent.legacyFunctionPtr = legacyFunctionPtr;
	newEvent.isLegacy = true;
	m_subscriptionListByName[eventName].push_back( newEvent );
	m_subscriptionMutex.unlock();
}

void EventSystem::UnsubscribeEventCallbackFunc( std::string const& eventName, void(*legacyFunctionPtr)() )
{
	m_subscriptionMutex.lock();
	for (EventSubscription& subIndex : m_subscriptionListByName[eventName])
	{
		if (subIndex.legacyFunctionPtr == legacyFunctionPtr)
		{
			subIndex.legacyFunctionPtr = nullptr;
			m_subscriptionMutex.unlock();
			return;
		}
	}
	m_subscriptionMutex.unlock();
}

// int EventSystem::FireEvent( std::string const& eventName )
// {
// 	EventArgs args;
// 	return FireEvent( eventName, args );
// }

// int EventSystem::FireEvent( std::string const& eventName, EventArgs& args )
// {
// 	int counter = 0;
// 	for (EventSubscription subIndex : m_subscriptionListByName[eventName])
// 	{
// 		EventCallBackFunc func = subIndex.functionPtr;
// 		if (func != nullptr)
// 		{
// 			func( args );
// 			counter++;
// 		}
// 	}
// 	return counter;
// }

int EventSystem::FireEvent( std::string args )
{
	Strings strs;
	if (args[0] != '"')
		strs = Split( args, " " );
	else
		strs.push_back( args );
	return FireEvent( strs );
}

int EventSystem::FireEvent( Strings args )
{
	std::string eventName = args[0];
	m_subscriptionMutex.lock();
	if (m_subscriptionListByName.find( eventName ) == m_subscriptionListByName.end())
	{
		m_subscriptionMutex.unlock();
		return 0;
	}
		

	int counter = 0;
	for (EventSubscription subIndex : m_subscriptionListByName[eventName])
	{
		if (subIndex.functionPtr == nullptr)
			continue;

		bool flag = false;
		if (eventName == "pause")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.legacyFunctionPtr)();
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "shutdown")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.legacyFunctionPtr)();
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "control")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.legacyFunctionPtr)();
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "help")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.legacyFunctionPtr)();
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "history")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.legacyFunctionPtr)();
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "clear")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.legacyFunctionPtr)();
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "DRClear")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.legacyFunctionPtr)();
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "DRToggle")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.legacyFunctionPtr)();
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "funcinputdown")
		{
			flag = reinterpret_cast<bool(*)(unsigned int)>(subIndex.legacyFunctionPtr)((unsigned int)std::stoul( args[1] ));
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "funcinputup")
		{
			flag = reinterpret_cast<bool(*)(unsigned int)>(subIndex.legacyFunctionPtr)((unsigned int)std::stod( args[1] ));
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "killallenemies")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.legacyFunctionPtr)();
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "litinput")
		{
			flag = reinterpret_cast<bool(*)(unsigned int)>(subIndex.legacyFunctionPtr)((unsigned int)std::stoi( args[1] ));
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "set")
		{
			flag = reinterpret_cast<bool(*)(char const*, float)>(subIndex.legacyFunctionPtr)(args[1].c_str(), stof( args[2] ));
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
		else if (eventName == "LoadGameConfig")
		{
			flag = reinterpret_cast<bool(*)(char const*)>(subIndex.legacyFunctionPtr)(args[1].c_str());
			counter++;
			if (flag)
			{
				m_subscriptionMutex.unlock();
				return flag;
			}
		}
	}
	m_subscriptionMutex.unlock();
	return counter;
}

std::unordered_map<std::string, SubscriptionList>::iterator EventSystem::CaseInsensitiveFind( std::unordered_map<std::string, SubscriptionList>& subscriptionList, const std::string& key )
{
	std::string lowerKey = ToLower( key );
	for (auto i = subscriptionList.begin(); i != subscriptionList.end(); i++)
	{
		if (ToLower( i->first ) == lowerKey)
		{
			return i;
		}
	}
	return subscriptionList.end();
}

bool EventSystem::IsEventSubscribed( std::string const& eventName )
{
	bool subscribed = false;
	m_subscriptionMutex.lock();
	subscribed = CaseInsensitiveFind(m_subscriptionListByName, eventName) != m_subscriptionListByName.end();
	m_subscriptionMutex.unlock();
	return subscribed;
}

Strings EventSystem::GetAllSubscribedName()
{
	Strings names;
	m_subscriptionMutex.lock();
	for (auto &subscription : m_subscriptionListByName)
	{
		names.push_back( subscription.first );
	}
	m_subscriptionMutex.unlock();
	return names;
}
