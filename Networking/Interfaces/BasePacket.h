#pragma once
#include <vector>

namespace leap::networking
{
    class BasePacket
    {
    public:
        virtual char* GetData(){ return m_Data.data(); }
        virtual void SetData(std::vector<char>& data) { m_Data = std::move(data); }
        void SetData(BasePacket& other) { SetData(other.m_Data); }
        virtual int Length() const { return static_cast<int>(m_Data.size()); }

        virtual ~BasePacket() = default;
    protected:
        std::vector<char> m_Data{};
    };
}