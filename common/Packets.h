// 客户端/服务端共享协议定义

#ifndef REALMNET_COMMON_PACKETS_H
#define REALMNET_COMMON_PACKETS_H

#include <string>
#include "core/BasePacket.h"
#include "core/BinaryWriter.h"
#include "core/BinaryReader.h"
#include "core/PacketRegistrar.h"

class LoginRequest : public RealmNet::BasePacket
{
    REALMNET_PACKET(LoginRequest)

    std::string account;
    std::string password;

    void serialize(RealmNet::BinaryWriter& writer) const override
    {
        writer.writeString(account);
        writer.writeString(password);
    }

    void deserialize(RealmNet::BinaryReader& reader) override
    {
        account = reader.readString();
        password = reader.readString();
    }
};
REGISTER_PACKET(LoginRequest);

class LoginResponse : public RealmNet::BasePacket
{
    REALMNET_PACKET(LoginResponse)

    bool success = false;
    std::string message;

    void serialize(RealmNet::BinaryWriter& writer) const override
    {
        writer.write(success);
        writer.writeString(message);
    }

    void deserialize(RealmNet::BinaryReader& reader) override
    {
        success = reader.read<bool>();
        message = reader.readString();
    }
};
REGISTER_PACKET(LoginResponse);

#endif
