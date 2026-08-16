#pragma once

#include <string>
#include <map>

class Entity;

class Controller
{
public:
	Controller() = default;
	virtual ~Controller();

	virtual void Possess( Entity* entity );
	virtual void Unpossess();
	virtual void Update( float deltaSeconds ) = 0;

	Entity* GetEntity() const { return m_entity; }

protected:
	Entity* m_entity = nullptr;
};

using ControllerCreator = Controller* (*)();

class ControllerFactory
{
public:
	static void Register( std::string const& name, ControllerCreator creator );
	static Controller* Create( std::string const& name );
	static bool IsNone( std::string const& name );
	static void RegisterStandardControllers();

private:
	static std::map<std::string, ControllerCreator>& GetRegistry();
};
