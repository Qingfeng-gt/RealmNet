//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_PACKETDISPATCHER_H
#define REALMNET_PACKETDISPATCHER_H

#include <cstdint>
#include <unordered_map>
#include <functional>


namespace RealmNet {
    class BasePacket;
    class Connection;

    class PacketDispatcher
    {
    public:

        using Handler =
            std::function<
                void(Connection&, BasePacket&)>;

        void registerHandler(
            uint32_t type,
            Handler handler);

        void dispatch(
            Connection& conn,
            BasePacket& packet);

    private:

        std::unordered_map<
            uint32_t,
            Handler> m_handlers;
    };
}
#endif //REALMNET_PACKETDISPATCHER_H