#include "Game/App.hpp"
#include "Game/VertexMap.hpp"
#include "Game/Map.hpp"
#include "Game/Entity.hpp"
#include "Game/PlayerController.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"

Window* g_theWindow = nullptr;
App* g_theApp = nullptr;
RandomNumberGenerator* g_theRNG = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
VertexMap* g_theMap = nullptr;
GameConfig App::g_gameConfig = {};

App::App()
{
}

App::~App() {}

void App::Startup()
{
	LoadGameConfig();

	g_theMap = new VertexMap;
	InputConfig inputConfig;
	g_theInput = new InputSystem( inputConfig );
	g_theInput->Startup();
	g_theInput->LoadKeyBindingsFromXml();
	g_theInput->LoadKeyNameLookUp();

	WindowConfig windowConfig = {
		g_theInput,
		"FPSDemo",
		g_gameConfig.windowAspect
	};
	g_theWindow = new Window( windowConfig );
	g_theWindow->StartUp();

	RenderConfig renderConfig = {
		g_theWindow
	};
	g_theRenderer = new Renderer( renderConfig );
	g_theRenderer->Startup();

	g_theRNG = new RandomNumberGenerator;

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );
	g_theAudio->Startup();
	LoadUiSounds();

	m_game = new GameAttract;
	m_game->Startup();

	EventSystemConfig eventSystemConfig;
	g_eventSystem = new EventSystem( eventSystemConfig );
	g_eventSystem->Startup();

	DevConsoleConfig devConsoleConfig = {
		g_theRenderer,
		&m_devCamera,
		g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" ),
		40.5f,
		0.8f,
		100,
		300,
		false,
	};
	g_devConsole = new DevConsole( devConsoleConfig );
	g_devConsole->Startup();

	DebugRenderConfig debugRenderConfig = {
		g_theRenderer,
		g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" ),
	};
	DebugRenderSystemStartup( debugRenderConfig );

	m_devCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );

	g_eventSystem->SubscribeEventCallBackFunc( "pause", reinterpret_cast<void(*)()>(TogglePause), 1 );
	g_eventSystem->SubscribeEventCallBackFunc( "shutdown", reinterpret_cast<void(*)()>(ToggleShuttingDown), 1 );
	g_eventSystem->SubscribeEventCallBackFunc( "control", reinterpret_cast<void(*)()>(PrintAllControls), 1 );
	g_eventSystem->SubscribeEventCallBackFunc( "god", &CommandGod );
	g_eventSystem->SubscribeEventCallBackFunc( "killall", &CommandKillAll );

	g_eventSystem->SubscribeEventCallBackFunc( "funcinputdown", reinterpret_cast<void(*)()>(DevConsoleFunctionKey), 2 );
	g_eventSystem->SubscribeEventCallBackFunc( "litinput", reinterpret_cast<void(*)()>(DevConsoleLiteralKey), 2 );
	g_eventSystem->SubscribeEventCallBackFunc( "funcinputdown", reinterpret_cast<void(*)()>(InputSystemKeyDown), 2 );
	g_eventSystem->SubscribeEventCallBackFunc( "funcinputup", reinterpret_cast<void(*)()>(InputSystemKeyUp), 2 );

	g_eventSystem->FireEvent( "control" );
}

void App::Run()
{
	while (!g_theApp->IsShuttingDown())
	{
		g_theApp->RunFrame();

		//SwapBuffers( reinterpret_cast<HDC>(g_theWindow->GetDeviceContext()) );
	}
}

void App::Shutdown()
{
	g_devConsole->Shutdown();
	delete g_devConsole;

	g_eventSystem->Shutdown();
	delete g_eventSystem;

	m_game->Shutdown();
	delete m_game;

	g_theAudio->Shutdown();
	delete g_theAudio;

	g_theRenderer->Shutdown();
	delete g_theRenderer;

	g_theWindow->Shutdown();
	delete g_theWindow;

	g_theInput->Shutdown();
	delete g_theInput;
}

void App::BeginFrame()
{
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
	g_eventSystem->BeginFrame();
	g_devConsole->BeginFrame();
	Clock::TickSystemClock();
}

void App::Reboot()
{
	m_game->Shutdown();
	delete m_game;
	m_game = new GameAttract;
	m_game->Startup();
}

void App::InputResponse()
{
	if (g_theInput->IsKeyDown( 'T' ) ||
		g_theInput->GetController( 0 ).IsButtonDown( XboxButtonID::RS ))
	{
		m_game->m_clock->SetTimeScale( 0.1f );
	}
	if (g_theInput->IsNewKeyPressed( 'P' ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::Y ))
	{
		m_game->m_clock->TogglePause();
	}
	if (g_theInput->IsNewKeyPressed( 'O' ))
	{
		m_game->m_clock->StepSingleFrame();
	}

	if (g_theInput->WasKeyJustReleased( 'T' ) ||
		g_theInput->GetController( 0 ).WasButtonJustReleased( XboxButtonID::RS ))
	{
		m_game->m_clock->SetTimeScale( 1.f );
	}

	if (g_theInput->IsNewKeyPressed( KEYCODE_F1 ))
	{
		Command_DebugRenderToggle();
	}
	if (g_theInput->IsNewKeyPressed( KEYCODE_F8 ))
	{
		Reboot();
	}
}

void App::LoadGameConfig()
{
	XmlDocument mapDefXml;
	const char* filePath = "Data/GameConfig.xml";
	XmlError xmlResult = mapDefXml.LoadFile( filePath );
	GUARANTEE_OR_DIE( xmlResult == tinyxml2::XML_SUCCESS, Stringf( "failed to load xml file" ) );
	XmlElement* rootElement = mapDefXml.RootElement();
	GUARANTEE_OR_DIE( rootElement, Stringf( "rootElement is nullptr" ) );
	g_gameConfig.defaultMap = ParseXmlAttribute( *rootElement, "defaultMap", "" );
	g_gameConfig.sfxVolume = ParseXmlAttribute( *rootElement, "sfxVolume", 1.f );
	g_gameConfig.buttonClickSound = ParseXmlAttribute( *rootElement, "buttonClickSound", "" );
	g_gameConfig.windowAspect = ParseXmlAttribute( *rootElement, "windowAspect", 2.f );
}

void App::Update()
{
	InputResponse();

	if ((HWND)g_theWindow->GetHwnd() != GetActiveWindow() ||
		g_devConsole->GetMode() == DISPLAY ||
		m_game->GetGameMode() == (int)AppState::IN_ATTRACT)
	{
		g_theInput->SetCursorMode( false, false );
	}
	else
	{
		g_theInput->SetCursorMode( true, true );
	}

	float deltaSeconds = Clock::s_systemClock.GetDeltaSeconds();
	if (m_game != nullptr && m_game->m_clock != nullptr)
	{
		deltaSeconds = m_game->m_clock->GetDeltaSeconds();
	}
	UpdateGame( deltaSeconds );

	if (IsQuitting())
	{
		if (m_game->GetGameMode() == (int)AppState::IN_ATTRACT)
		{
			m_isShuttingDown = true;
		}
		else
		{
			Reboot();
		}
		SetQuitting( false );
	}
	else if (m_restartGame)
	{
		m_restartGame = false;
		SetState( AppState::IN_GAME );
	}
	m_age += deltaSeconds;
}

void App::Render() const
{
	g_theRenderer->ClearScreen( Rgba8( 50, 50, 50, 255 ) );

	m_game->Render();

	g_devConsole->Render( m_devCamera.GetBounds() );
}

void App::EndFrame()
{
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theAudio->EndFrame();
	g_eventSystem->EndFrame();
	g_devConsole->EndFrame();
}

void App::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}

void App::UpdateGame( float deltaSeconds )
{
	m_game->Update( deltaSeconds );
}

void App::SetState( AppState state )
{
	m_game->Shutdown();
	delete m_game;
	switch (state)
	{
	case AppState::IN_GAME:
		m_game = new GameRun;
		break;
	case AppState::IN_ATTRACT:
		m_game = new GameAttract;
		break;
	case AppState::IN_SETTING:
		m_game = new GameSetting;
		break;
	case AppState::STATE_COUNT:
		break;
	default:
		break;
	}
	m_game->Startup();
}

void App::RequestRestartGame()
{
	m_restartGame = true;
}

Map* App::GetCurrentMap() const
{
	if (m_game == nullptr)
	{
		return nullptr;
	}
	return m_game->GetMap();
}

bool TogglePause()
{
	g_theApp->SetPause( !g_theApp->IsPaused() );
	if (g_theApp->IsPaused())
	{
		g_devConsole->AddLine( DevConsole::INFOMSG_MINOR, "Game Paused" );
	}
	else
	{
		g_devConsole->AddLine( DevConsole::INFOMSG_MINOR, "Game Unpaused" );
	}
	return true;
}

bool ToggleShuttingDown()
{
	g_theApp->SetShuttingDown( !g_theApp->IsShuttingDown() );
	return true;
}

bool PrintAllControls()
{
	g_devConsole->AddLine( DevConsole::INFOMSG_MINOR, "F1: toggle debug draw" );

	std::unordered_map<std::string, Key> keybinding = g_theInput->GetKeybinding();

	//for (auto iter = keybinding.begin(); iter != keybinding.end(); iter++)

	for (std::string str : g_theInput->m_keybindingOrder)
	{
		Key key = keybinding[str];
		if (key.value == 0)
			continue;
		g_devConsole->AddLine( DevConsole::INFOMSG_MINOR, Stringf( "%s: %s", g_theInput->GetButtonNameByValue( key.value ).c_str(), g_theInput->GetKeyNameByValue( key.value ).c_str() ) );
	}

	return true;
}

bool CommandGod()
{
	Map* map = (g_theApp != nullptr) ? g_theApp->GetCurrentMap() : nullptr;
	if (map == nullptr)
	{
		g_devConsole->AddLine( DevConsole::WARNINGMSG, "god: not in a map" );
		return false;
	}

	bool const enabled = map->TogglePlayerInvincible();
	Entity* player = (map->GetPlayerController() != nullptr) ? map->GetPlayerController()->GetEntity() : nullptr;
	if (player == nullptr || player->m_isDead)
	{
		g_devConsole->AddLine( DevConsole::WARNINGMSG, "god: no living player" );
		return false;
	}

	g_devConsole->AddLine( DevConsole::INFOMSG_MAJOR, enabled ? "god: ON" : "god: OFF" );
	return true;
}

bool CommandKillAll()
{
	Map* map = (g_theApp != nullptr) ? g_theApp->GetCurrentMap() : nullptr;
	if (map == nullptr)
	{
		g_devConsole->AddLine( DevConsole::WARNINGMSG, "killall: not in a map" );
		return false;
	}

	int const killed = map->KillAllDemons();
	g_devConsole->AddLine( DevConsole::INFOMSG_MAJOR, Stringf( "killall: %d", killed ) );
	return true;
}
