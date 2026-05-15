//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_PACKETHANDLER_H
#define REALMNET_PACKETHANDLER_H

namespace RealmNet
{
    class IConnection;

    template <typename T>
    class PacketHandler
    {
    public:
        virtual ~PacketHandler() = default;

        virtual void handle(
            IConnection& conn,
            const T& packet
        ) = 0;
    };
}

#endif //REALMNET_PACKETHANDLER_H
