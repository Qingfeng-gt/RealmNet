//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_CONNECTION_H
#define REALMNET_CONNECTION_H
#include <cstring>
#include <memory>
#include <vector>
#include <functional>
#include <iostream>

#include "PacketCodec.h"
#include "../socket/ISocket.h"

namespace RealmNet
{   
    //连接接口
    class IConnection 
    {
    public:
        using PacketCallback = std::function<void(std::shared_ptr<IConnection>,std::shared_ptr<RealmNet::BasePacket>)>;
        
        virtual ~IConnection() = default;
        virtual bool sendPacket(const BasePacket& packet) = 0;
        virtual bool onReadable() = 0;
        virtual void setPacketCallback(PacketCallback callback) = 0;
        virtual bool isConnected() const = 0;
        virtual std::shared_ptr<ISocket> socket() const = 0;
    };
    
    template <typename SocketType = ISocket>
    class Connection : public IConnection,public std::enable_shared_from_this<IConnection>
    {
    public:
        explicit Connection(std::shared_ptr<SocketType> socket)
            :
            m_socket(std::move(socket))
        {
        }
        
        bool sendPacket(const BasePacket& packet) override
        {
            auto data = m_codec.encode(packet);

            std::cout   
                << "Send packet, size: "
                << data.size()
                << " bytes"
                << "Packet type: "
                << packet.typeName() << "("<< packet.type() << ")"
                << std::endl;

            bool result = m_socket->send(data.data(), data.size());

            if (!result)
            {
                std::cerr << "Send failed" << std::endl;
            }

            return result;
        }

        bool onReadable() override
        {
            std::vector<uint8_t> tempBuffer(4096);

            int bytes = m_socket->recv(tempBuffer.data(), tempBuffer.size());

            if (bytes <= 0)
            {
                m_connected = false;
                return false;
            }

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
                                        << "Received data, size: "
                                        << bytes
                                        << " bytes"
                                        << "Packet type:"
                                        << pkt->typeName() << "("<< pkt->type() << ")"
                                        << std::endl;
                                    if (m_packetCallback)
                                        m_packetCallback(shared_from_this(), std::move(pkt));
                                });

                m_recvBuffer.erase(
                    m_recvBuffer.begin(),
                    m_recvBuffer.begin() + sizeof(uint32_t) + packetSize);
            }

            return true;
        }

        void setPacketCallback(IConnection::PacketCallback callback) override
        {
            m_packetCallback = std::move(callback);
        }

        bool isConnected() const override
        {
            return m_connected;
        }

        std::shared_ptr<ISocket> socket() const override
        {
            return m_socket;
        }
        
        virtual std::shared_ptr<SocketType> typedSocket() const
        {
            return m_socket;
        }
    private:
        std::shared_ptr<SocketType> m_socket;

        PacketCodec m_codec;

        std::vector<uint8_t> m_recvBuffer;

        IConnection::PacketCallback m_packetCallback;

        bool m_connected = true;
    };
}
#endif //REALMNET_CONNECTION_H
