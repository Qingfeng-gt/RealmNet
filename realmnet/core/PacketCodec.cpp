#include "PacketCodec.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include <cstring>

namespace RealmNet
{
    std::vector<uint8_t>
    PacketCodec::encode(const BasePacket& packet)
    {
        BinaryWriter writer;

        writer.write(packet.type());

        packet
            .serialize(writer);

        auto& buffer = writer.buffer();

        uint32_t size =
            static_cast<uint32_t>(
                buffer.size());

        std::vector<uint8_t> result;

        auto sizePtr =
            reinterpret_cast<
                const uint8_t*>(&size);

        result.insert(
            result.end(),
            sizePtr,
            sizePtr + sizeof(size));

        result.insert(
            result.end(),
            buffer.begin(),
            buffer.end());

        return result;
    }

    void PacketCodec::process(
        std::vector<uint8_t>& buffer,
        std::function<
            void(std::unique_ptr<BasePacket>)>
        callback)
    {
        if (buffer.size() < sizeof(uint32_t))
            return;

        uint32_t packetSize;
        memcpy(
            &packetSize,
            buffer.data(),
            sizeof(packetSize));

        if (buffer.size() < sizeof(uint32_t) + packetSize)
            return;

        const uint8_t* packetData =
            buffer.data() + sizeof(uint32_t);

        uint32_t packetType;
        memcpy(
            &packetType,
            packetData,
            sizeof(packetType));

        auto packet =
            PacketFactory::instance()
            .create(packetType);

        if (!packet)
            return;

        BinaryReader reader(
            packetData + sizeof(packetType),
            packetSize - sizeof(packetType));

        packet->deserialize(reader);

        callback(std::move(packet));
    }
}
