#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <filesystem>

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine//Input/InputSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"

Window* Window::s_theWindow = nullptr;

Window::Window( WindowConfig const& config )
{
	Window::s_theWindow = this;
	m_config = config;
}

Window::~Window()
{

}

void Window::StartUp()
{
	CreateOSWindow();
}

void Window::BeginFrame()
{
	RunMessagePump();
}

void Window::EndFrame()
{

}

void Window::Shutdown()
{

}

WindowConfig const& Window::GetConfig() const
{
	return m_config;
}

float Window::GetAspect() const
{
	return m_config.m_clientAspect;
}

void* Window::GetDeviceContext() const
{
	return m_displayDeviceContext;
}

void* Window::GetHwnd() const
{
	return m_hwnd;
}

bool Window::IsWindowFocused()
{
	return GetActiveWindow() == (HWND)Window::GetMainWindowInstance()->GetHwnd();
}

IntVec2 Window::GetClientDimensions() const
{
	return m_clientDimensions;
}

Window* Window::GetMainWindowInstance()
{
	return Window::s_theWindow;
}

std::string Window::GetFileNameFromBrowser( const char* directory )
{
	char filename[MAX_PATH];
	filename[0] = '\0';

	OPENFILENAMEA data = {};
	data.lStructSize = sizeof( data );
	data.lpstrFile = filename;
	data.nMaxFile = sizeof( filename );
	data.lpstrFilter = "All\0*.*\0";
	data.nFilterIndex = 1;
	data.lpstrInitialDir = directory;
	data.hwndOwner = (HWND)GetHwnd();
	data.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	while (ShowCursor( true ) < 0)
	{

	}
	std::filesystem::path defaultPath = std::filesystem::current_path();

	GetOpenFileNameA( &data );

	std::filesystem::current_path( defaultPath );

	DWORD error = GetLastError();

	if (error != 0)
	{
		LPSTR messageBuffer = nullptr;
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS;
		FormatMessageA( flags, NULL, error, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPSTR)&messageBuffer, 0, NULL );
		ERROR_RECOVERABLE( messageBuffer );
		LocalFree( messageBuffer );
		return "";
	}

	return data.lpstrFile;
}

void Window::CreateOSWindow()
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	//const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	DWORD windowStyleFlags;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );

	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	if (m_config.m_isFullScreen)
	{
		windowStyleFlags = WS_POPUP;
		m_clientDimensions.x = static_cast<int>(desktopWidth);
		m_clientDimensions.y = static_cast<int>(desktopHeight);
		m_config.m_clientAspect = desktopAspect;
	}
	else
	{
		windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
		// Calculate maximum client size (as some % of desktop size)

		if (m_config.m_size != IntVec2{ -1, -1 })
		{
			m_clientDimensions.x = m_config.m_size.x;
			m_clientDimensions.y = m_config.m_size.y;
			m_config.m_clientAspect = (float)m_clientDimensions.x / (float)m_clientDimensions.y;
		}
		else
		{
			constexpr float maxClientFractionOfDesktop = 0.90f;
			float clientWidth = desktopWidth * maxClientFractionOfDesktop;
			float clientHeight = desktopHeight * maxClientFractionOfDesktop;
			if (m_config.m_clientAspect > desktopAspect)
			{
				// Client window has a wider aspect than desktop; shrink client height to match its width
				clientHeight = clientWidth / m_config.m_clientAspect;
			}
			else
			{
				// Client window has a taller aspect than desktop; shrink client width to match its height
				clientWidth = clientHeight * m_config.m_clientAspect;
			}

			m_clientDimensions.x = static_cast<int>(clientWidth);
			m_clientDimensions.y = static_cast<int>(clientHeight);
		}
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - float( m_clientDimensions.x ));
	float clientMarginY = 0.5f * (desktopHeight - float( m_clientDimensions.y ));
	if (!m_config.m_isFullScreen && m_config.m_pos != IntVec2{ -1, -1 })
	{
		clientMarginX = (float)m_config.m_pos.x;
		clientMarginY = (float)m_config.m_pos.y;
	}
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + m_clientDimensions.x;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + m_clientDimensions.y;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[1024];
	MultiByteToWideChar( GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[0] ) );

	m_hwnd = static_cast<void*>(CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		//(HINSTANCE)applicationInstanceHandle,
		NULL,
		NULL ));

	ShowWindow( static_cast<HWND>(m_hwnd), SW_SHOW );
	SetForegroundWindow( static_cast<HWND>(m_hwnd) );
	SetFocus( static_cast<HWND>(m_hwnd) );

	m_displayDeviceContext = GetDC( static_cast<HWND>(m_hwnd) );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );
}

void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage ); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}

extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam );

LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	Window* window = Window::GetMainWindowInstance();
	GUARANTEE_OR_DIE( window != nullptr, "Window was null!" );

	InputSystem* input = window->GetConfig().m_inputSystem;
	GUARANTEE_OR_DIE( input != nullptr, "Window's InputSystem was null!" );

	if (ImGui_ImplWin32_WndProcHandler( windowHandle, wmMessageCode, wParam, lParam ))
		return true;

	switch (wmMessageCode)
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
	case WM_CLOSE:
	{
		//g_theApp->SetShuttingDown( true );

		if (g_eventSystem != nullptr)
			g_eventSystem->FireEventEX( "shutdown" );
		else
			ERROR_RECOVERABLE( "App closing not yet supported, terminate from task manager" );
		return 0;
	}

	// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
	case WM_KEYDOWN:
	{
		//input->HandleKeyPressed( (unsigned int)wParam );
		//g_eventSystem->FireEvent( "funcinputdown " + std::to_string( (unsigned int)wParam ) );
		g_eventSystem->FireEventEX( "funcinputdown", (unsigned int)wParam );
		break;
	}

	// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
	case WM_KEYUP:
	{
		//input->HandleKeyReleased( (unsigned int)wParam );
		//g_eventSystem->FireEvent( "funcinputup " + std::to_string( (unsigned int)wParam ) );
		g_eventSystem->FireEventEX( "funcinputup", (unsigned int)wParam );
		break;
	}
	case WM_CHAR:
	{
		//g_eventSystem->FireEvent( "litinput " + std::to_string( (unsigned int)wParam ) );
		g_eventSystem->FireEventEX( "litinput", (unsigned int)wParam );
		break;
	}
	case WM_LBUTTONDOWN:
	{
		//g_eventSystem->FireEvent( "funcinputdown " + std::to_string( (unsigned int)1 ) );
		g_eventSystem->FireEventEX( "funcinputdown", (unsigned int)1 );
		break;
	}
	case WM_RBUTTONDOWN:
	{
		//g_eventSystem->FireEvent( "funcinputdown " + std::to_string( (unsigned int)2 ) );
		g_eventSystem->FireEventEX( "funcinputdown", (unsigned int)2 );
		break;
	}
	case WM_MBUTTONDOWN:
	{
		//g_eventSystem->FireEvent( "funcinputdown " + std::to_string( (unsigned int)3 ) );
		g_eventSystem->FireEventEX( "funcinputdown", (unsigned int)3 );
		break;
	}
	case WM_LBUTTONUP:
	{
		//g_eventSystem->FireEvent( "funcinputup " + std::to_string( (unsigned int)1 ) );
		g_eventSystem->FireEventEX( "funcinputup", (unsigned int)1 );
		break;
	}
	case WM_RBUTTONUP:
	{
		//g_eventSystem->FireEvent( "funcinputup " + std::to_string( (unsigned int)2 ) );
		g_eventSystem->FireEventEX( "funcinputup", (unsigned int)2 );
		break;
	}
	case WM_MBUTTONUP:
	{
		//g_eventSystem->FireEvent( "funcinputup " + std::to_string( (unsigned int)3 ) );
		g_eventSystem->FireEventEX( "funcinputup", (unsigned int)3 );
		break;
	}
	case WM_MOUSEWHEEL:
	{
		//g_eventSystem->FireEvent( "funcinputdown " + std::to_string( (GET_WHEEL_DELTA_WPARAM( wParam ) > 0) ? 4 : 5 ) );
		g_eventSystem->FireEventEX( "funcinputdown", (GET_WHEEL_DELTA_WPARAM( wParam ) > 0) ? 4 : 5 );
		break;
	}
	case WM_MOUSEHWHEEL:
	{
		//g_eventSystem->FireEvent( "funcinputdown " + std::to_string( (GET_WHEEL_DELTA_WPARAM( wParam ) > 0) ? 6 : 7 ) );
		g_eventSystem->FireEventEX( "funcinputdown", (GET_WHEEL_DELTA_WPARAM( wParam ) > 0) ? 6 : 7 );
		break;
	}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}

bool InputSystemKeyDown( unsigned int param )
{
	Window::GetMainWindowInstance()->GetConfig().m_inputSystem->HandleKeyPressed( param );
	Window::GetMainWindowInstance()->GetConfig().m_inputSystem->m_latestPressedKey = param;
	return true;
}

bool InputSystemKeyUp( unsigned int param )
{
 	Window::GetMainWindowInstance()->GetConfig().m_inputSystem->HandleKeyReleased( param );
	return true;
}
