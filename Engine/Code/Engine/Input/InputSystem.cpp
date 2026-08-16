#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment( lib, "xinput9_1_0" )

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"

InputSystem::InputSystem()
{
}

InputSystem::InputSystem( InputConfig const& config )
{
	m_config = config;
}

InputSystem::~InputSystem()
{
}

void InputSystem::Startup()
{
	m_controllers.resize( NUM_XBOX_CONTROLLERS );
}

void InputSystem::Shutdown()
{
}

void InputSystem::BeginFrame()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		XINPUT_STATE xboxControllerState = {};
		DWORD errorStatus = XInputGetState( controllerIndex, &xboxControllerState );
		if (errorStatus == ERROR_SUCCESS)
		{
			m_controllers[controllerIndex].Update();
		}
	}
	if (m_cursorState.m_hiddenMode == true) 
		while (::ShowCursor( 0 ) > 0);
	else 
		while (::ShowCursor( 1 ) < 0);
	
	UpdateRawCursorClientPosition();
	if (m_cursorState.m_relativeMode == true)
	{
		m_cursorState.m_cursorClientDelta = m_cursorState.m_cursorClientPosition - m_cursorPosLastFrame;
		LPRECT lp = new RECT;
		HWND hwnd = GetActiveWindow();
		GetWindowRect( hwnd, lp );
		SetCursorPos( (int)(lp->right + lp->left) >> 1, (int)(lp->top + lp->bottom) >> 1 );
		UpdateRawCursorClientPosition();
	}
}

void InputSystem::EndFrame()
{
	UpdatePrevState();
	m_cursorPosLastFrame = m_cursorState.m_cursorClientPosition;
	m_cursorState.m_cursorClientDelta = IntVec2::ZERO;
}

void InputSystem::HandleKeyPressed( unsigned int keyIndex )
{
	m_buttonState[keyIndex].currDown = true;
	m_buttonState[keyIndex].timePressed = (float)GetCurrentTimeSeconds();
}

void InputSystem::HandleKeyReleased( unsigned int keyIndex )
{
	m_buttonState[keyIndex].currDown = false;
	m_buttonState[keyIndex].timePressed = 0.f;
}

void InputSystem::UpdatePrevState()
{
// 	for (int i = 0; i < NUM_KEYCODES; i++)
// 	{
// 		m_buttonState[i].prevDown = m_buttonState[i].currDown;
// 	}
	for (std::unordered_map<unsigned int, KeyButtonState>::iterator it = m_buttonState.begin(); it != m_buttonState.end(); ++it)
	{
		it->second.prevDown = it->second.currDown;
	}
	m_buttonState[4].currDown = false;
	m_buttonState[5].currDown = false;
}

bool InputSystem::IsKeyDown( unsigned int keyIndex )
{
	return m_buttonState[keyIndex].currDown;
}

bool InputSystem::IsKeyDown( std::string keyName )
{
	return IsKeyDown( m_keybindings[0][keyName].value );
}

bool InputSystem::IsNewKeyPressed( unsigned int keyIndex )
{
	return IsKeyDown( keyIndex ) && !WasKeyJustPressed( keyIndex );
}

bool InputSystem::IsNewKeyPressed( std::string keyName )
{
	return IsNewKeyPressed( m_keybindings[0][keyName].value );
}

bool InputSystem::IsNewKeyPressedRecently( unsigned int keyIndex )
{
	if ((float)GetCurrentTimeSeconds() - m_buttonState[keyIndex].timePressed > SIMUTANEOUS_PRESS_THRESHOLD)
		return false;
	return true;
}

bool InputSystem::IsNewKeyPressedRecently( std::string keyName )
{
	return IsNewKeyPressedRecently( m_keybindings[0][keyName].value );
}

bool InputSystem::AreNewKeysPressed( std::initializer_list<unsigned int> keyIndexes )
{
	float min = m_buttonState[*keyIndexes.begin()].timePressed;
	float max = min;
	for (unsigned int keyIndex : keyIndexes)
	{
		if (!IsNewKeyPressedRecently( keyIndex ))
			return false;
		if (m_buttonState[keyIndex].timePressed > max)
			max = m_buttonState[keyIndex].timePressed;
		if (m_buttonState[keyIndex].timePressed < min)
			min = m_buttonState[keyIndex].timePressed;
	}
	return (max - min) <= SIMUTANEOUS_PRESS_THRESHOLD;
}

bool InputSystem::AreNewKeysPressed( std::initializer_list<std::string> keyNames )
{
	float min = m_buttonState[m_keybindings[0][*keyNames.begin()].value].timePressed;
	float max = min;
	for (std::string keyName : keyNames)
	{
		unsigned int keyIndex = m_keybindings[0][keyName].value;
		if (!IsNewKeyPressedRecently( keyIndex ))
			return false;
		if (m_buttonState[keyIndex].timePressed > max)
			max = m_buttonState[keyIndex].timePressed;
		if (m_buttonState[keyIndex].timePressed < min)
			min = m_buttonState[keyIndex].timePressed;
	}
	return (max - min) <= SIMUTANEOUS_PRESS_THRESHOLD;
}

bool InputSystem::WasKeyJustPressed( unsigned int keyIndex )
{
	return m_buttonState[keyIndex].prevDown;
}

bool InputSystem::WasKeyJustPressed( std::string keyName )
{
	return WasKeyJustPressed( m_keybindings[0][keyName].value );
}

bool InputSystem::WasKeyJustReleased( unsigned int keyIndex )
{
	return !IsKeyDown( keyIndex ) && WasKeyJustPressed( keyIndex );
}

bool InputSystem::WasKeyJustReleased( std::string keyName )
{
	return WasKeyJustReleased( m_keybindings[0][keyName].value );
}

bool InputSystem::WereNewKeysPressedRecently( std::initializer_list<unsigned int> keyIndexes )
{
	float min = m_buttonState[*keyIndexes.begin()].timePressed;
	float max = min;
	for (unsigned int keyIndex : keyIndexes)
	{
		if (!IsNewKeyPressedRecently( keyIndex ))
			return false;
		if (m_buttonState[keyIndex].timePressed > max)
			max = m_buttonState[keyIndex].timePressed;
		if (m_buttonState[keyIndex].timePressed < min)
			min = m_buttonState[keyIndex].timePressed;
	}
	float nowT = (float)GetCurrentTimeSeconds();
	return (max - min) <= SIMUTANEOUS_PRESS_THRESHOLD && (nowT - min) <= INPUT_BUFFER_THRESHOLD;
}

bool InputSystem::WereNewKeysPressedRecently( std::initializer_list<std::string> keyNames )
{
	float min = m_buttonState[m_keybindings[0][*keyNames.begin()].value].timePressed;
	float max = min;
	for (std::string keyName : keyNames)
	{
		unsigned int keyIndex = m_keybindings[0][keyName].value;
		if (!IsNewKeyPressedRecently( keyIndex ))
			return false;
		if (m_buttonState[keyIndex].timePressed > max)
			max = m_buttonState[keyIndex].timePressed;
		if (m_buttonState[keyIndex].timePressed < min)
			min = m_buttonState[keyIndex].timePressed;
	}
	float nowT = (float)GetCurrentTimeSeconds();
	return (max - min) <= SIMUTANEOUS_PRESS_THRESHOLD && (nowT - min) <= INPUT_BUFFER_THRESHOLD;
}

int InputSystem::GetControllerCount() const
{
	return (int)m_controllers.size();
}

XboxController const& InputSystem::GetController( int controllerID )
{
	return m_controllers[controllerID];
}

int InputSystem::IsMouseWheelJustScrolled()
{
	return m_mouseWheelState;
}

void InputSystem::SetCursorMode( bool hiddenMode, bool relativeMode )
{
	m_cursorState.m_hiddenMode = hiddenMode;
	m_cursorState.m_relativeMode = relativeMode;
}

void InputSystem::UpdateRawCursorClientPosition()
{
	LPPOINT lp = new POINT;
	GetCursorPos( lp );
	HWND hwnd = GetActiveWindow();
	ScreenToClient( hwnd, lp );
	m_cursorState.m_cursorClientPosition = IntVec2( lp->x, lp->y );
}

Vec2 InputSystem::GetCursorClientDelta() const
{
	return (m_cursorState.m_relativeMode == true) ?
		Vec2( (float)m_cursorState.m_cursorClientDelta.x, (float)m_cursorState.m_cursorClientDelta.y ) :
		Vec2( 0.f, 0.f );
}

Vec2 InputSystem::GetCursorNormalizedPosition() const
{
	LPRECT lp = new RECT;
	HWND hwnd = GetActiveWindow();
	GetClientRect( hwnd, lp );
	float width = abs((float)(lp->right - lp->left));
	float height = abs((float)(lp->top - lp->bottom));
	float posXZTO = RangeMapZeroToOne( (float)m_cursorState.m_cursorClientPosition.x, 0.f, width );
	float posYZTO = 1.f - RangeMapZeroToOne( (float)m_cursorState.m_cursorClientPosition.y, 0.f, height );
	return Vec2( posXZTO, posYZTO );
}

void InputSystem::LoadKeyBindingsFromXml()
{
	XmlDocument keyBindingsDefXml;
	const char* filePath = "Data/Keybindings.xml";
	XmlError xmlResult = keyBindingsDefXml.LoadFile( filePath );
	GUARANTEE_OR_DIE( xmlResult == tinyxml2::XML_SUCCESS, Stringf( "failed to load xml file" ) );
	XmlElement* rootElement = keyBindingsDefXml.RootElement();
	GUARANTEE_OR_DIE( rootElement, Stringf( "rootElement is nullptr" ) );
	XmlElement* keybindingElement = rootElement->FirstChildElement();
	int counter = -1;
	while (keybindingElement)
	{
		std::string elementName = keybindingElement->Name();
		GUARANTEE_OR_DIE( elementName == "Keybindings", Stringf( "elementName is invalid(should be Keybindings)" ) );
		XmlElement* keyElement = keybindingElement->FirstChildElement( "Key" );
		counter++;
		std::unordered_map<std::string, Key> keybinding;
		while (keyElement)
		{
			std::string keyName = ParseXmlAttribute( *keyElement, "name", "" );
			unsigned int keyValue = ParseXmlAttribute( *keyElement, "value", 0u );
			if (keyValue != 0)
			{
				bool keyModifiable = ParseXmlAttribute( *keyElement, "canBeModified", true );
				keybinding[keyName] = Key{ keyValue, keyModifiable };
				m_keybindingOrder.push_back( keyName );
			}
			keyElement = keyElement->NextSiblingElement();
		}
		if (!keybinding.empty())
		{
			m_keybindings.push_back( keybinding );
		}
		keybindingElement = keybindingElement->NextSiblingElement();
	}
}

void InputSystem::SaveKeyBindingsToXml()
{
	XmlDocument doc;
	XmlElement* rootElem = doc.NewElement( "Definition" );
	for (size_t i = 0; i < m_keybindings.size(); i++)
	{
		XmlElement* keybindingElem = doc.NewElement( "Keybindings" );

		for (std::string str : m_keybindingOrder)
		{
			Key key = m_keybindings[i][str];
			if (key.value == 0)
				continue;
			XmlElement* keyElem = doc.NewElement( "Key" );
			keyElem->SetAttribute( "name", str.c_str() );
			keyElem->SetAttribute( "value", key.value );
			keyElem->SetAttribute( "canBeModified", key.canBeModified );
			keybindingElem->InsertEndChild( keyElem );
		}
		rootElem->InsertEndChild( keybindingElem );
	}
	doc.InsertFirstChild( rootElem );
	doc.SaveFile( "Data/KeyBindings.xml" );
}

void InputSystem::LoadKeyNameLookUp()
{
	m_keyNameLookUp[0] = "";
	m_keyNameLookUp[1] = "LMouse";
	m_keyNameLookUp[2] = "RMouse";
	m_keyNameLookUp[3] = "MMouse";
	m_keyNameLookUp[4] = "WheelUp";
	m_keyNameLookUp[5] = "WheelDown";
	m_keyNameLookUp[6] = "WheelLeft";
	m_keyNameLookUp[7] = "WheelRight";
	m_keyNameLookUp[8] = "Backspace";
	m_keyNameLookUp[9] = "Tab";
	m_keyNameLookUp[10] = "";
	m_keyNameLookUp[11] = "";
	m_keyNameLookUp[12] = "";
	m_keyNameLookUp[13] = "Enter";
	m_keyNameLookUp[14] = "";
	m_keyNameLookUp[15] = "";
	m_keyNameLookUp[16] = "Shift";
	m_keyNameLookUp[17] = "Ctrl";
	m_keyNameLookUp[18] = "Alt";
	m_keyNameLookUp[19] = "Pause";
	m_keyNameLookUp[20] = "Caps Lock";
	m_keyNameLookUp[21] = "";
	m_keyNameLookUp[22] = "";
	m_keyNameLookUp[23] = "";
	m_keyNameLookUp[24] = "";
	m_keyNameLookUp[25] = "";
	m_keyNameLookUp[26] = "";
	m_keyNameLookUp[27] = "ESC";
	m_keyNameLookUp[28] = "";
	m_keyNameLookUp[29] = "";
	m_keyNameLookUp[30] = "";
	m_keyNameLookUp[31] = "";
	m_keyNameLookUp[32] = "Space";
	m_keyNameLookUp[33] = "Page Up";
	m_keyNameLookUp[34] = "Page Down";
	m_keyNameLookUp[35] = "End";
	m_keyNameLookUp[36] = "Home";
	m_keyNameLookUp[37] = "Arrow Left";
	m_keyNameLookUp[38] = "Arrow Up";
	m_keyNameLookUp[39] = "Arrow Right";
	m_keyNameLookUp[40] = "Arrow Down";
	m_keyNameLookUp[41] = "";
	m_keyNameLookUp[42] = "";
	m_keyNameLookUp[43] = "";
	m_keyNameLookUp[44] = "";
	m_keyNameLookUp[45] = "Insert";
	m_keyNameLookUp[46] = "Delete";
	m_keyNameLookUp[47] = "";
	m_keyNameLookUp[48] = "0";
	m_keyNameLookUp[49] = "1";
	m_keyNameLookUp[50] = "2";
	m_keyNameLookUp[51] = "3";
	m_keyNameLookUp[52] = "4";
	m_keyNameLookUp[53] = "5";
	m_keyNameLookUp[54] = "6";
	m_keyNameLookUp[55] = "7";
	m_keyNameLookUp[56] = "8";
	m_keyNameLookUp[57] = "9";
	m_keyNameLookUp[58] = "";
	m_keyNameLookUp[59] = "";
	m_keyNameLookUp[60] = "";
	m_keyNameLookUp[61] = "=";
	m_keyNameLookUp[62] = "";
	m_keyNameLookUp[63] = "";
	m_keyNameLookUp[64] = "";
	m_keyNameLookUp[65] = "A";
	m_keyNameLookUp[66] = "B";
	m_keyNameLookUp[67] = "C";
	m_keyNameLookUp[68] = "D";
	m_keyNameLookUp[69] = "E";
	m_keyNameLookUp[70] = "F";
	m_keyNameLookUp[71] = "G";
	m_keyNameLookUp[72] = "H";
	m_keyNameLookUp[73] = "I";
	m_keyNameLookUp[74] = "J";
	m_keyNameLookUp[75] = "K";
	m_keyNameLookUp[76] = "L";
	m_keyNameLookUp[77] = "M";
	m_keyNameLookUp[78] = "N";
	m_keyNameLookUp[79] = "O";
	m_keyNameLookUp[80] = "P";
	m_keyNameLookUp[81] = "Q";
	m_keyNameLookUp[82] = "R";
	m_keyNameLookUp[83] = "S";
	m_keyNameLookUp[84] = "T";
	m_keyNameLookUp[85] = "U";
	m_keyNameLookUp[86] = "V";
	m_keyNameLookUp[87] = "W";
	m_keyNameLookUp[88] = "X";
	m_keyNameLookUp[89] = "Y";
	m_keyNameLookUp[90] = "Z";
	m_keyNameLookUp[91] = "Windows";
	m_keyNameLookUp[92] = "";
	m_keyNameLookUp[93] = "Right Click";
	m_keyNameLookUp[94] = "";
	m_keyNameLookUp[95] = "";
	m_keyNameLookUp[96] = "0 (Num Lock)";
	m_keyNameLookUp[97] = "1 (Num Lock)";
	m_keyNameLookUp[98] = "2 (Num Lock)";
	m_keyNameLookUp[99] = "3 (Num Lock)";
	m_keyNameLookUp[100] = "4 (Num Lock)";
	m_keyNameLookUp[101] = "5 (Num Lock)";
	m_keyNameLookUp[102] = "6 (Num Lock)";
	m_keyNameLookUp[103] = "7 (Num Lock)";
	m_keyNameLookUp[104] = "8 (Num Lock)";
	m_keyNameLookUp[105] = "9 (Num Lock)";
	m_keyNameLookUp[106] = "* (Num Lock)";
	m_keyNameLookUp[107] = "+ (Num Lock)";
	m_keyNameLookUp[108] = "";
	m_keyNameLookUp[109] = "- (Num Lock)";
	m_keyNameLookUp[110] = ". (Num Lock)";
	m_keyNameLookUp[111] = "/ (Num Lock)";
	m_keyNameLookUp[112] = "F1";
	m_keyNameLookUp[113] = "F2";
	m_keyNameLookUp[114] = "F3";
	m_keyNameLookUp[115] = "F4";
	m_keyNameLookUp[116] = "F5";
	m_keyNameLookUp[117] = "F6";
	m_keyNameLookUp[118] = "F7";
	m_keyNameLookUp[119] = "F8";
	m_keyNameLookUp[120] = "F9";
	m_keyNameLookUp[121] = "F10";
	m_keyNameLookUp[122] = "F11";
	m_keyNameLookUp[123] = "F12";
	m_keyNameLookUp[124] = "";
	m_keyNameLookUp[125] = "";
	m_keyNameLookUp[126] = "";
	m_keyNameLookUp[127] = "";
	m_keyNameLookUp[128] = "";
	m_keyNameLookUp[129] = "";
	m_keyNameLookUp[130] = "";
	m_keyNameLookUp[131] = "";
	m_keyNameLookUp[132] = "";
	m_keyNameLookUp[133] = "";
	m_keyNameLookUp[134] = "";
	m_keyNameLookUp[135] = "";
	m_keyNameLookUp[136] = "";
	m_keyNameLookUp[137] = "";
	m_keyNameLookUp[138] = "";
	m_keyNameLookUp[139] = "";
	m_keyNameLookUp[140] = "";
	m_keyNameLookUp[141] = "";
	m_keyNameLookUp[142] = "";
	m_keyNameLookUp[143] = "";
	m_keyNameLookUp[144] = "Num Lock";
	m_keyNameLookUp[145] = "Scroll Lock";
	m_keyNameLookUp[146] = "";
	m_keyNameLookUp[147] = "";
	m_keyNameLookUp[148] = "";
	m_keyNameLookUp[149] = "";
	m_keyNameLookUp[150] = "";
	m_keyNameLookUp[151] = "";
	m_keyNameLookUp[152] = "";
	m_keyNameLookUp[153] = "";
	m_keyNameLookUp[154] = "";
	m_keyNameLookUp[155] = "";
	m_keyNameLookUp[156] = "";
	m_keyNameLookUp[157] = "";
	m_keyNameLookUp[158] = "";
	m_keyNameLookUp[159] = "";
	m_keyNameLookUp[160] = "";
	m_keyNameLookUp[161] = "";
	m_keyNameLookUp[162] = "";
	m_keyNameLookUp[163] = "";
	m_keyNameLookUp[164] = "";
	m_keyNameLookUp[165] = "";
	m_keyNameLookUp[166] = "";
	m_keyNameLookUp[167] = "";
	m_keyNameLookUp[168] = "";
	m_keyNameLookUp[169] = "";
	m_keyNameLookUp[170] = "";
	m_keyNameLookUp[171] = "";
	m_keyNameLookUp[172] = "";
	m_keyNameLookUp[173] = "";
	m_keyNameLookUp[174] = "";
	m_keyNameLookUp[175] = "";
	m_keyNameLookUp[176] = "";
	m_keyNameLookUp[177] = "";
	m_keyNameLookUp[178] = "";
	m_keyNameLookUp[179] = "";
	m_keyNameLookUp[180] = "";
	m_keyNameLookUp[181] = "";
	m_keyNameLookUp[182] = "My Computer";
	m_keyNameLookUp[183] = "My Calculator";
	m_keyNameLookUp[184] = "";
	m_keyNameLookUp[185] = "";
	m_keyNameLookUp[186] = "";
	m_keyNameLookUp[187] = "";
	m_keyNameLookUp[188] = ",";
	m_keyNameLookUp[189] = "";
	m_keyNameLookUp[190] = ".";
	m_keyNameLookUp[191] = "/";
	m_keyNameLookUp[192] = "`";
	m_keyNameLookUp[193] = "";
	m_keyNameLookUp[194] = "";
	m_keyNameLookUp[195] = "";
	m_keyNameLookUp[196] = "";
	m_keyNameLookUp[197] = "";
	m_keyNameLookUp[198] = "";
	m_keyNameLookUp[199] = "";
	m_keyNameLookUp[200] = "";
	m_keyNameLookUp[201] = "";
	m_keyNameLookUp[202] = "";
	m_keyNameLookUp[203] = "";
	m_keyNameLookUp[204] = "";
	m_keyNameLookUp[205] = "";
	m_keyNameLookUp[206] = "";
	m_keyNameLookUp[207] = "";
	m_keyNameLookUp[208] = "";
	m_keyNameLookUp[209] = "";
	m_keyNameLookUp[210] = "";
	m_keyNameLookUp[211] = "";
	m_keyNameLookUp[212] = "";
	m_keyNameLookUp[213] = "";
	m_keyNameLookUp[214] = "";
	m_keyNameLookUp[215] = "";
	m_keyNameLookUp[216] = "";
	m_keyNameLookUp[217] = "";
	m_keyNameLookUp[218] = "";
	m_keyNameLookUp[219] = "[";
	m_keyNameLookUp[220] = "\\";
	m_keyNameLookUp[221] = "]";
	m_keyNameLookUp[222] = "'";
	m_keyNameLookUp[223] = "";
	m_keyNameLookUp[224] = "";
	m_keyNameLookUp[225] = "";
	m_keyNameLookUp[226] = "";
	m_keyNameLookUp[227] = "";
	m_keyNameLookUp[228] = "";
	m_keyNameLookUp[229] = "";
	m_keyNameLookUp[230] = "";
	m_keyNameLookUp[231] = "";
	m_keyNameLookUp[232] = "";
	m_keyNameLookUp[233] = "";
	m_keyNameLookUp[234] = "";
	m_keyNameLookUp[235] = "";
	m_keyNameLookUp[236] = "";
	m_keyNameLookUp[237] = "";
	m_keyNameLookUp[238] = "";
	m_keyNameLookUp[239] = "";
	m_keyNameLookUp[240] = "";
	m_keyNameLookUp[241] = "";
	m_keyNameLookUp[242] = "";
	m_keyNameLookUp[243] = "";
	m_keyNameLookUp[244] = "";
	m_keyNameLookUp[245] = "";
	m_keyNameLookUp[246] = "";
	m_keyNameLookUp[247] = "";
	m_keyNameLookUp[248] = "";
	m_keyNameLookUp[249] = "";
	m_keyNameLookUp[250] = "";
	m_keyNameLookUp[251] = "";
	m_keyNameLookUp[252] = "";
	m_keyNameLookUp[253] = "";
	m_keyNameLookUp[254] = "";
	m_keyNameLookUp[255] = "";
}

std::string InputSystem::GetButtonNameByValue( unsigned int index )
{
	if (index < 0 || index > 255)
		return "";

	return m_keyNameLookUp[index];
}

std::string InputSystem::GetKeyNameByFunction( std::string functionName )
{
	for (std::unordered_map<std::string, Key>::iterator it = m_keybindings[0].begin(); it != m_keybindings[0].end(); it++)
	{
		if (it->first == functionName)
		{
			return GetButtonNameByValue( it->second.value );
		}
	}
	return "";
}

std::unordered_map<std::string, Key> InputSystem::GetKeybinding() const
{
	return m_keybindings[0];
}

void InputSystem::UpdateCurrentKeybinding( std::string name, int newValue )
{
	if (m_keybindings[0][name].canBeModified)
	{
		m_keybindings[0][name].value = newValue;
	}
}

int InputSystem::GetModifiableKeyCount()
{
	int count = 0;
	for (std::unordered_map<std::string, Key>::iterator it = m_keybindings[0].begin(); it != m_keybindings[0].end(); it++)
	{
		if (it->second.canBeModified)
		{
			count++;
		}
	}
	return count;
}

int InputSystem::GetKeyValueByName( std::string name )
{
	return m_keybindings[0][name].value;
}

std::string InputSystem::GetKeyNameByValue( unsigned int value )
{
	for (std::unordered_map<std::string, Key>::iterator it = m_keybindings[0].begin(); it != m_keybindings[0].end(); it++)
	{
		if (it->second.value == value)
		{
			return it->first;
		}
	}
	return "";
}

bool InputSystem::GetKeyModifiableByName( std::string name )
{
	return m_keybindings[0][name].canBeModified;
}


const unsigned int KEYCODE_F1 = VK_F1;
const unsigned int KEYCODE_F2 = VK_F2;
const unsigned int KEYCODE_F3 = VK_F3;
const unsigned int KEYCODE_F4 = VK_F4;
const unsigned int KEYCODE_F5 = VK_F5;
const unsigned int KEYCODE_F6 = VK_F6;
const unsigned int KEYCODE_F7 = VK_F7;
const unsigned int KEYCODE_F8 = VK_F8;
const unsigned int KEYCODE_F9 = VK_F9;
const unsigned int KEYCODE_F10 = VK_F10;
const unsigned int KEYCODE_F11 = VK_F11;
const unsigned int KEYCODE_F12 = VK_F12;
const unsigned int KEYCODE_ESC = VK_ESCAPE;
const unsigned int KEYCODE_SPACE = VK_SPACE;
const unsigned int KEYCODE_ENTER = VK_RETURN;
const unsigned int KEYCODE_LEFT = VK_LEFT;
const unsigned int KEYCODE_RIGHT = VK_RIGHT;
const unsigned int KEYCODE_UP = VK_UP;
const unsigned int KEYCODE_DOWN = VK_DOWN;
const unsigned int KEYCODE_TILDE = 0xc0;
const unsigned int KEYCODE_BACKSPACE = VK_BACK;
const unsigned int KEYCODE_INSERT = VK_INSERT;
const unsigned int KEYCODE_DELETE = VK_DELETE;
const unsigned int KEYCODE_HOME = VK_HOME;
const unsigned int KEYCODE_END = VK_END;
const unsigned int KEYCODE_SHIFT = VK_SHIFT;
const unsigned int KEYCODE_CTRL = VK_CONTROL;
const unsigned int KEYCODE_MOUSELEFT = 1;
const unsigned int KEYCODE_MOUSERIGHT = 2;
const unsigned int KEYCODE_MOUSEMIDDLE = 3;
const unsigned int KEYCODE_MOUSEWHEELUP = 4;
const unsigned int KEYCODE_MOUSEWHEELDOWN = 5;
