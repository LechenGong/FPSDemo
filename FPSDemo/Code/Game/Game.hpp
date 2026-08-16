#pragma once

#include <vector>

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class App;
class Clock;
class Timer;
class PlayerController;
class Map;

class Game
{
public:
	Game() {};
	virtual ~Game() = default;

	virtual void Startup() = 0;
	virtual void Update( float deltaSeconds ) = 0;
	virtual void Render() const = 0;
	virtual void Shutdown() = 0;

	//Input Game----------------------------------
	virtual void InputResponse() = 0;

	virtual int GetGameMode() const = 0;
	virtual Map* GetMap() { return nullptr; }

	Clock* m_clock = nullptr;

private:
};

class GameRun : public Game
{
public:
	GameRun();
	~GameRun();

	void Startup() override;
	void Update( float deltaSeconds ) override;
	void Render() const override;
	void Shutdown() override;

	//Input Game----------------------------------
	void InputResponse() override;

	int GetGameMode() const override;
	Map* GetMap() override { return m_map; }

private:
	Camera m_screenCamera;
	Camera m_worldCamera;
	PlayerController* m_playerController = nullptr;
	Map* m_map = nullptr;
	Timer* m_timer = nullptr;
};

class GameAttract : public Game
{
public:
	GameAttract();
	~GameAttract();

	void Startup() override;
	void Update( float deltaSeconds ) override;
	void Render() const override;
	void Shutdown() override;
	int GetGameMode() const override;

	void InputResponse() override;
	void MenuFunctionButton();

private:
	void RollingAnimation( float directionModifier, float deltaSeconds );
	void AbortAnimation();
	void ResetRollAnim();

	enum class NextState
	{
		ToGold,
		ToSetting,
		ToQuit,
		StateCount
	};
	std::vector<Vertex_PCU> PullPaintBaseOnState( NextState state ) const;
	NextState m_selectedState = NextState::ToGold;
	std::vector<Vertex_PCU> m_shapes[5];
	float const ROLL_TIME = 0.7f;
	float m_rollTimer = 0.f;
	bool m_isRolling = false;
	float m_rollingOffset = 0.f;
	float m_rollingDirection = 0.f;

private:
	Camera m_screenCamera;
	Timer* m_blinkTimer = nullptr;

	Vec3 m_sunDirection = Vec3( 2.f, 1.f, -1.f );
	float m_sunIntensity = 0.85f;
	float m_ambientIntensity = 0.35f;

	void RenderUI() const;

	float m_age = 0.f;
};

class GameSetting : public Game
{
public:
	GameSetting();
	~GameSetting();

	void Startup() override;
	void Update( float deltaSeconds ) override;
	void Render() const override;
	void Shutdown() override;
	int GetGameMode() const override;

	void InputResponse() override;
	void InputCategory();
	void InputKeybind();
	void InputAudio();

	void DisplayScrollUp();
	void DisplayScrollDown();

	Camera m_screenCamera;

private:
	int m_totalCategories = 2;
	int m_totalElements;
	// 0 = false, positive number = index
	int m_selectingCategory = 0;
	// 0 = false, positive number = index
	int m_selectingElement = 0;

private:
	std::vector<Vertex_PCU> m_categorys;
	Vec2 m_displayOffset = Vec2::ZERO;
	Timer* m_blinkTimer = nullptr;
	bool m_blinkShow = true;

	std::vector<Vertex_PCU> m_naviMain;
	std::vector<Vertex_PCU> m_naviAudio;
	std::vector<Vertex_PCU> m_naviKeybind;
	std::vector<Vertex_PCU> m_naviKeybind1;
};