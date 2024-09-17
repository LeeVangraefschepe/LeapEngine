#pragma once
namespace leap::networking
{
	class BasePacket;
	class IClient
	{
	public:
		virtual ~IClient() = default;

		virtual bool GetPacket(BasePacket& packet) = 0;
		virtual void SendTCP(const BasePacket& packet) = 0;
		virtual void SendUDP(const BasePacket& packet) = 0;
		virtual void Run() = 0;
		virtual bool IsConnected() = 0;
	};
}