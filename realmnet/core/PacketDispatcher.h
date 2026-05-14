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


        template<typename PacketT, typename HandlerT>
        void registerHandler()
        {
            m_handlers[PacketT::ID] = [](Connection& conn,
               BasePacket& packet) {

                HandlerT handler;

                handler.handle(
                    conn,
                    static_cast<PacketT&>(
                        packet
                    )
                );
            };
        }

        void dispatch(
            Connection& conn,
            BasePacket& packet);

    private:
        using Handler =
                    std::function<
                        void(Connection&, BasePacket&)>;
        std::unordered_map<
            uint32_t,
            Handler> m_handlers;
    };
}
#endif //REALMNET_PACKETDISPATCHER_H