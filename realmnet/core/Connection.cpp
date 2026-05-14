// Connection implementation
// Manages data send and receive for a single client connection

#include "Connection.h"
#include "PacketCodec.h"
#include <iostream>
#include <cstring>

namespace RealmNet {

    Connection::Connection(
        std::shared_ptr<ISocket> socket)
        :
        m_socket(std::move(socket))
    {
    }

    bool Connection::sendPacket(
        const BasePacket& packet)
    {
        auto data = m_codec.encode(packet);

        std::cout
            << "Send packet, size: "
            << data.size()
            << " bytes"
            << std::endl;

        bool result = m_socket->send(data.data(), data.size());

        if (!result)
        {
            std::cerr << "Send failed" << std::endl;
        }

        return result;
    }

    bool Connection::onReadable()
    {
        std::vector<uint8_t> tempBuffer(4096);

        int bytes = m_socket->recv(tempBuffer.data(), tempBuffer.size());

        if (bytes <= 0)
        {
            m_connected = false;
            return false;
        }

        std::cout
            << "Received data, size: "
            << bytes
            << " bytes"
            << std::endl;

        m_recvBuffer.insert(
            m_recvBuffer.end(),
            tempBuffer.begin(),
            tempBuffer.begin() + bytes);

        while (m_recvBuffer.size() >= sizeof(uint32_t))
        {
            uint32_t packetSize;
            std::memcpy(&packetSize, m_recvBuffer.data(), sizeof(packetSize));

            if (m_recvBuffer.size() < sizeof(uint32_t) + packetSize)
                break;

            std::vector<uint8_t> packetBuffer(
                m_recvBuffer.begin(),
                m_recvBuffer.begin() + sizeof(uint32_t) + packetSize);

            m_codec.process(packetBuffer,
                [&](std::unique_ptr<BasePacket> pkt)
                {
                    std::cout
                        << "Packet received, type: "
                        << pkt->type()
                        << std::endl;

                    if (m_packetCallback)
                        m_packetCallback(*this, *pkt);
                });

            m_recvBuffer.erase(
                m_recvBuffer.begin(),
                m_recvBuffer.begin() + sizeof(uint32_t) + packetSize);
        }

        return true;
    }

    void Connection::setPacketCallback(
        PacketCallback callback)
    {
        m_packetCallback = std::move(callback);
    }

}
