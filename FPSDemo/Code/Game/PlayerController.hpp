#pragma once

#include "Game/Controller.hpp"
#include "Engine/Renderer/Camera.hpp"

class PlayerController : public Controller
{
public:
	PlayerController() = default;
	virtual ~PlayerController() = default;

	void Possess( Entity* entity ) override;
	void Update( float deltaSeconds ) override;

	void InputResponse( float deltaSeconds );
	void UpdateWeaponInput( float deltaSeconds );
	void UpdateShopInput();
	void SetupCamera();
	void SetupHudCamera();
	void UpdateCameraFromEntity( float deltaSeconds = 0.f );
	void UpdateDeathCamera( float deltaSeconds );

public:
	Camera m_camera;
	Camera m_hudCamera;
	float m_dashCooldownTimer = 0.f;
	bool m_sprintEnabled = false;
	bool m_dashEnabled = false;
	bool m_deathFallStarted = false;
	float m_deathFallSeconds = 0.f;
	float m_deathFallSide = 1.f;
	float m_deathFallPitchStart = 0.f;
};
