#include <vector>
#include <string>

#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Controller.hpp"
#include "Game/Map.hpp"
#include "Game/Entity.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/VertexMap.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"

GameRun::GameRun()
{
}

GameRun::~GameRun()
{
}

void GameRun::Startup()
{
	ControllerFactory::RegisterStandardControllers();
	WeaponDefinition::InitializeWeaponDefs();
	EntityDefinition::InitializeEntityDefs();
	EntityDefinition::InitializeProjectileEntityDef();
	LoadAsset();

	m_clock = new Clock;
	m_map = new Map( this, App::g_gameConfig.defaultMap );
	m_map->Startup();
	m_playerController = m_map->GetPlayerController();
	GUARANTEE_OR_DIE( m_playerController != nullptr, "MapDefinition must spawn an entity with controller=\"PlayerController\"" );
	m_timer = new Timer( 2.f, m_clock );

	m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );
	
	{
		std::vector<Vertex_PCU> grid;
		int count = 5;
		for (int i = -50; i <= 50; i++)
		{
			float thickness = (count == 5) ? 0.05f : 0.025f;
			Rgba8 color = (count == 5) ? Rgba8::GREEN : Rgba8::GRAY;
			DebugAddWorldLine( Vec3( (float)i, -50.f, -thickness ), Vec3( (float)i, 50.f, thickness ), thickness, -1.f, color, color, DebugRenderMode::USE_DEPTH );
			(count == 5) ? count = 1 : count++;
		}
		count = 5;
		for (int i = -50; i <= 50; i++)
		{
			float thickness = (count == 5) ? 0.05f : 0.025f;
			Rgba8 color = (count == 5) ? Rgba8::RED : Rgba8::GRAY;
			DebugAddWorldLine( Vec3( -50.f, (float)i, -0.1f ), Vec3( 50.f, (float)i, thickness ), thickness, -1.f, color, color, DebugRenderMode::USE_DEPTH );
			(count == 5) ? count = 1 : count++;
		}

		DebugAddWorldBasis( Mat44( Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), Vec3::ZERO ), -1.f, DebugRenderMode::USE_DEPTH );
		Mat44 xAxisMat;
		xAxisMat.AppendZRotation( -90.f );
		xAxisMat.SetTranslation3D( Vec3( 0.2f, 0.f, 0.2f ) );
		DebugAddWorldText( "x - Forward", xAxisMat, 0.2f, Vec2( 1.f, 1.f ), -1.f, Rgba8::RED );
		Mat44 yAxisMat;
		yAxisMat.AppendZRotation( 180.f );
		yAxisMat.SetTranslation3D( Vec3( 0.f, 0.2f, 0.2f ) );
		DebugAddWorldText( "y - left", yAxisMat, 0.2f, Vec2( 0.f, 1.f ), -1.f, Rgba8::GREEN );
		Mat44 zAxisMat;
		zAxisMat.AppendXRotation( 90.f );
		zAxisMat.AppendYRotation( 180.f );
		zAxisMat.SetTranslation3D( Vec3( 0.f, -0.2f, 1.f ) );
		DebugAddWorldText( "z - up", zAxisMat, 0.2f, Vec2( 0.f, 0.f ), -1.f, Rgba8::BLUE );
	}
}

void GameRun::Update( float deltaSeconds )
{
	m_map->Update( deltaSeconds );
	m_timer->DecrementPeriodIfElapsed();

	Entity* playerEntity = m_playerController->GetEntity();
	if (DebugRenderIsVisible())
	{
		DebugAddScreenText( Stringf( "Time: %.2f  FPS: %.2f  Scale: %.2f", m_clock->GetTotalSeconds(), 1.f / Clock::s_systemClock.GetDeltaSeconds(), m_clock->GetTimeScale() ),
			m_screenCamera.GetDimensions(), 20.f, Vec2::ZERO, 0.f, Rgba8::WHITE, Rgba8::WHITE );
		if (playerEntity != nullptr)
		{
			bool const grounded = playerEntity->IsGrounded();
			DebugAddScreenText( Stringf( "Grounded: %s", grounded ? "true" : "false" ),
				Vec2( 10.f, m_screenCamera.GetDimensions().y - 10.f ), 20.f, Vec2( 1.f, 0.f ), 0.f,
				grounded ? Rgba8::GREEN : Rgba8::RED, grounded ? Rgba8::GREEN : Rgba8::RED );
			DebugAddMessage( Stringf( "Player %.2f  %.2f  %.2f", playerEntity->m_position.x, playerEntity->m_position.y, playerEntity->m_position.z ),
				0.f, Rgba8::WHITE, Rgba8::WHITE );
		}
	}
	InputResponse();
}

void GameRun::Shutdown()
{
	m_playerController = nullptr;
	if (m_map != nullptr)
	{
		m_map->Shutdown();
		delete m_map;
		m_map = nullptr;
	}
}

void GameRun::Render() const
{
	g_theRenderer->BeginCamera( m_playerController->m_camera );
	g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	m_map->Render();
	g_theRenderer->EndCamera( m_playerController->m_camera );

	DebugRenderWorld( m_playerController->m_camera, DebugRenderMode::ALWAYS );
	DebugRenderWorld( m_playerController->m_camera, DebugRenderMode::USE_DEPTH );
	DebugRenderWorld( m_playerController->m_camera, DebugRenderMode::NUL );
	DebugRenderWorld( m_playerController->m_camera, DebugRenderMode::X_RAY );
	DebugRenderScreen( m_screenCamera );

	g_theRenderer->BeginCamera( m_playerController->m_hudCamera );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	m_map->RenderHUD();
	g_theRenderer->EndCamera( m_playerController->m_hudCamera );

	// Restore world depth state so it cannot leak into the next frame.
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
}

void GameRun::InputResponse()
{
	if (m_map != nullptr && m_map->IsMatchOver())
	{
		bool const canLeave = m_map->IsMatchVictory() || m_map->IsEndPromptVisible();
		if (canLeave &&
			(g_theInput->IsNewKeyPressed( KEYCODE_ESC ) ||
			 g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::BACK )))
		{
			g_theApp->SetQuitting( true );
			return;
		}
		if (m_map->IsMatchDefeat() && m_map->IsEndPromptVisible() && g_theInput->IsNewKeyPressed( KEYCODE_SPACE ))
		{
			g_theApp->RequestRestartGame();
		}
		return;
	}

	if (g_theInput->IsNewKeyPressed( KEYCODE_ESC ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::BACK ))
	{
		g_theApp->SetQuitting( true );
	}
}

int GameRun::GetGameMode() const
{
	return 1;
}

//----------------------------------------------------------------------------------------------

GameAttract::GameAttract()
{
}

GameAttract::~GameAttract()
{
}

void GameAttract::Startup()
{
	//g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
	m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );
	m_clock = new Clock;
	m_blinkTimer = new Timer( 1.6f, m_clock );

	m_sunDirection = Vec3( 2.f, 1.f, -1.f );
	m_sunIntensity = 1.f;
	m_ambientIntensity = 1.f;

	m_shapes[0] = PullPaintBaseOnState( NextState::ToGold );
	m_shapes[1] = PullPaintBaseOnState( NextState::ToSetting );
	m_shapes[2] = PullPaintBaseOnState( NextState::ToQuit );
}

void GameAttract::Update( float deltaSeconds )
{
	//m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );
	if (m_blinkTimer->HasPeriodElapsed())
	{
		m_age += 0.f;
	}
	m_blinkTimer->DecrementPeriodIfElapsed();
	m_age += deltaSeconds;

	if (m_rollTimer > 0)
	{
		RollingAnimation( m_rollingDirection, deltaSeconds );
	}
	else
	{
		if (static_cast<int>(m_rollingOffset) != 0)
		{
			if (m_rollingDirection > 0)
				m_selectedState = (NextState)(((int)m_selectedState + 1) % (int)NextState::StateCount);
			else if (m_rollingDirection < 0)
				m_selectedState = (NextState)(((int)m_selectedState + (int)NextState::StateCount - 1) % (int)NextState::StateCount);
			m_rollingOffset = 0;
			m_blinkTimer->Start();
		}
	}
	InputResponse();
}

void GameAttract::Render() const
{
	g_theRenderer->BeginCamera( m_screenCamera );
	Vec3 sunDirFixed = m_sunDirection;
	sunDirFixed.z = -1.f;
	g_theRenderer->SetLightingConstants( sunDirFixed, m_sunIntensity, m_ambientIntensity );
	RenderUI();

	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( g_theRenderer->CreateShader( "Default" ) );
	g_theRenderer->BindTexture( &font->GetTexture() );

	if (m_rollingOffset == 0.f)
	{
		std::vector<Vertex_PCU> displayText;
		displayText.reserve( 50 );
		switch ((NextState)m_selectedState)
		{
		case GameAttract::NextState::ToGold:
			font->AddVertsForTextInBox2D( displayText, AABB2( 500.f, 0.f, 1600.f, 800.f ), 150.f, "PLAY", Rgba8::GREEN );
			break;
		case GameAttract::NextState::ToSetting:
			font->AddVertsForTextInBox2D( displayText, AABB2( 500.f, 0.f, 1600.f, 800.f ), 150.f, "SETTINGS", Rgba8::GRAY );
			break;
		case GameAttract::NextState::ToQuit:
			font->AddVertsForTextInBox2D( displayText, AABB2( 500.f, 0.f, 1600.f, 800.f ), 150.f, "QUIT", Rgba8::RED );
			break;
		}
		g_theRenderer->DrawVertexArray( displayText );
	}


	g_theRenderer->EndCamera( m_screenCamera );
}

void GameAttract::Shutdown()
{
	delete m_blinkTimer;
	m_blinkTimer = nullptr;

	delete m_clock;
	m_clock = nullptr;
}

int GameAttract::GetGameMode() const
{
	return (int)AppState::IN_ATTRACT;
}

void GameAttract::InputResponse()
{
	MenuFunctionButton();
}

void GameAttract::RenderUI() const
{

	float blinkFrac = abs( m_blinkTimer->GetElapsedFraction() - 0.5f ) + 0.5f;
	g_theRenderer->BindTexture( nullptr );

	unsigned char tempAlpha = static_cast<unsigned char>(255 * blinkFrac);
	Vec2 rollingShakeOffset = 8.f * Vec2( g_theRNG->RollRandomFloatInRange( -1.5f, 1.5f ) * m_rollTimer / ROLL_TIME, g_theRNG->RollRandomFloatInRange( -1.5f, 1.5f ) * m_rollTimer / ROLL_TIME );
	for (int i = -2; i <= 2; i++)
	{
		NextState stateIcon = (NextState)((i + (int)NextState::StateCount + (int)m_selectedState) % (int)NextState::StateCount);
		Rgba8 color;
		switch (stateIcon)
		{
		case GameAttract::NextState::ToGold:
			color = Rgba8::GREEN;
			break;
		case GameAttract::NextState::ToSetting:
			color = Rgba8::GRAY;
			break;
		case GameAttract::NextState::ToQuit:
			color = Rgba8::RED;
			break;
		case GameAttract::NextState::StateCount:
			break;
		default:
			break;
		}
		if (i == 0 && m_rollingOffset == 0.f)
		{
			color.a = tempAlpha;
		}
		else
		{
			color.a = 50;
		}
		Vec2 selectionOffset = Vec2( 0.f, static_cast<float>(i * -1.f * 800.f / 3.f) );
		g_theRenderer->SetModelConstants( Mat44::CreateTranslation2D( Vec2( 400.f, 400.f ) + rollingShakeOffset + selectionOffset + Vec2( 0.f, m_rollingOffset ) ), color );
		g_theRenderer->DrawVertexArray( m_shapes[(i + (int)NextState::StateCount + (int)m_selectedState) % (int)NextState::StateCount] );
	}
}

void GameAttract::MenuFunctionButton()
{
	if (g_theInput->IsNewKeyPressed( "confirm" ))
	{
		if (m_rollingOffset == 0.f)
		{
			PlayUiClick();
			switch (m_selectedState)
			{
			case GameAttract::NextState::ToGold:
				g_theApp->SetState( AppState::IN_GAME );
				break;
			case GameAttract::NextState::ToSetting:
				g_theApp->SetState( AppState::IN_SETTING );
				break;
			case GameAttract::NextState::ToQuit:
				g_theApp->SetQuitting( true );
				break;
			case GameAttract::NextState::StateCount:
				break;
			default:
				break;
			}
		}
	}
	if (g_theInput->IsNewKeyPressed( "cancel" ))
	{
		PlayUiClick();
		g_theApp->SetQuitting( true );
	}
	for (int i = 0; i < g_theInput->GetControllerCount(); i++)
	{
		if (g_theInput->GetController( i ).isConnected())
		{
			if (g_theInput->GetController( i ).IsNewButtonDown( XboxButtonID::START ))
			{
				PlayUiClick();
				g_theApp->SetState( AppState::IN_GAME );
				break;
			}
			if (g_theInput->GetController( i ).IsNewButtonDown( XboxButtonID::BACK ))
			{
				PlayUiClick();
				g_theApp->SetQuitting( true );
			}
		}
	}

	if (g_theInput->IsNewKeyPressed( KEYCODE_UP ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::UP ))
	{
		if (m_rollTimer > 0)
			return;

		PlayUiClick();
		ResetRollAnim();
		m_rollingDirection = -1.0f;
	}
	else if (g_theInput->IsNewKeyPressed( KEYCODE_DOWN ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::DOWN ))
	{
		if (m_rollTimer > 0)
			return;

		PlayUiClick();
		ResetRollAnim();
		m_rollingDirection = 1.0f;
	}
}

void GameAttract::RollingAnimation( float directionModifier, float deltaSeconds )
{
	float totalDistance = static_cast<float>(1600.f / 6.f);
	float slowDistance = totalDistance * 0.2f;
	float fastDistance = totalDistance * 0.8f;
	float slowSpeed = slowDistance * 2.0f;
	float fastSpeed = fastDistance * 5.0f;
	float timeElapsed = ROLL_TIME - m_rollTimer;
	if (timeElapsed <= 0.5f)
	{
		m_rollingOffset += directionModifier * slowSpeed * deltaSeconds;
	}
	else if (timeElapsed <= ROLL_TIME)
	{
		m_rollingOffset += directionModifier * fastSpeed * deltaSeconds;
	}

	m_rollTimer -= deltaSeconds;
}

void GameAttract::AbortAnimation()
{

}

void GameAttract::ResetRollAnim()
{
	m_rollTimer = ROLL_TIME;
}

std::vector<Vertex_PCU> GameAttract::PullPaintBaseOnState( NextState state ) const
{
	std::vector<Vertex_PCU>emptyVector;
	switch (state)
	{
	case NextState::ToGold:
		return g_theMap->RUN_VERT_MAP;
	case NextState::ToSetting:
		return g_theMap->GEAR_VERT_MAP;
	case NextState::ToQuit:
		return g_theMap->QUIT_VERT_MAP;
	default:
		return emptyVector;
	}
}


GameSetting::GameSetting()
{
}

GameSetting::~GameSetting()
{
}

void GameSetting::Startup()
{
	m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	font->AddVertsForText2D( m_categorys, Vec2( 100.f, 500.f ), 40.f, " Audio  ", Rgba8::WHITE, 0.7f );
	font->AddVertsForText2D( m_categorys, Vec2( 100.f, 300.f ), 40.f, " Keybind", Rgba8::WHITE, 0.7f );

	font->AddVertsForTextInBox2D( m_naviMain, AABB2( 0.f, 0.f, 1600.f, 800.f ), 20.f, "Arrow Up/Down key to navigate up and down\nSpace key to select", Rgba8::WHITE, 0.7f, Vec2::ZERO );
	font->AddVertsForTextInBox2D( m_naviAudio, AABB2( 0.f, 0.f, 1600.f, 800.f ), 20.f, "Arrow Up/Down key to navigate up and down\nArrow Left/Right key to change volume", Rgba8::WHITE, 0.7f, Vec2::ZERO );
	font->AddVertsForTextInBox2D( m_naviKeybind, AABB2( 0.f, 0.f, 1600.f, 800.f ), 20.f, "Arrow Up/Down key to navigate up and down\nSpace key to change keybinding", Rgba8::WHITE, 0.7f, Vec2::ZERO );
	font->AddVertsForTextInBox2D( m_naviKeybind1, AABB2( 0.f, 0.f, 1600.f, 800.f ), 20.f, "Press any key to bind\nEsc key to cancel", Rgba8::WHITE, 0.7f, Vec2::ZERO );

	m_selectingCategory = 1;

	m_blinkTimer = new Timer( 0.3f, m_clock );
}

void GameSetting::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if (m_blinkTimer->HasPeriodElapsed())
	{
		m_blinkTimer->Start();
		m_blinkShow = !m_blinkShow;
	}
	InputResponse();
}

void GameSetting::Render() const
{
	std::vector<Vertex_PCU> m_elementsAudio;
	std::vector<Vertex_PCU> m_elementsKeybind;

	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );

	std::vector<Vertex_PCU> categorySelection;
	categorySelection.reserve( 100 );
	font->AddVertsForText2D( categorySelection, Vec2( 100.f, 700 - m_selectingCategory * 200.f ), 40.f, ">       <", Rgba8::WHITE, 0.7f );

	std::vector<Vertex_PCU> elementSelection;
	elementSelection.reserve( 100 );
	font->AddVertsForText2D( elementSelection, Vec2( 1000.f, 810.f - m_selectingElement * 50.f ), 20.f, ">          <", Rgba8::WHITE, 0.7f );

	g_theRenderer->BeginCamera( m_screenCamera );
	g_theRenderer->BindTexture( &font->GetTexture() );

	if (m_selectingElement == 0) g_theRenderer->DrawVertexArray( m_naviMain );
	else if (m_selectingCategory == 1 && m_selectingElement != 0) g_theRenderer->DrawVertexArray( m_naviAudio );
	else if (m_selectingCategory == 2 && m_selectingElement != 0)
		if (g_theApp->m_disableInput) g_theRenderer->DrawVertexArray( m_naviKeybind1 );
		else g_theRenderer->DrawVertexArray( m_naviKeybind );

	g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 255, 255, m_blinkShow || !(m_selectingElement == 0) ? 255 : 0 ) );
	g_theRenderer->DrawVertexArray( categorySelection );

	g_theRenderer->SetModelConstants( Mat44::CreateTranslation2D( m_displayOffset ), Rgba8( 255, g_theApp->m_disableInput ? 0 : 255, g_theApp->m_disableInput ? 0 : 255, m_blinkShow && m_selectingElement > 0 ? 255 : 0 ) );
	g_theRenderer->DrawVertexArray( elementSelection );

	if (m_selectingCategory == 1)
	{
		m_elementsAudio.reserve( 200 );

		font->AddVertsForText2D( m_elementsAudio, Vec2( 700.f, 760.f ), 20.f, "SFX   Volume", Rgba8::WHITE, 0.7f );
		font->AddVertsForTextInBox2D( m_elementsAudio, AABB2( 1000.f, 760.f, 1168.f, 810.f ), 20.f, " " + std::to_string( (int)(App::g_gameConfig.sfxVolume * 100.f) ) + " ", Rgba8::WHITE, 0.7f, Vec2( 0.5f, 0.f ) );

		g_theRenderer->SetModelConstants( Mat44::CreateTranslation2D( m_displayOffset ), Rgba8::WHITE );
		g_theRenderer->DrawVertexArray( m_elementsAudio );
	}
	else if (m_selectingCategory == 2)
	{
		m_elementsKeybind.reserve( 1000 );
		int counter = 0;
		auto keybinding = g_theInput->GetKeybinding();

		for (int i = 0; i < g_theInput->m_keybindingOrder.size(); i++)
		{
			if (keybinding[g_theInput->m_keybindingOrder[i]].canBeModified)
			{
				font->AddVertsForText2D( m_elementsKeybind, Vec2( 700.f, 760.f - counter * 50.f ), 20.f, g_theInput->m_keybindingOrder[i], Rgba8::WHITE, 0.7f );
				font->AddVertsForTextInBox2D( m_elementsKeybind, AABB2( 1000.f, 760.f - counter * 50.f, 1168.f, 810.f - counter * 50.f ), 20.f, " " + g_theInput->GetButtonNameByValue( keybinding[g_theInput->m_keybindingOrder[i]].value ) + " ", Rgba8::WHITE, 0.7f, Vec2( 0.5f, 0.f ) );
				counter++;
			}
		}

		g_theRenderer->SetModelConstants( Mat44().CreateTranslation2D( m_displayOffset ), Rgba8::WHITE );
		g_theRenderer->DrawVertexArray( m_elementsKeybind );
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( m_categorys );

	g_theRenderer->EndCamera( m_screenCamera );
}

void GameSetting::Shutdown()
{
}

int GameSetting::GetGameMode() const
{
	return (int)AppState::IN_SETTING;
}

void GameSetting::InputResponse()
{
	if (m_selectingElement)
	{
		if (m_selectingCategory == 1)
		{
			InputAudio();
		}
		else if (m_selectingCategory == 2)
		{
			InputKeybind();
		}
	}
	else if (m_selectingCategory)
	{
		InputCategory();
	}
}

void GameSetting::InputCategory()
{
	bool hasConnected = g_theInput->GetController( 0 ).isConnected();
	if (g_theInput->IsNewKeyPressed( KEYCODE_UP ) ||
		(hasConnected && g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::UP )))
	{
		PlayUiClick();
		m_blinkTimer->Start();
		m_selectingCategory--;
		if (m_selectingCategory == 0)
		{
			m_selectingCategory = m_totalCategories;
		}
	}
	if (g_theInput->IsNewKeyPressed( KEYCODE_DOWN ) ||
		(hasConnected && g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::DOWN )))
	{
		PlayUiClick();
		m_blinkTimer->Start();
		m_selectingCategory++;
		if (m_selectingCategory == m_totalCategories + 1)
		{
			m_selectingCategory = 1;
		}
	}

	if (g_theInput->IsNewKeyPressed( KEYCODE_ESC ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::B ))
	{
		PlayUiClick();
		m_selectingCategory = 0;
		g_theInput->SaveKeyBindingsToXml();
		//g_theApp->SaveGameConfig();
		g_theApp->SetQuitting( true );
	}

	if (g_theInput->IsNewKeyPressed( KEYCODE_SPACE ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::A ))
	{
		PlayUiClick();
		m_selectingElement = 1;
		switch (m_selectingCategory)
		{
		case 1:
			m_totalElements = 1;
			break;
		case 2:
			m_totalElements = g_theInput->GetModifiableKeyCount();
			break;
		default:
			break;
		}
	}

}

void GameSetting::InputKeybind()
{
	if (g_theApp->m_disableInput)
	{
		unsigned int latestInput;
		if (g_theInput->m_latestPressedKey != UINT_MAX)
		{
			if (g_theInput->m_latestPressedKey == KEYCODE_ESC)
			{
				g_theApp->m_disableInput = false;
			}
			else
			{
				PlayUiClick();
				latestInput = g_theInput->m_latestPressedKey;
				int oldValue = g_theInput->GetKeyValueByName( g_theInput->m_keybindingOrder[m_selectingElement - 1] );
				std::string conflictFunction = g_theInput->GetKeyNameByValue( latestInput );
				g_theInput->UpdateCurrentKeybinding( g_theInput->m_keybindingOrder[m_selectingElement - 1], latestInput );
				g_theInput->UpdateCurrentKeybinding( conflictFunction, oldValue );
				g_theApp->m_disableInput = false;
			}
		}
	}
	else
	{
		if (g_theInput->IsNewKeyPressed( KEYCODE_UP ))
		{
			PlayUiClick();
			m_selectingElement--;
			if (m_selectingElement <= 0)
			{
				m_selectingElement = m_totalElements;
			}
		}
		if (g_theInput->IsNewKeyPressed( KEYCODE_DOWN ))
		{
			PlayUiClick();
			m_selectingElement++;
			if (m_selectingElement > m_totalElements)
			{
				m_selectingElement = 1;
			}
		}
		if (g_theInput->IsNewKeyPressed( "confirm" ))
		{
			PlayUiClick();
			g_theApp->m_disableInput = true;
			g_theInput->m_latestPressedKey = UINT_MAX;
		}
		if (g_theInput->IsNewKeyPressed( "cancel" ))
		{
			PlayUiClick();
			m_selectingElement = 0;
			m_displayOffset = Vec2::ZERO;
		}
		if (g_theInput->IsNewKeyPressed( KEYCODE_MOUSEWHEELUP ))
		{
			DisplayScrollUp();
		}
		if (g_theInput->IsNewKeyPressed( KEYCODE_MOUSEWHEELDOWN ))
		{
			DisplayScrollDown();
		}
	}
}

void GameSetting::InputAudio()
{
	if (g_theInput->IsNewKeyPressed( KEYCODE_UP ))
	{
		PlayUiClick();
		m_selectingElement--;
		if (m_selectingElement <= 0)
		{
			m_selectingElement = m_totalElements;
		}
	}
	if (g_theInput->IsNewKeyPressed( KEYCODE_DOWN ))
	{
		PlayUiClick();
		m_selectingElement++;
		if (m_selectingElement > m_totalElements)
		{
			m_selectingElement = 1;
		}
	}
	if (g_theInput->IsNewKeyPressed( KEYCODE_LEFT ))
	{
		PlayUiClick();
		App::g_gameConfig.sfxVolume = Clamp( App::g_gameConfig.sfxVolume - 0.05f, 0.0, 1.0 );
	}
	if (g_theInput->IsNewKeyPressed( KEYCODE_RIGHT ))
	{
		PlayUiClick();
		App::g_gameConfig.sfxVolume = Clamp( App::g_gameConfig.sfxVolume + 0.05f, 0.0, 1.0 );
	}


	if (g_theInput->IsNewKeyPressed( "cancel" ))
	{
		PlayUiClick();
		m_selectingElement = 0;
		m_displayOffset = Vec2::ZERO;
	}
	// 	if (g_theInput->IsNewKeyPressed( KEYCODE_MOUSEWHEELUP ))
	// 	{
	// 		DisplayScrollUp();
	// 	}
	// 	if (g_theInput->IsNewKeyPressed( KEYCODE_MOUSEWHEELDOWN ))
	// 	{
	// 		DisplayScrollDown();
	// 	}
}

void GameSetting::DisplayScrollUp()
{
	m_displayOffset -= Vec2( 0.f, 50.f );
}

void GameSetting::DisplayScrollDown()
{
	m_displayOffset += Vec2( 0.f, 50.f );
}
