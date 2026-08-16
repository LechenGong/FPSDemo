#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>

#include <any>
#include <functional>
#include <utility>
#include <typeindex>

#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

typedef NamedStrings EventArgs;
typedef void (*EventCallBackFunc)(EventArgs);

struct EventSubscription
{
	void* legacyFunctionPtr = nullptr;
	std::function<std::any( std::vector<std::any> )> functionPtr;
	std::vector<std::type_index> paramTypes;
	void* objectInstance = nullptr;
	bool isLegacy = false;

	bool IsLegacy() { return isLegacy; }
};

typedef std::vector<EventSubscription> SubscriptionList;

struct EventSystemConfig
{

};

class EventSystem
{
public:
	EventSystem( EventSystemConfig const& config );
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SubscribeEventCallBackFunc( std::string const& eventName, void(*legacyFunctionPtr)(), int numberOfArgs, std::string formatting = "" );
	
	template<typename Ret, typename... Args, size_t... I>
	static std::any CallWithAnyArgsHelper( Ret( *func )(Args...), std::vector<std::any>& args, std::index_sequence<I...> ) 
	{
		return func( std::any_cast<Args>(args[I])... ); // throws if type mismatch
	}

	template<typename Ret, typename T, typename... Args, size_t... I>
	static std::any CallWithAnyArgsHelper( Ret( T::* method )(Args...), T& instance, std::vector<std::any>& args, std::index_sequence<I...> )
	{
		return (instance.*method)(std::any_cast<Args>(args[I])...);
	}

	template<typename Ret, typename... Args>
	std::function<std::any( std::vector<std::any> )> MakeUniversalCallable( Ret( *func )(Args...) ) 
	{
		return [func]( std::vector<std::any> args ) -> std::any 
			{
				if (args.size() != sizeof...(Args))
				{
					throw std::runtime_error( "Wrong number of args" );
				}
				return CallWithAnyArgsHelper( func, args, std::index_sequence_for<Args...>{} );
			};
	}

	template<typename Ret, typename... Args>
	void SubscribeEventCallBackFunc( std::string const& eventName, Ret( *func )(Args...) )
	{
		m_subscriptionMutex.lock();
	
		for (EventSubscription& registeredFunction : m_subscriptionListByName[eventName]) 
		{
			if (registeredFunction.legacyFunctionPtr == reinterpret_cast<void*>(func))
			{
				m_subscriptionMutex.unlock();
				return;
			}
		}

		EventSubscription newEvent;
		newEvent.legacyFunctionPtr = reinterpret_cast<void*>(func);
		//newEvent.functionPtr = MakeUniversalCallable( func );
		newEvent.functionPtr = [func]( std::vector<std::any> args ) -> std::any {
			return CallWithAnyArgsHelper( func, args, std::index_sequence_for<Args...>{} );
			};
		newEvent.paramTypes = { typeid(Args)... };


		for (EventSubscription& registeredFunctions : m_subscriptionListByName[eventName])
		{
			if (registeredFunctions.functionPtr == nullptr &&
				registeredFunctions.legacyFunctionPtr == nullptr)
			{
				registeredFunctions = newEvent;
				m_subscriptionMutex.unlock();
				return;
			}
		}

		m_subscriptionListByName[eventName].push_back( newEvent );

		m_subscriptionMutex.unlock();
	};

	template<typename T, typename Ret, typename... Args>
	void SubscribeEventCallBackMethod( std::string const& eventName, T* instance, Ret( T::*method )(Args...) )
	{
		m_subscriptionMutex.lock();

		void* legacyFunctionPtr = *reinterpret_cast<void**>(&method);
		void* objectInstance = reinterpret_cast<void*>(instance);

		for (EventSubscription& registeredFunction : m_subscriptionListByName[eventName])
		{
			if (registeredFunction.legacyFunctionPtr == legacyFunctionPtr &&
				registeredFunction.objectInstance == objectInstance )
			{
				m_subscriptionMutex.unlock();
				return;
			}
		}

		EventSubscription newEvent;
		newEvent.paramTypes = { typeid(Args)... };
		newEvent.legacyFunctionPtr = legacyFunctionPtr;
		newEvent.functionPtr = [instance, method]( std::vector<std::any> args ) -> std::any {
			// argument amount/type protected at fire time
			return CallWithAnyArgsHelper( method, *instance, args, std::index_sequence_for<Args...>{} );
			};
		newEvent.objectInstance = objectInstance;

		for (EventSubscription& registeredFunctions : m_subscriptionListByName[eventName])
		{
			if (registeredFunctions.functionPtr == nullptr)
			{
				registeredFunctions = newEvent;
				m_subscriptionMutex.unlock();
				return;
			}
		}

		m_subscriptionListByName[eventName].push_back( newEvent );
		m_subscriptionMutex.unlock();
	}

	void UnsubscribeEventCallbackFunc( std::string const& eventName, void(*legacyFunctionPtr)() );

	template<typename T, typename Ret, typename... Args>
	void UnsubscribeEventCallbackMethod( std::string const& eventName, T* instance, Ret( T::*method )(Args...) )
	{
		m_subscriptionMutex.lock();

		void* legacyFunctionPtr = *reinterpret_cast<void**>(&method);
		void* objectInstance = reinterpret_cast<void*>(instance);

		for (EventSubscription& registeredFunction : m_subscriptionListByName[eventName])
		{
			if (registeredFunction.legacyFunctionPtr == legacyFunctionPtr &&
				registeredFunction.objectInstance == objectInstance)
			{
				registeredFunction.legacyFunctionPtr = nullptr;
				registeredFunction.functionPtr = nullptr;
				registeredFunction.objectInstance = nullptr;
				m_subscriptionMutex.unlock();
				return;
			}
		}
		m_subscriptionMutex.unlock();
	}

	template<typename T>
	void UnsubscribeAllMethodsForObject( T* instance )
	{
		m_subscriptionMutex.lock();

		void* objectInstance = reinterpret_cast<void*>(instance);

		for (auto iter = m_subscriptionListByName.begin(); iter != m_subscriptionListByName.end(); iter++)
		{
			for (EventSubscription& registeredFunction : iter->second)
			{
				if (registeredFunction.objectInstance == objectInstance)
				{
					registeredFunction.legacyFunctionPtr = nullptr;
					registeredFunction.functionPtr = nullptr;
					registeredFunction.objectInstance = nullptr;
				}
			}
		}
		m_subscriptionMutex.unlock();
	}

	int	FireEvent( std::string args );
	int	FireEvent( Strings args );

	std::unordered_map<std::string, SubscriptionList>::iterator CaseInsensitiveFind(
		std::unordered_map<std::string, SubscriptionList>& subscriptionList, const std::string& key );

	template<typename... Args>
	std::vector<std::any> PackToAnyVector( Args&&... args ) 
	{
		return { std::any( std::forward<Args>( args ) )... };
	}

	template<typename... Args>
	int FireEventEX( std::string const& eventName, Args&&... args )
	{
		int counter = 0;
		
		std::vector<std::any> argsVector = PackToAnyVector( std::forward<Args>(args)... );
		size_t argsCount = argsVector.size();

		m_subscriptionMutex.lock();
		auto it = CaseInsensitiveFind( m_subscriptionListByName, eventName );
		if (it != m_subscriptionListByName.end()) 
		{
			for (auto& func : it->second) 
			{
				if (!func.functionPtr && !func.legacyFunctionPtr)
					continue;

				if (func.IsLegacy())
				{
					auto typedFunc = reinterpret_cast<bool(*)(Args...)>(func.legacyFunctionPtr);
					bool flag = typedFunc( std::forward<Args>( args )... );
					if (flag)
					{
						m_subscriptionMutex.unlock();
						return counter;
					}
					counter++;
				}
				else
				{
					if (func.paramTypes.size() > argsCount)
						continue;
					
					std::vector<std::any> thisArgsVector( argsVector.begin(), argsVector.begin() + func.paramTypes.size() );

// 					if (argsVector.size() != func.paramTypes.size())
// 					{
// 						ERROR_RECOVERABLE( (eventName + " event argument amount mismatch").c_str() );
// 						continue;
// 					}

					bool argumentTypesMatches = true;

					for (size_t i = 0; i < func.paramTypes.size(); i++)
					{
						if (std::type_index(thisArgsVector[i].type()) != func.paramTypes[i])
						{
							argumentTypesMatches = false;
							break;
						}
					}

					if (!argumentTypesMatches)
					{
						ERROR_RECOVERABLE( (eventName + " event argument type mismatch").c_str() );
						continue;
					}

					bool flag = false;
					try
					{
						std::any result = func.functionPtr( thisArgsVector );
						flag = std::any_cast<bool>(result);
					}
					catch (std::exception const& e)
					{
						ERROR_RECOVERABLE( ("Error calling event: " + std::string( e.what() )).c_str() );
					}

					if (flag)
					{
						m_subscriptionMutex.unlock();
						return counter;
					}
					counter++;
				}
				
			}
		}
		m_subscriptionMutex.unlock();
		return counter;
	}

	bool IsEventSubscribed( std::string const& eventName );

	Strings GetAllSubscribedName();

protected:
	EventSystemConfig m_config;
	std::unordered_map<std::string, SubscriptionList> m_subscriptionListByName;
	mutable std::recursive_mutex m_subscriptionMutex;
};
