//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_PACKETDISPATCHER_H
#define REALMNET_PACKETDISPATCHER_H

#include <cstdint>
#include <unordered_map>
#include <functional>

#include "Connection.h"


namespace RealmNet
{
    class BasePacket;
    class IConnection;

    class PacketDispatcher
    {
    public:
        template <typename PacketT, typename HandlerT>
        void registerHandler()
        {
            m_handlers[PacketT::ID] = [](IConnection& conn,
                                         BasePacket& packet)
            {
                HandlerT handler;

                handler.handle(
                    conn,
                    static_cast<PacketT&>(
                        packet
                    )
                );
            };
        }

        template <typename PacketT>
        void registerHandler(std::function<void(IConnection&, PacketT&)> handlerFn)
        {
            m_handlers[PacketT::ID] = [fn = std::move(handlerFn)](IConnection& conn, BasePacket& packet)
            {
                fn(conn, static_cast<PacketT&>(packet));
            };
        }


        void dispatch(
            IConnection& conn,
            BasePacket& packet);

    private:
        using Handler = std::function<void(IConnection&, BasePacket&)>;
        std::unordered_map<uint32_t,Handler> m_handlers;
    };
}
#endif //REALMNET_PACKETDISPATCHER_H
