#pragma once
#include "../Interfaces/BasePacket.h"
#include <numeric>
#include <sstream>
#include <vector>

namespace leap::networking
{
    class LeapPacket final : public BasePacket
    {
    public:
	    explicit LeapPacket(int headerId = -1) { Write(headerId); }

        void SetData(std::vector<char>& data) override;
        char* GetData() override;
        int ReadHeaderId();

        template <typename T>
        T Read()
        {
            auto it = m_Data.begin();
            std::advance(it, sizeof(T));

            std::stringstream ss(std::accumulate(m_Data.begin(), it, std::string("")), std::ios_base::in | std::ios_base::binary);

            T value{};
            ss.read(reinterpret_cast<char*>(&value), sizeof(T));

            m_Data.erase(m_Data.begin(), it);

            return value;
        }

        void ReadString(std::string& string);

        template <typename T>
        void Write(T value)
        {
            std::stringstream ss{ std::ios::out | std::ios::binary };
            ss.write(reinterpret_cast<char*>(&value), sizeof(value));
            for (const auto data{ ss.str() }; char byte : data)
            {
                m_Data.push_back(byte);
            }
        }

        void WriteString(const std::string& string);
    };
}