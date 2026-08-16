#pragma once

#include <unordered_map>
#include <vector>
#include <initializer_list>

#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Math/IntVec2.hpp"

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 1;

constexpr float INPUT_BUFFER_THRESHOLD = 0.4f; // 400ms
constexpr float SIMUTANEOUS_PRESS_THRESHOLD = 0.1f; // 100ms

struct InputConfig
{
};

struct CursorState
{
	IntVec2 m_cursorClientDelta;
	IntVec2 m_cursorClientPosition;

	bool m_hiddenMode = false;
	bool m_relativeMode = false;
};

struct Key
{
	unsigned int value;
	bool canBeModified = true;
};

class InputSystem
{
public:
	InputSystem();
	InputSystem( InputConfig const& config );
	~InputSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void HandleKeyPressed( unsigned int keyIndex );
	void HandleKeyReleased( unsigned int keyIndex );
	void UpdatePrevState();

	bool IsKeyDown( unsigned int keyIndex );
	bool IsKeyDown( std::string keyName );
	bool IsNewKeyPressed( unsigned int keyIndex );
	bool IsNewKeyPressed( std::string keyName );
	bool IsNewKeyPressedRecently( unsigned int keyIndex );
	bool IsNewKeyPressedRecently( std::string keyName );
	bool AreNewKeysPressed( std::initializer_list<unsigned int> keyIndexes );
	bool AreNewKeysPressed( std::initializer_list<std::string> keyNames );
	bool WasKeyJustPressed( unsigned int keyIndex );
	bool WasKeyJustPressed( std::string keyName );
	bool WasKeyJustReleased( unsigned int keyIndex );
	bool WasKeyJustReleased( std::string keyName );
	bool WereNewKeysPressedRecently( std::initializer_list<unsigned int> keyIndexes );
	bool WereNewKeysPressedRecently( std::initializer_list<std::string> keyNames );
	int GetControllerCount() const;
	XboxController const& GetController( int controllerID );

	int IsMouseWheelJustScrolled();

	void SetCursorMode( bool hiddenMode, bool relativeMode );
	void UpdateRawCursorClientPosition();
	Vec2 GetCursorClientDelta() const;
	Vec2 GetCursorNormalizedPosition() const;
	CursorState GetCursorState() const { return m_cursorState; }

	void LoadKeyNameLookUp();
	void LoadKeyBindingsFromXml();
	void SaveKeyBindingsToXml();
	std::string GetButtonNameByValue( unsigned int index );
	std::string GetKeyNameByFunction( std::string functionName );
	std::unordered_map<std::string, Key> GetKeybinding() const;
	void UpdateCurrentKeybinding( std::string name, int newValue );
	int GetModifiableKeyCount();
	int GetKeyValueByName( std::string name );
	std::string GetKeyNameByValue( unsigned int value );
	bool GetKeyModifiableByName( std::string name );

	unsigned int m_latestPressedKey = UINT_MAX;

	std::vector<std::string> m_keybindingOrder;
private:
	//KeyButtonState m_buttonState[NUM_KEYCODES];
	std::unordered_map<unsigned int, KeyButtonState> m_buttonState;
	std::vector<std::unordered_map<std::string, Key>> m_keybindings;
	std::unordered_map<unsigned int, std::string> m_keyNameLookUp;
	//XboxController m_controllers[NUM_XBOX_CONTROLLERS];
	std::vector<XboxController> m_controllers;
	IntVec2 m_cursorPosLastFrame = IntVec2::ZERO;
	int m_mouseWheelState = 0;

protected:
	InputConfig m_config;
	CursorState m_cursorState;
};

extern const unsigned int KEYCODE_F1;
extern const unsigned int KEYCODE_F2;
extern const unsigned int KEYCODE_F3;
extern const unsigned int KEYCODE_F4;
extern const unsigned int KEYCODE_F5;
extern const unsigned int KEYCODE_F6;
extern const unsigned int KEYCODE_F7;
extern const unsigned int KEYCODE_F8;
extern const unsigned int KEYCODE_F9;
extern const unsigned int KEYCODE_F10;
extern const unsigned int KEYCODE_F11;
extern const unsigned int KEYCODE_F12;
extern const unsigned int KEYCODE_ESC;
extern const unsigned int KEYCODE_SPACE;
extern const unsigned int KEYCODE_ENTER;
extern const unsigned int KEYCODE_LEFT;
extern const unsigned int KEYCODE_RIGHT;
extern const unsigned int KEYCODE_UP;
extern const unsigned int KEYCODE_DOWN;
extern const unsigned int KEYCODE_TILDE;
extern const unsigned int KEYCODE_BACKSPACE;
extern const unsigned int KEYCODE_INSERT;
extern const unsigned int KEYCODE_DELETE;
extern const unsigned int KEYCODE_HOME;
extern const unsigned int KEYCODE_END;
extern const unsigned int KEYCODE_SHIFT;
extern const unsigned int KEYCODE_CTRL;
extern const unsigned int KEYCODE_MOUSELEFT;
extern const unsigned int KEYCODE_MOUSERIGHT;
extern const unsigned int KEYCODE_MOUSEMIDDLE;
extern const unsigned int KEYCODE_MOUSEWHEELUP;
extern const unsigned int KEYCODE_MOUSEWHEELDOWN;
