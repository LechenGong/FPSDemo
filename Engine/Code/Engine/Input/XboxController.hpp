#pragma once

#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"

enum class XboxButtonID
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	START,
	BACK,
	LT,
	RT,
	LS,
	RS,
	WHITE,
	BLACK,
	A,
	B,
	X,
	Y,
	NUM
};

class XboxController
{
	friend class InputSystem;

public:
	XboxController();
	~XboxController();
	bool isConnected() const;
	int GetControllerID() const;

	AnalogJoysticker const& GetLeftStick() const;
	AnalogJoysticker const& GetRightStick() const;
	float const& GetLeftTrigger() const;
	float const& GetRightTrigger() const;
	KeyButtonState const& GetButton( XboxButtonID buttonID ) const;
	bool IsButtonDown( XboxButtonID buttonID ) const;
	bool IsNewButtonDown( XboxButtonID buttonID ) const;
	bool WasButtonJustPressed( XboxButtonID buttonID ) const;
	bool WasButtonJustReleased( XboxButtonID buttonID ) const;

private:
	void Update();
	void Reset();
	void UpdateJoystick( AnalogJoysticker& out_joystick, short rawX, short rawY );
	void UpdateTrigger(float& out_triggerValue, unsigned char rawValue);
	void UpdateButton( XboxButtonID buttonID, unsigned short bitField, unsigned short bitGate );

private:
	int m_id = -1;
	bool m_isConnected = false;
	float m_leftTrigger = 0.f;
	float m_rightTrigger = 0.f;
	KeyButtonState m_buttons[(int)XboxButtonID::NUM];
	AnalogJoysticker m_leftStick;
	AnalogJoysticker m_rightStick;
};