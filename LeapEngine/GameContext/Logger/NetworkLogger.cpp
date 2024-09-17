#include "NetworkLogger.h"
#include "Leap/LeapPacket.h"
#include <sstream>

leap::NetworkLogger::NetworkLogger()
{
	SetEnabled(true);
    m_Client.Run();
}

leap::NetworkLogger::~NetworkLogger()
{
	Debug::OnEvent.RemoveListener(this);
}

void leap::NetworkLogger::SetEnabled(bool enable)
{
    if (m_Enabled == enable) return;

    if (enable)
    {
        Debug::OnEvent.AddListener(this, &NetworkLogger::OnDebug);
    }
    else
    {
        Debug::OnEvent.RemoveListener(this);
    }
    m_Enabled = enable;
}

void leap::NetworkLogger::OnDebug(const Debug::LogInfo& data)
{
    std::stringstream ss{};
    ss << data.Time << ' ' << data.Message << '\n';
    ss << "** " << data.Location.file_name() << " line " << data.Location.line() << ":" << data.Location.column() << '\n';

    switch (data.Type)
    {
    case Debug::Type::Message:
	    {
			networking::LeapPacket packet{0};
            packet.WriteString(ss.str());
            if (packet.Length() > MAX_BUFFER) return;
            m_Client.SendUDP(packet);
	    }
        break;
    case Debug::Type::Warning:
	    {
			networking::LeapPacket packet{1};
            packet.WriteString(ss.str());
            if (packet.Length() > MAX_BUFFER) return;
            m_Client.SendUDP(packet);
	    }
        break;
    case Debug::Type::Error:
	    {
			networking::LeapPacket packet{2};
            packet.WriteString(ss.str());
            if (packet.Length() > MAX_BUFFER) return;
            m_Client.SendUDP(packet);
	    }
        break;
    }
}
