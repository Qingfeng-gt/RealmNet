#ifndef REALMNET_COMMON_PACKETS_H
#define REALMNET_COMMON_PACKETS_H

#include <string>
#include "core/BasePacket.h"
#include "core/BinaryWriter.h"
#include "core/BinaryReader.h"
#include "core/TypeHash.h"
#include "core/PacketRegistrar.h"

class LoginRequest : public RealmNet::BasePacket
{
public:

    static constexpr TypeID ID =
        RealmNet::fnv1a("LoginRequest");

    std::string account;

    std::string password;

    TypeID type() const override
    {
        return ID;
    }

    void serialize(
        RealmNet::BinaryWriter& writer) const override
    {
        writer.writeString(account);
        writer.writeString(password);
    }

    void deserialize(
        RealmNet::BinaryReader& reader) override
    {
        account =
            reader.readString();

        password =
            reader.readString();
    }
};
REGISTER_PACKET(LoginRequest);

class LoginResponse : public RealmNet::BasePacket
{
public:

    static constexpr TypeID ID =
        RealmNet::fnv1a("LoginResponse");

    bool success = false;

    std::string message;

    TypeID type() const override
    {
        return ID;
    }

    void serialize(
        RealmNet::BinaryWriter& writer) const override
    {
        writer.write(success);

        writer.writeString(message);
    }

    void deserialize(
        RealmNet::BinaryReader& reader) override
    {
        success =
            reader.read<bool>();

        message =
            reader.readString();
    }
};
REGISTER_PACKET(LoginResponse);


#endif //REALMNET_COMMON_PACKETS_H
