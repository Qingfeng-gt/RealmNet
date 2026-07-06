#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <cstdint>

namespace RealmNet
{
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

        // =========================================
        // 基础类型
        // =========================================

        template<typename T>
        T read()
        {
            static_assert(
                std::is_trivially_copyable_v<T>,
                "BinaryReader::read requires trivially copyable type"
            );

            checkRead(sizeof(T));

            T value;

            memcpy(
                &value,
                m_data + m_offset,
                sizeof(T));

            m_offset += sizeof(T);

            return value;
        }

        // =========================================
        // string
        // =========================================

        std::string readString()
        {
            uint32_t len =
                read<uint32_t>();

            checkRead(len);

            std::string str(
                reinterpret_cast<const char*>(
                    m_data + m_offset),
                len);

            m_offset += len;

            return str;
        }

        // =========================================
        // vector
        // =========================================

        template<typename T, typename Func>
        std::vector<T> readVector(Func&& readItem)
        {
            uint32_t size =
                read<uint32_t>();

            std::vector<T> vec;
            vec.reserve(size);

            for (uint32_t i = 0; i < size; ++i)
            {
                vec.push_back(
                    readItem(*this));
            }

            return vec;
        }

        // =========================================
        // raw bytes
        // =========================================

        void readBytes(
            void* dst,
            size_t size)
        {
            checkRead(size);

            memcpy(
                dst,
                m_data + m_offset,
                size);

            m_offset += size;
        }

        // =========================================
        // util
        // =========================================

        bool hasMore() const
        {
            return m_offset < m_size;
        }

        size_t remain() const
        {
            return m_size - m_offset;
        }

        size_t offset() const
        {
            return m_offset;
        }

        void skip(size_t size)
        {
            checkRead(size);
            m_offset += size;
        }

    private:

        void checkRead(size_t size)
        {
            if (m_offset + size > m_size)
            {
                throw std::runtime_error(
                    "BinaryReader buffer overflow");
            }
        }

    private:

        const uint8_t* m_data = nullptr;

        size_t m_size = 0;

        size_t m_offset = 0;
    };
}