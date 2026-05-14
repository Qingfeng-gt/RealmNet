//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_BINARYREADER_H
#define REALMNET_BINARYREADER_H
#include <cstdint>
#include <string>
#include <cstring>

namespace RealmNet {
    class BinaryReader
    {
    public:

        BinaryReader(
            const uint8_t* data,
            size_t size)
            :
            m_data(data),
            m_size(size)
        {
        }

        template<typename T>
        T read()
        {
            T value;

            memcpy(
                &value,
                m_data + m_offset,
                sizeof(T));

            m_offset += sizeof(T);

            return value;
        }

        std::string readString()
        {
            uint32_t len =
                read<uint32_t>();

            std::string str(
                reinterpret_cast<
                    const char*>(
                    m_data + m_offset),
                len);

            m_offset += len;

            return str;
        }

    private:

        const uint8_t* m_data;

        size_t m_size;

        size_t m_offset = 0;
    };
}
#endif //REALMNET_BINARYREADER_H