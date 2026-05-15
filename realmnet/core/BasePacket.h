//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_BASEPACKET_H
#define REALMNET_BASEPACKET_H

#include <cstdint>
#include <string>
#include <string_view>

#include "TypeHash.h"

namespace RealmNet
{
    class BinaryWriter;
    class BinaryReader;

    class BasePacket
    {
    public:
        using TypeID = uint32_t;
        static constexpr std::string_view TypeName = "BasePacket";
        static constexpr TypeID ID = RealmNet::fnv1a(TypeName.data());
        virtual TypeID type() const
        {
            return ID;
        }
        virtual std::string typeName() const
        {
            return TypeName.data();
        }
        
        virtual ~BasePacket() = default;
        virtual void serialize(
            BinaryWriter& writer) const = 0;

        virtual void deserialize(
            BinaryReader& reader) = 0;
    };
}

#define REALMNET_PACKET(name) \
    public : \
        static constexpr std::string_view TypeName = #name; \
        static constexpr TypeID ID = RealmNet::fnv1a(TypeName.data()); \
        TypeID type() const override \
        { \
            return ID; \
        } \
        std::string typeName() const override \
        { \
            return TypeName.data(); \
        } 

#endif //REALMNET_BASEPACKET_H
