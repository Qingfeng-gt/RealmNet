//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_PACKETCODEC_H
#define REALMNET_PACKETCODEC_H
#include <vector>
#include <functional>

#include "PacketFactory.h"

namespace RealmNet
{
    class PacketCodec
    {
    public:
        std::vector<uint8_t>
        encode(const BasePacket& packet);

        void process(
            std::vector<uint8_t>& buffer,
            std::function<
                void(std::unique_ptr<BasePacket>)>
            callback);
    };
}
#endif //REALMNET_PACKETCODEC_H
