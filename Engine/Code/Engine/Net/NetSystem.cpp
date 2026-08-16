#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Net/NetSystem.hpp"
#include "Game/EngineBuildPreferences.hpp"

NetSystem* g_netSystem = nullptr;

#if !defined( ENGINE_DISABLE_NETWORKING )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "Engine/Core/StringUtils.hpp"

NetSystem::NetSystem()
{
}

NetSystem::NetSystem( NetSystemConfig const& config )
{
	m_config = config;
	std::string modeStringInLower = ToLower( config.m_modeString );
	if (modeStringInLower == "client")
	{
		m_mode = Mode::CLIENT;
	}
	else if (modeStringInLower == "server")
	{
		m_mode = Mode::SERVER;
	}
	else
	{
		m_mode = Mode::NONE;
	}
	Strings ipAddress = Split( config.m_hostAddressString, ':', true );
	if (ipAddress.size() != 2)
	{
		ERROR_AND_DIE( Stringf( "Invalid IP Address: %s", config.m_hostAddressString.c_str() ).c_str() );
	}

	sockaddr_in sa;
	if (inet_pton( AF_INET, ipAddress[0].c_str(), &sa.sin_addr) <= 0) {
		ERROR_AND_DIE( Stringf( "Invalid IP Address: %s", ipAddress[0].c_str() ).c_str() );
	}
	m_hostAddress = sa.sin_addr.S_un.S_addr;
	//unsigned long a = ntohl( sa.sin_addr.S_un.S_addr );
	m_hostPort = htons( static_cast<unsigned short>(std::stoi( ipAddress[1] )) );
	//std::string as = std::to_string( ntohs( m_hostPort ) );

	m_sendBuffer = new char[m_config.m_sendBufferSize];
	m_recvBuffer = new char[m_config.m_recvBufferSize];

// 	memset( m_sendBuffer, 0, m_config.m_sendBufferSize );
// 	memset( m_recvBuffer, 0, m_config.m_recvBufferSize );
}

NetSystem::~NetSystem()
{
}

void NetSystem::Startup()
{
	if (!m_activated)
		return;

	if (m_mode == Mode::CLIENT)
	{
		WSADATA data;
		int result = WSAStartup( MAKEWORD( 2, 2 ), &data );
		if (result != 0)
		{
			int errorCode = WSAGetLastError();
			ERROR_AND_DIE( Stringf( "WSAStartup Failed: %d %s", errorCode, GetWSAErrorMessage( errorCode ).c_str() ).c_str() );
		}

		m_clientSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

		unsigned long blockingMode = 1;
		ioctlsocket( m_clientSocket, FIONBIO, &blockingMode );

		m_clientState = ClientState::Disconnected;
	}
	else if (m_mode == Mode::SERVER)
	{
		WSADATA data;
		int result = WSAStartup( MAKEWORD( 2, 2 ), &data );
		if (result != 0)
		{
			int errorCode = WSAGetLastError();
			ERROR_AND_DIE( Stringf( "WSAStartup Failed: %d %s", errorCode, GetWSAErrorMessage( errorCode ).c_str() ).c_str() );
		}

		m_listenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

		unsigned long blockingMode = 1;
		ioctlsocket( m_listenSocket, FIONBIO, &blockingMode );

		sockaddr_in sa;
		sa.sin_family = AF_INET;
		sa.sin_addr.S_un.S_addr = m_hostAddress;
		sa.sin_port = m_hostPort;
		result = bind( m_listenSocket, (sockaddr*)&sa, (int)sizeof( sa ) );
		if (result == SOCKET_ERROR)
		{
// 			int errorCode = WSAGetLastError();
// 			switch (errorCode) {
// 			case WSAEADDRINUSE:
// 				std::cout << "Address already in use." << std::endl;
// 				break;
// 			case WSAEINVAL:
// 				std::cout << "The socket is already bound to an address." << std::endl;
// 				break;
// 			case WSAENOTSOCK:
// 				std::cout << "The descriptor is not a socket." << std::endl;
// 				break;
// 			default:
// 				std::cout << "Other error." << std::endl;
// 				break;
// 			}
			m_serverState = ServerState::Error;
		}

		listen( m_listenSocket, SOMAXCONN );
		m_serverState = ServerState::Listening;
	}
	m_initialized = true;
}

void NetSystem::Shutdown()
{
	if (m_mode == Mode::CLIENT)
	{
		closesocket( m_clientSocket );
	}
	else if (m_mode == Mode::SERVER)
	{
		closesocket( m_clientSocket );
		closesocket( m_listenSocket );
	}
	m_initialized = false;
	WSACleanup();
}

void NetSystem::BeginFrame()
{
	if (!m_activated)
		return;

	if (m_mode == Mode::CLIENT)
	{
		
		if (m_clientState == ClientState::Disconnected)
		{
			if (m_clientSocket == INVALID_SOCKET)
			{
				m_clientSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
				unsigned long blockingMode = 1;
				ioctlsocket( m_clientSocket, FIONBIO, &blockingMode );
			}
			sockaddr_in sa;
			sa.sin_family = AF_INET;
			sa.sin_addr.S_un.S_addr = m_hostAddress;
			sa.sin_port = m_hostPort;
			int result = connect( m_clientSocket, (sockaddr*)(&sa), (int)sizeof( sa ) );
			if (result == SOCKET_ERROR)
			{
				int errorCode = WSAGetLastError();
				if (errorCode == WSAEWOULDBLOCK)
				{
					// In the process of connecting
					m_clientState = ClientState::Connecting;
				}
				else
				{
					// Failed to connect
					m_clientState = ClientState::Error;
					DebuggerPrintf( "Error: %d", errorCode );
					//ERROR_AND_DIE( Stringf( "Connect Error: %d %s", errorCode, GetWSAErrorMessage( errorCode ).c_str() ).c_str() );
				}
			}
			else
			{
				m_clientState = ClientState::Connected;
			}
		}
		if (m_clientState == ClientState::Connecting)
		{
			fd_set writeSockets;
			fd_set exceptSockets;
			FD_ZERO( &writeSockets );
			FD_ZERO( &exceptSockets );
			FD_SET( m_clientSocket, &writeSockets );
			FD_SET( m_clientSocket, &exceptSockets );
			timeval waitTime = { };
			int selectResult = select( 0, NULL, &writeSockets, &exceptSockets, &waitTime );

			if (selectResult == SOCKET_ERROR)
			{
				sockaddr_in sa;
				sa.sin_family = AF_INET;
				sa.sin_addr.S_un.S_addr = m_hostAddress;
				sa.sin_port = m_hostPort;
				int result = connect( m_clientSocket, (sockaddr*)(&sa), (int)sizeof( sa ) );
				if (result == SOCKET_ERROR)
				{
					int errorCode = WSAGetLastError();
					if (errorCode == WSAEWOULDBLOCK)
					{
						m_clientState = ClientState::Connecting;
					}
					else
					{
						m_clientState = ClientState::Error;
						DebuggerPrintf( "Error: %d", errorCode );
						//ERROR_AND_DIE( Stringf( "Connect Error: %d %s", errorCode, GetWSAErrorMessage( errorCode ).c_str() ).c_str() );
					}
				}
			}
			else if (selectResult > 0)
			{
				if (FD_ISSET( m_clientSocket, &exceptSockets ))
				{
					sockaddr_in sa;
					sa.sin_family = AF_INET;
					sa.sin_addr.S_un.S_addr = m_hostAddress;
					sa.sin_port = m_hostPort;
					int result = connect( m_clientSocket, (sockaddr*)(&sa), (int)sizeof( sa ) );
					if (result == SOCKET_ERROR)
					{
						int errorCode = WSAGetLastError();
						if (errorCode == WSAEWOULDBLOCK)
						{
							m_clientState = ClientState::Connecting;
						}
						else
						{
							m_clientState = ClientState::Error;
							//ERROR_AND_DIE( Stringf( "Connect Error: %d %s", errorCode, GetWSAErrorMessage( errorCode ).c_str() ).c_str() );
						}
					}
					else
					{
						m_clientState = ClientState::Connected;
					}
				}
				else if (FD_ISSET( m_clientSocket, &writeSockets ))
				{
					m_clientState = ClientState::Connected;
				}
			}
			else
			{
				m_clientState = ClientState::Connecting;
			}
		}
		if (m_clientState == ClientState::Connected)
		{
			int bufferUsed = 0;

			while (!m_sendQueue.empty() && bufferUsed < m_config.m_sendBufferSize)
			{
				std::string& message = m_sendQueue.front();
				int messageSize = (int)message.size();

				if (!message.empty())
				{
					int spaceRemain = m_config.m_sendBufferSize - bufferUsed;
					int bytesToCopy = MIN( messageSize, spaceRemain );

					if (message.length() == spaceRemain)
					{
						if (bytesToCopy == 1)
							break;

						std::memcpy( &m_sendBuffer[bufferUsed], message.c_str(), bytesToCopy - 1 );
						bufferUsed += bytesToCopy - 1;

						message.erase( 0, bytesToCopy - 1 );

						if (message.empty())
						{
							m_sendQueue.pop();
							ERROR_AND_DIE( "This shouldn't get called, debug" );
						}
					}
					else if (message.length() < spaceRemain)
					{
						std::memcpy( &m_sendBuffer[bufferUsed], message.c_str(), bytesToCopy + 1 );
						bufferUsed += bytesToCopy + 1;

						message.erase( 0, bytesToCopy );

						if (message.empty())
						{
							m_sendQueue.pop();
						}
					}
					else
					{
						std::memcpy( &m_sendBuffer[bufferUsed], message.c_str(), bytesToCopy );
						bufferUsed += bytesToCopy;

						message.erase( 0, bytesToCopy );
					}
				}
			}

			if (bufferUsed > 0)
			{
				int sendResult = send( m_clientSocket, m_sendBuffer, static_cast<int>(bufferUsed), 0 );

				if (sendResult == SOCKET_ERROR)
				{
					int sendError = WSAGetLastError();
					if (sendError == WSAEWOULDBLOCK)
					{
					}
					else
					{
						m_serverState = ServerState::Error;
						m_sendQueue = {};
					}
				}
			}

			int recvResult = recv( m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0 );
			if (recvResult == SOCKET_ERROR)
			{
				int recvError = WSAGetLastError();
				if (recvError == WSAEWOULDBLOCK)
				{
					// Nothing to recv right now
				}
				else
				{
					m_clientState = ClientState::Error;
					DebuggerPrintf( "Error: %d", recvError );
					//ERROR_AND_DIE( Stringf( "Send Failed: %d %s", recvError, GetWSAErrorMessage( recvError ).c_str() ).c_str() );
				}
			}
			else if (recvResult > 0)
			{
				m_recvQueue += std::string( m_recvBuffer, recvResult );
				//m_serverState = ServerState::Receiving;
				//DebuggerPrintf( "Received: %d Bytes\n", recvResult );
			}
			else
			{
				m_clientState = ClientState::Disconnected;
				closesocket( m_clientSocket );
				m_clientSocket = INVALID_SOCKET;
			}

			while (true)
			{
				size_t pos = m_recvQueue.find( '\0' );
				if (pos == std::string::npos)
					break;

				std::string firstMessage = m_recvQueue.substr( 0, pos );
				m_recvQueue.erase( 0, pos + 1 );
				g_devConsole->Execute( firstMessage, false );
				//DebuggerPrintf( "Message: %s\n", firstMessage.c_str() );
			}
		}

		if (m_clientState == ClientState::Error)
		{
			closesocket( m_clientSocket );
			m_clientSocket = INVALID_SOCKET;
			m_clientState = ClientState::Disconnected;
		}
	}
	else if (m_mode == Mode::SERVER)
	{
		if (m_serverState == ServerState::Listening)
		{
			if (m_clientSocket == INVALID_SOCKET)
			{
				m_clientSocket = accept( m_listenSocket, NULL, NULL );
				if (m_clientSocket == INVALID_SOCKET)
				{
					int errorCode = WSAGetLastError();
					if (errorCode != WSAEWOULDBLOCK)
					{
						m_serverState = ServerState::Disconnected;
						//ERROR_AND_DIE( Stringf( "Send Failed: %d %s", errorCode, GetWSAErrorMessage( errorCode ).c_str() ).c_str() );
					}
				}
				else
				{
					unsigned long blockingMode = 1;
					ioctlsocket( m_clientSocket, FIONBIO, &blockingMode );
					m_serverState = ServerState::Connected;
				}
			}
			else
			{
				m_serverState = ServerState::Connected;
			}
		}
		if (m_serverState == ServerState::Connected)
		{
			int bufferUsed = 0;

			while (!m_sendQueue.empty() && bufferUsed < m_config.m_sendBufferSize)
			{
				std::string& message = m_sendQueue.front();
				int messageSize = (int)message.length();

				if (!message.empty())
				{
					int spaceRemain = m_config.m_sendBufferSize - bufferUsed;
					int bytesToCopy = MIN( messageSize, spaceRemain );

					if (message.length() == spaceRemain)
					{
						if (bytesToCopy == 1)
							break;

						std::memcpy( &m_sendBuffer[bufferUsed], message.c_str(), bytesToCopy - 1 );
						bufferUsed += bytesToCopy - 1;

						message.erase( 0, bytesToCopy - 1 );

						if (message.empty())
						{
							m_sendQueue.pop();
							ERROR_AND_DIE( "This shouldn't get called, debug" );
						} 
					}
					else if (message.length() < spaceRemain)
					{
						std::memcpy( &m_sendBuffer[bufferUsed], message.c_str(), bytesToCopy + 1 );
						bufferUsed += bytesToCopy + 1;

						message.erase( 0, bytesToCopy );

						if (message.empty())
						{
							m_sendQueue.pop();
						}
					}
					else
					{
						std::memcpy( &m_sendBuffer[bufferUsed], message.c_str(), bytesToCopy );
						bufferUsed += bytesToCopy;

						message.erase( 0, bytesToCopy );
					}
				}
			}

			if (bufferUsed > 0)
			{
				int sendResult = send( m_clientSocket, m_sendBuffer, static_cast<int>(bufferUsed), 0 );

				if (sendResult == SOCKET_ERROR)
				{
					int sendError = WSAGetLastError();
					if (sendError == WSAEWOULDBLOCK)
					{
					}
					else
					{
						m_serverState = ServerState::Error;
						m_sendQueue = {};
					}
				}
			}

			int recvResult = recv( m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0 );
			if (recvResult == SOCKET_ERROR)
			{
				int recvError = WSAGetLastError();
				if (recvError == WSAEWOULDBLOCK)
				{
					// Nothing to receive right now
				}
				else
				{
					m_serverState = ServerState::Error;
					DebuggerPrintf( "Error: %d", recvError );
					//ERROR_AND_DIE( Stringf( "Send Failed: %d %s", recvError, GetWSAErrorMessage( recvError ).c_str() ).c_str() );
				}
			}
			else if (recvResult > 0)
			{
				m_recvQueue += std::string( m_recvBuffer, recvResult );
				//m_serverState = ServerState::Receiving;
				//DebuggerPrintf( "Received: %d Bytes\n", recvResult );
			}
			else
			{
				m_serverState = ServerState::Listening;
				closesocket( m_clientSocket );
				m_clientSocket = INVALID_SOCKET;
			}

			while (true)
			{
				size_t pos = m_recvQueue.find( '\0' );
				if (pos == std::string::npos)
					break;

				std::string firstMessage = m_recvQueue.substr( 0, pos );
				m_recvQueue.erase( 0, pos + 1 );
				g_devConsole->Execute( firstMessage, false );
				//DebuggerPrintf( "Message: %s\n", firstMessage.c_str() );
			}
		}

		if (m_serverState == ServerState::Error)
		{
			closesocket( m_clientSocket );
			m_clientSocket = INVALID_SOCKET;
			m_serverState = ServerState::Listening;
		}

		if (m_serverState == ServerState::Disconnected)
		{
			closesocket( m_clientSocket );
			m_clientSocket = INVALID_SOCKET;
			m_serverState = ServerState::Listening;
		}
	}
}

void NetSystem::EndFrame()
{
}

void NetSystem::AddToSendQueue( const char* message )
{
	m_sendQueue.push( message );
}

void NetSystem::Activate()
{
	m_activated = true;
	if (!m_initialized)
		Startup();

	std::memset(m_sendBuffer, 0, m_config.m_sendBufferSize);
	std::memset( m_recvBuffer, 0, m_config.m_recvBufferSize );
	while (!m_sendQueue.empty()) m_sendQueue.pop();
	m_recvQueue = "";
}

void NetSystem::Deactivate()
{
	m_activated = false;
	std::memset( m_sendBuffer, 0, m_config.m_sendBufferSize );
	std::memset( m_recvBuffer, 0, m_config.m_recvBufferSize );
	while (!m_sendQueue.empty()) m_sendQueue.pop();
	m_recvQueue = "";
	Shutdown();
}

bool NetSystem::IsConnected()
{
	if (m_mode == Mode::NONE)
		return false;
	if (!m_activated)
		return false;

	if (m_mode == Mode::CLIENT)
	{
		return m_clientState == ClientState::Connected;
	}
	if (m_mode == Mode::SERVER)
	{
		return m_serverState == ServerState::Connected;
	}
	return false;
}

std::string NetSystem::GetWSAErrorMessage( int errorCode )
{
	char* errorMsg = nullptr;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorCode, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPTSTR)&errorMsg, 0, NULL );

	std::string errorStr = (errorMsg != nullptr) ? errorMsg : "Unknown error";

	if (errorMsg)
	{
		LocalFree( errorMsg );
	}
	return errorStr;
}

#endif // !defined( ENGINE_DISABLE_NETWORKING )

#if defined( ENGINE_DISABLE_NETWORKING )

NetSystem::NetSystem()
{
}

NetSystem::NetSystem( NetSystemConfig const& config )
{
}

NetSystem::~NetSystem()
{
}

void NetSystem::Startup()
{
}

void NetSystem::Shutdown()
{
}

void NetSystem::BeginFrame()
{
}

void NetSystem::EndFrame()
{
}

std::string NetSystem::GetWSAErrorMessage( int errorCode )
{
	return std::string();
}

#endif // !defined( ENGINE_DISABLE_NETWORKING )