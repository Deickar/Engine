#define WIN32_LEAN_AND_MEAN
#pragma once
#include <ws2tcpip.h>
#include <unordered_map>
#include <list>
#include <CEGUI/CEGUI.h>

#include "NetworkData.h"

class GameConsoleWindow;

class NetworkServer
{
private:
//Just a normal unsigned int
typedef unsigned int UserID;

//Port number that is unlikely to be used on client computers. Also easy to remember.
//http://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers
#define DEFAULT_PORT "22222"
#define DEFAULT_BUFLEN 512

public:
	struct ServerSettings
	{
	};

	NetworkServer(GameConsoleWindow* window);
	~NetworkServer();

	//Potentially receive a port? A password? Welcome message?
	bool Initialize(const ServerSettings& settings);
	bool Update();
	void Shutdown();

private:
	bool AddClient(UserID& outId);
	void RemoveClient(UserID id);

	//See if any client has sent any data since last time
	bool ReceiveClientData();

	//Distribute any received client data to all clients
	bool SendDataToClients();

	//Sends a packet with size 0. Use purely for events that require no data, such as server shutdowns and things. TODO: think about this and see if there actually are zero-data events...
	void SendEventPacket(DataPacketType eventType);

	void SendDisconnectMessage(UserID client_id);

	//Returns true on success
	bool ReadDataHeader(UserID client_id, char* receivingBuffer, DataPacketType* outType, int* outSize);

	//These functions don't need to return a bool, because if the above header returns true, it means that these will... hopefully.. contain the right data.
	void ReadStringData(UserID client_id, char* receivingBuffer, int bufferSize);
	void ReadUserData(UserID client_id, char* receivingBuffer, int bufferSize);

private:
	GameConsoleWindow* consoleWindow;

	CEGUI::Colour serverColour;

	// data buffer
	char network_data[MAX_PACKET_SIZE];
	char packet_header[DataPacketHeader::sizeOfStruct];

	// Socket to listen for new connections
	SOCKET listenSocket;

	// Socket to give to the clients
	SOCKET clientSocket;

	// for error checking return values
	int iResult, iFlag;
	UserID clientId;

	//List with data to send to all clients at the end of each update
	std::list<DataPacket> dataToSend;

	//List of sessions to terminate
	std::list<std::unordered_map<UserID, UserData>::iterator> clientsToDisconnect;

	// table to keep track of each client's socket
	std::unordered_map<UserID, UserData> sessions;
};

