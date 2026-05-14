//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_PACKETHANDLER_H
#define REALMNET_PACKETHANDLER_H

namespace RealmNet {
    class Connection;

    template<typename T>
    class PacketHandler {
    public:
        virtual ~PacketHandler() = default;

        virtual void handle(
            Connection& conn,
            const T& packet
        ) = 0;
    };
}

#endif //REALMNET_PACKETHANDLER_H