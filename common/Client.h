// TCP 客户端封装：连接、收发、消息路由

#ifndef COMMON_CLIENT_H
#define COMMON_CLIENT_H

#include <iostream>
#include <memory>
#include <string>

#include "TcpSocket.h"
#include "core/Connection.h"
#include "core/PacketDispatcher.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

template <typename SocketType = TcpSocket>
class Client
{
public:
    Client()
    {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }

    ~Client()
    {
        disconnect();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    // 注册 Handler 类
    template <typename PacketT, typename HandlerT>
    void registerHandler()
    {
        m_dispatcher.registerHandler<PacketT, HandlerT>();
    }

    // 注册 lambda
    template <typename PacketT>
    void registerHandler(std::function<void(RealmNet::IConnection&, PacketT&)> handler)
    {
        m_dispatcher.registerHandler<PacketT>(std::move(handler));
    }

    bool connect(const std::string& host, uint16_t port)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

        if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) == -1)
        {
            std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
            return false;
        }

        m_socket = std::make_shared<SocketType>(fd);
        m_conn.reset(new RealmNet::Connection<SocketType>(m_socket));

        m_conn->setPacketCallback(
            [this](std::shared_ptr<RealmNet::IConnection> c,
                   std::shared_ptr<RealmNet::BasePacket> pkt)
            {
                m_dispatcher.dispatch(*c, *pkt);
            });

        std::cout << "Connected to " << host << ":" << port << std::endl;
        return true;
    }

    bool send(const RealmNet::BasePacket& packet)
    {
        if (!m_conn) return false;
        return m_conn->sendPacket(packet);
    }

    // 阻塞接收循环
    void run()
    {
        while (m_conn && m_conn->isConnected())
            m_conn->onReadable();
    }

    // 单次接收（供外部轮询用）
    void tick()
    {
        if (m_conn && m_conn->isConnected())
            m_conn->onReadable();
    }

    void disconnect()
    {
        m_conn.reset();
        m_socket.reset();
    }

    bool isConnected() const { return m_conn && m_conn->isConnected(); }

    std::shared_ptr<RealmNet::IConnection> connection() const { return m_conn; }

    RealmNet::PacketDispatcher& dispatcher() { return m_dispatcher; }

private:
    std::shared_ptr<SocketType> m_socket;
    std::shared_ptr<RealmNet::IConnection> m_conn;
    RealmNet::PacketDispatcher m_dispatcher;
};

#endif // COMMON_CLIENT_H
