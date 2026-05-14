//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_CONNECTION_H
#define REALMNET_CONNECTION_H
#include <memory>
#include <vector>
#include <functional>

#include "PacketCodec.h"
#include "../socket/ISocket.h"

namespace RealmNet {
    class Connection
    {
    public:

        using PacketCallback =
            std::function<
                void(Connection&, BasePacket&)>;

        explicit Connection(
            std::shared_ptr<ISocket> socket);

        bool sendPacket(
            const BasePacket& packet);

        bool onReadable();

        void setPacketCallback(
            PacketCallback callback);

        bool isConnected() const { return m_connected; }

    private:

        std::shared_ptr<ISocket> m_socket;

        PacketCodec m_codec;

        std::vector<uint8_t> m_recvBuffer;

        PacketCallback m_packetCallback;

        bool m_connected = true;
    };
}
#endif //REALMNET_CONNECTION_H