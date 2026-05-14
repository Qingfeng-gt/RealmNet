//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_BINARYWRITER_H
#define REALMNET_BINARYWRITER_H
#include <cstdint>
#include <vector>
#include <string>

namespace RealmNet {
    class BinaryWriter
    {
    public:

        template<typename T>
        void write(const T& value)
        {
            const uint8_t* ptr =
                reinterpret_cast<
                    const uint8_t*>(&value);

            m_buffer.insert(
                m_buffer.end(),
                ptr,
                ptr + sizeof(T));
        }

        void writeString(
            const std::string& str)
        {
            uint32_t len =
                static_cast<uint32_t>(
                    str.size());

            write(len);

            m_buffer.insert(
                m_buffer.end(),
                str.begin(),
                str.end());
        }

        const std::vector<uint8_t>&
        buffer() const
        {
            return m_buffer;
        }

    private:

        std::vector<uint8_t> m_buffer;
    };
}

#endif //REALMNET_BINARYWRITER_H