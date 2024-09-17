#include "LeapClient.h"
#include <array>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include "Debug.h"

#pragma comment(lib, "ws2_32.lib")

leap::networking::LeapClient::LeapClient(int port, const std::string& serverIp, int receiveBuffer, int packetBuffer) : m_BufferSize(receiveBuffer)
{
    // Initialize Winsock
    WSADATA wsData;
    WORD version = MAKEWORD(2, 2);
    if (WSAStartup(version, &wsData) != 0)
    {
        std::stringstream ss{};
        ss << "Error initializing Winsock: " << WSAGetLastError();
        Debug::LogError(ss.str());
        return;
    }

    // Create a socket for the client
    m_TCPsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_TCPsocket == INVALID_SOCKET)
    {
        std::stringstream ss{};
        ss << "Error creating socket: " << WSAGetLastError();
        Debug::LogError(ss.str());
        WSACleanup();
        return;
    }
    m_UDPsocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_UDPsocket == INVALID_SOCKET)
    {
        perror("Error creating UDP socket");
        WSACleanup();
        return;
    }

    // Connect to the server
    m_pServerAdress = std::make_unique<sockaddr_in>();
    m_pServerAdress->sin_family = AF_INET;
    m_pServerAdress->sin_port = htons(static_cast<u_short>(port));  //Port number of the server
    inet_pton(AF_INET, serverIp.c_str(), &m_pServerAdress->sin_addr);  //IP address of the server

    Connect();

    m_PacketReceiver = std::make_unique<EventPool<BasePacket>>(packetBuffer);
    m_PacketSender = std::make_unique<EventPool<InternalPacket>>(packetBuffer);

    m_SendThread = std::jthread{ &LeapClient::HandleSend, this };
}

leap::networking::LeapClient::~LeapClient()
{
    if (!m_Connected) return;

    // Close sockets so threads dont have blocking call on waiting
    closesocket(m_TCPsocket);
    closesocket(m_UDPsocket);

    // Awake threads
    m_Connected = false;
    m_SendThread.request_stop();
    m_TCPReceive.request_stop();
    m_UDPReceive.request_stop();
    m_SendCondition.notify_one();

    // Wait till threads are done
    m_SendThread.join();
    m_TCPReceive.join();
    m_UDPReceive.join();

    // Cleanup the sockets
    WSACleanup();
}

bool leap::networking::LeapClient::GetPacket(BasePacket& packet)
{
    BasePacket cachedPacket{};
    const auto result = m_PacketReceiver->Get(cachedPacket);
    if (result) packet.SetData(cachedPacket);
    return result;
}

void leap::networking::LeapClient::SendTCP(const BasePacket& packet)
{
    m_PacketSender->Add(InternalPacket{ packet });
    m_SendCondition.notify_one();
}

void leap::networking::LeapClient::SendUDP(const BasePacket& packet)
{
    m_PacketSender->Add(InternalPacket{ packet, true });
    m_SendCondition.notify_one();
}

void leap::networking::LeapClient::Run()
{
    Connect();
    if (!m_Connected)
    {
        Debug::LogWarning("Unable to start client. No connection to server.");
        return;
    }
    m_TCPReceive = std::jthread{ [this]() { TCPRun(); } };
    m_UDPReceive = std::jthread{ [this]() { UDPRun(); } };
}

void leap::networking::LeapClient::Connect()
{
    if (m_Connected) return;
    if (connect(m_TCPsocket, reinterpret_cast<sockaddr*>(m_pServerAdress.get()), sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        std::stringstream ss{};
        ss << "Error connecting to server: " << WSAGetLastError();
        Debug::LogWarning(ss.str());

        closesocket(m_TCPsocket);
        closesocket(m_UDPsocket);
        WSACleanup();
        m_Connected = false;
        return;
    }
    Debug::Log("Leap client is successfully connected.");
    m_Connected = true;
}

bool leap::networking::LeapClient::IsConnected()
{
    return true;
}

void leap::networking::LeapClient::TCPRun()
{
    const std::stop_token& stopToken{ m_TCPReceive.get_stop_token() };
    while (m_Connected && !stopToken.stop_requested())
    {
        m_Connected = HandleReceiveTCP();
    }
}

void leap::networking::LeapClient::UDPRun()
{
    const std::stop_token& stopToken{ m_UDPReceive.get_stop_token() };
    while (m_Connected && !stopToken.stop_requested())
    {
        HandleReceiveUDP();
    }
}

bool leap::networking::LeapClient::HandleReceiveTCP()
{
    // Receive a response from the server
    std::vector<char> buffer{};
    buffer.resize(m_BufferSize);

    const int bytesReceived = recv(m_TCPsocket, buffer.data(), m_BufferSize, 0);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0)
    {
        std::stringstream ss{};
        ss << "Error receiving TCP socket: " << WSAGetLastError();
        Debug::LogWarning(ss.str());

        closesocket(m_TCPsocket);
        WSACleanup();
        m_Connected = false;
        return false;
    }

    if (bytesReceived > m_BufferSize)
    {
        Debug::LogError("Buffer overflowed when reveiving from TCP");
    }

    // Create packet core
    std::vector<char> charBuffer{ std::begin(buffer), std::end(buffer) };

    // WARNING PACKET CREATION WILL DELETE CHAR BUFFER
    BasePacket packet{};
    packet.SetData(charBuffer);
    m_PacketReceiver->Add(packet);

    return true;
}

void leap::networking::LeapClient::HandleReceiveUDP()
{
    // Receive a response from the server
    std::vector<char> buffer{};
    buffer.resize(m_BufferSize);

    const int bytesReceived = recv(m_UDPsocket, buffer.data(), m_BufferSize, 0);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0)
    {
        return;
    }

    // Create packet core
    std::vector<char> charBuffer{ std::begin(buffer), std::end(buffer) };

    // WARNING PACKET CREATION WILL DELETE CHAR BUFFER
    BasePacket packet{};
    packet.SetData(charBuffer);
    m_PacketReceiver->Add(packet);
}

void leap::networking::LeapClient::HandleSend()
{
    const std::stop_token& stopToken{ m_SendThread.get_stop_token() };
    InternalPacket data{};

    while (!stopToken.stop_requested())
    {
        std::unique_lock lock{ m_Mutex };
        m_SendCondition.wait(lock, [&]() { return m_PacketSender->Get(data) || stopToken.stop_requested(); });
        lock.unlock();

        if (stopToken.stop_requested() || m_Connected == false) { break; }

        if (data.IsUDP)
        {
            const int bytesSentUDP = sendto(m_UDPsocket, data.Packet.GetData(), data.Packet.Length(), 0, reinterpret_cast<struct sockaddr*>(m_pServerAdress.get()), sizeof(sockaddr_in));
            if (bytesSentUDP == SOCKET_ERROR) perror("Error sending data to UDP server");
        }
        else
        {
            if (send(m_TCPsocket, data.Packet.GetData(), data.Packet.Length(), 0) == SOCKET_ERROR)
            {
                closesocket(m_TCPsocket);
                WSACleanup();
                m_Connected = false;
            }
        }
    }
}
