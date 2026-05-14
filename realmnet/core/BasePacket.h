//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_BASEPACKET_H
#define REALMNET_BASEPACKET_H

#include <cstdint>

namespace RealmNet {
    class BinaryWriter;
    class BinaryReader;

    class BasePacket
    {
    public:

        using TypeID = uint32_t;

        static constexpr TypeID ID = 0;

        virtual ~BasePacket() = default;

        virtual TypeID type() const = 0;

        virtual void serialize(
            BinaryWriter& writer) const = 0;

        virtual void deserialize(
            BinaryReader& reader) = 0;
    };
}
#endif //REALMNET_BASEPACKET_H