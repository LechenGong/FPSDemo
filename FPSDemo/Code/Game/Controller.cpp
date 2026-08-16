#include "Game/Controller.hpp"
#include "Game/Entity.hpp"
#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Controller::~Controller()
{
	Unpossess();
}

void Controller::Possess( Entity* entity )
{
	if (m_entity == entity)
	{
		return;
	}

	Unpossess();
	m_entity = entity;
	if (m_entity != nullptr)
	{
		m_entity->PossessedBy( this );
	}
}

void Controller::Unpossess()
{
	if (m_entity == nullptr)
	{
		return;
	}

	Entity* previousEntity = m_entity;
	m_entity = nullptr;
	previousEntity->Unpossessed();
}

std::map<std::string, ControllerCreator>& ControllerFactory::GetRegistry()
{
	static std::map<std::string, ControllerCreator> registry;
	return registry;
}

void ControllerFactory::Register( std::string const& name, ControllerCreator creator )
{
	GUARANTEE_OR_DIE( !IsNone( name ), "Cannot register None/empty controller name" );
	GUARANTEE_OR_DIE( creator != nullptr, Stringf( "Null creator for controller: %s", name.c_str() ).c_str() );
	GetRegistry()[name] = creator;
}

Controller* ControllerFactory::Create( std::string const& name )
{
	if (IsNone( name ))
	{
		return nullptr;
	}

	auto found = GetRegistry().find( name );
	GUARANTEE_OR_DIE( found != GetRegistry().end(), Stringf( "Unknown controller: %s", name.c_str() ).c_str() );
	return found->second();
}

bool ControllerFactory::IsNone( std::string const& name )
{
	if (name.empty())
	{
		return true;
	}

	std::string lower = name;
	for (char& c : lower)
	{
		if (c >= 'A' && c <= 'Z')
		{
			c = (char)(c - 'A' + 'a');
		}
	}
	return lower == "none";
}

static Controller* CreatePlayerController()
{
	return new PlayerController();
}

static Controller* CreateAIController()
{
	return new AIController();
}

void ControllerFactory::RegisterStandardControllers()
{
	Register( "PlayerController", CreatePlayerController );
	Register( "AIController", CreateAIController );
}
