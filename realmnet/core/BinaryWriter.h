#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <type_traits>
#include <cstdint>

namespace RealmNet
{
    class BinaryWriter
    {
    public:

        BinaryWriter() = default;

        // =========================================
        // 基础类型
        // =========================================

        template<typename T>
        void write(const T& value)
        {
            static_assert(
                std::is_trivially_copyable_v<T>,
                "BinaryWriter::write requires trivially copyable type"
            );

            const auto* ptr =
                reinterpret_cast<const uint8_t*>(&value);

            m_buffer.insert(
                m_buffer.end(),
                ptr,
                ptr + sizeof(T));
        }

        // =========================================
        // string
        // =========================================

        void writeString(const std::string& str)
        {
            uint32_t len =
                static_cast<uint32_t>(str.size());

            write(len);

            if (!str.empty())
            {
                const auto* ptr =
                    reinterpret_cast<const uint8_t*>(str.data());

                m_buffer.insert(
                    m_buffer.end(),
                    ptr,
                    ptr + len);
            }
        }

        // =========================================
        // vector
        // =========================================

        template<typename T, typename Func>
        void writeVector(
            const std::vector<T>& vec,
            Func&& writeItem)
        {
            uint32_t size =
                static_cast<uint32_t>(vec.size());

            write(size);

            for (const auto& item : vec)
            {
                writeItem(*this, item);
            }
        }

        // =========================================
        // raw bytes
        // =========================================

        void writeBytes(
            const void* data,
            size_t size)
        {
            const auto* ptr =
                reinterpret_cast<const uint8_t*>(data);

            m_buffer.insert(
                m_buffer.end(),
                ptr,
                ptr + size);
        }

        // =========================================
        // util
        // =========================================

        void clear()
        {
            m_buffer.clear();
        }

        size_t size() const
        {
            return m_buffer.size();
        }

        const uint8_t* data() const
        {
            return m_buffer.data();
        }

        const std::vector<uint8_t>& buffer() const
        {
            return m_buffer;
        }

    private:
        std::vector<uint8_t> m_buffer;
    };
}