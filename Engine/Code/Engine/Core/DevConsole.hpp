#pragma once

#include <string>
#include <mutex>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"

struct AABB2;
class Renderer;
class BitmapFont;
class Camera;
class Timer;

struct DevConsoleConfig
{
	Renderer* renderer = nullptr;
	Camera* camera = nullptr;
	BitmapFont* font = nullptr;
	float linesToRender = 32.5;
	float fontAspect = 0.57f;
	int maxCommandHistory = 100;
	int maxLogLines = 300;
	bool startOpen = false;
};

struct DevConsoleLine
{
	Rgba8 color;
	std::string text;
};

enum DevConsoleMode
{
	HIDDEN,
	DISPLAY
};

class DevConsole
{
public:
	DevConsole( DevConsoleConfig const& config );
	~DevConsole();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void RecordInput();
	void UsePrevInput();
	void UseNextInput();
	void FillInput();
	void ClearSelected();
	void ResetSelected();
	bool IsSelecting() const { return m_selectStart != m_cursorPos; }
	void CopyToClipBoard();
	void PasteFromClipBoard();
	void DisplayScrollUp();
	void DisplayScrollDown();

	void Execute( std::string const& consoleCommandText, bool echoInput = true );
	void AddLine( Rgba8 const& color, std::string const& text );
	void ResetLog();
	void Render( AABB2 const& bounds, Renderer* rendererOverride = nullptr ) const;
	Strings GetHistoryInput();

	DevConsoleMode GetMode() const;
	void SetMode( DevConsoleMode mode );
	void ToggleMode();
	bool IsOn() const { return m_mode != DevConsoleMode::HIDDEN; }

	static const Rgba8 ERRORMSG;
	static const Rgba8 WARNINGMSG;
	static const Rgba8 INFOMSG_MAJOR;
	static const Rgba8 INFOMSG_MINOR;

	std::string m_inputText;
	int	m_cursorPos = 0;
	int m_selectStart = 0;
	Timer* m_cursorTimer = nullptr;

protected:
	void Render_OpenFull( AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect = 1.f ) const;

private:
	bool IsPosSelected( int pos ) const;

	DevConsoleConfig			m_config;
	DevConsoleMode				m_mode = HIDDEN;
	std::vector<DevConsoleLine>	m_lines;
	std::vector<std::string>	m_historyInput;
	int							m_frameNumber = 0;
	int							m_historyIndex;
	mutable std::mutex			m_linesMutex;
	int							m_displayLineOffset = 0;
};

bool PrintAllCommand();
bool ClearScreen();
bool PrintHistory();
bool DevConsoleFunctionKey( unsigned int param );
bool DevConsoleLiteralKey( unsigned int param );