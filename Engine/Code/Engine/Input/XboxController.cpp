#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment( lib, "xinput9_1_0" )

#include "XboxController.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

XboxController::XboxController()
{
}

XboxController::~XboxController()
{
}

bool XboxController::isConnected() const
{
	return m_isConnected;
}

int XboxController::GetControllerID() const
{
	return m_id;
}

AnalogJoysticker const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}

AnalogJoysticker const& XboxController::GetRightStick() const
{
	return m_rightStick;
}

float const& XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

float const& XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}

KeyButtonState const& XboxController::GetButton( XboxButtonID buttonID ) const
{
	return m_buttons[(int)buttonID];
}

bool XboxController::IsButtonDown( XboxButtonID buttonID ) const
{
	return m_buttons[(int)buttonID].currDown;
}

bool XboxController::IsNewButtonDown( XboxButtonID buttonID ) const
{
	if (IsButtonDown( buttonID ) && !WasButtonJustPressed( buttonID ))
	{
		int a = 0;
		a++;
	}
	return IsButtonDown( buttonID ) && !WasButtonJustPressed( buttonID );
}

bool XboxController::WasButtonJustPressed( XboxButtonID buttonID ) const
{
	return m_buttons[(int)buttonID].prevDown;
}

bool XboxController::WasButtonJustReleased( XboxButtonID buttonID ) const
{
	return !IsButtonDown( buttonID ) && WasButtonJustPressed( buttonID );
}

void XboxController::Update()
{
	XINPUT_STATE xboxControllerState = {};
	DWORD errorStatus = XInputGetState( 0, &xboxControllerState );
	if (errorStatus != ERROR_SUCCESS)
	{
		Reset();
		m_isConnected = false;
	}
	m_isConnected = true;

	UpdateJoystick( m_leftStick, xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY );
	UpdateJoystick( m_rightStick, xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY );
	UpdateTrigger( m_leftTrigger, xboxControllerState.Gamepad.bLeftTrigger );
	UpdateTrigger( m_rightTrigger, xboxControllerState.Gamepad.bRightTrigger );

	UpdateButton( XboxButtonID::UP, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP );
	UpdateButton( XboxButtonID::DOWN, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN );
	UpdateButton( XboxButtonID::LEFT, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT );
	UpdateButton( XboxButtonID::RIGHT, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT );
	UpdateButton( XboxButtonID::START, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_START );
	UpdateButton( XboxButtonID::BACK, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_BACK );
	UpdateButton( XboxButtonID::LT, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB );
	UpdateButton( XboxButtonID::RT, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB );
	UpdateButton( XboxButtonID::LS, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER );
	UpdateButton( XboxButtonID::RS, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER );
	UpdateButton( XboxButtonID::A, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_A );
	UpdateButton( XboxButtonID::B, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_B );
	UpdateButton( XboxButtonID::X, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_X );
	UpdateButton( XboxButtonID::Y, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_Y );
}

void XboxController::Reset()
{
	m_leftStick.Reset();
	m_rightStick.Reset();
	for (KeyButtonState buttonIndex : m_buttons)
	{
		buttonIndex.currDown = false;
		buttonIndex.prevDown = false;
	}
}

void XboxController::UpdateJoystick( AnalogJoysticker& out_joystick, short rawX, short rawY )
{
	out_joystick.UpdatePosition(
		RangeMap( static_cast<float>(rawX), -32768.f, 32767.f, -1.f, 1.f ),
		RangeMap( static_cast<float>(rawY), -32768.f, 32767.f, -1.f, 1.f )
	);
}

void XboxController::UpdateTrigger( float& out_triggerValue, unsigned char rawValue )
{
	out_triggerValue = RangeMap( static_cast<float>(rawValue), 0.f, 255.f, 0.f, 1.f );
}

void XboxController::UpdateButton( XboxButtonID buttonID, unsigned short bitField, unsigned short bitGate )
{
	m_buttons[(int)buttonID].prevDown = m_buttons[(int)buttonID].currDown;
	if (buttonID == XboxButtonID::RT)
	{
		m_buttons[(int)buttonID].currDown = m_rightTrigger >= 0.05f;
	}
	else if (buttonID == XboxButtonID::LT)
	{
		m_buttons[(int)buttonID].currDown = m_leftTrigger >= 0.05f;
	}
	else
	{
		m_buttons[(int)buttonID].currDown = (bitField & bitGate) == bitGate;
	}
}
