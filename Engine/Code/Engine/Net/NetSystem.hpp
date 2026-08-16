#pragma once

#include <string>
#include <queue>


struct NetSystemConfig
{
	std::string m_modeString;
	int m_sendBufferSize = 2048;
	int m_recvBufferSize = 2048;
	std::string m_hostAddressString;
};

enum class Mode
{
	NONE = 0,
	CLIENT,
	SERVER,
	COUNT
};

enum class ClientState
{
	Disconnected,   // Client is not connected to the server
	Connecting,     // Client is attempting to connect to the server
	Connected,      // Client is connected to the server
// 	Sending,        // Client is sending data to the server
// 	Receiving,      // Client is receiving data from the server
	Error           // An error occurred (connection lost, etc.)
};

enum class ServerState
{
	Listening,      // Server is listening for incoming connections
	Accepting,      // Server is accepting a connection from a client
	Connected,      // Server is connected to a client
// 	Sending,        // Server is sending data to a client
// 	Receiving,		// Server is receiving data from a client
	Disconnected,   // Server is not connected to a client
	Error           // An error occurred (e.g., failure to accept a connection)
};

class NetSystem
{
public:
	NetSystem();
	NetSystem( NetSystemConfig const& config );
	~NetSystem();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void AddToSendQueue( const char* );
	Mode GetNetMode() { return m_mode; }

	void Activate();
	void Deactivate();

	bool IsEnabled() { return m_activated; }
	bool IsServer() { return m_mode == Mode::SERVER; }
	bool IsConnected();

protected:
	std::string GetWSAErrorMessage( int errorCode );

protected:
	Mode m_mode = Mode::NONE;
	NetSystemConfig m_config;
	bool m_initialized = false;
	bool m_activated = false;

	uintptr_t m_listenSocket = ~0ull; // equivalent to INVALID_SOCKET;
	uintptr_t m_clientSocket = ~0ull; // equivalent to INVALID_SOCKET;
	unsigned long m_hostAddress = 0;
	unsigned short m_hostPort = 0;

	char* m_sendBuffer = nullptr;
	char* m_recvBuffer = nullptr;

	ClientState m_clientState;
	ServerState m_serverState;

	std::queue<std::string> m_sendQueue;
	std::string m_recvQueue;
};
