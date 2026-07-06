// TCP 服务端封装：监听、select 轮询、客户端管理

#ifndef COMMON_SERVER_H
#define COMMON_SERVER_H

#include <iostream>
#include <memory>
#include <vector>
#include <functional>

#include "TcpSocket.h"
#include "core/Connection.h"
#include "core/PacketDispatcher.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

template <typename SocketType = TcpSocket>
class Server
{
public:
    using ConnectCallback = std::function<void(std::shared_ptr<RealmNet::IConnection>)>;
    using DisconnectCallback = std::function<void(std::shared_ptr<RealmNet::IConnection>)>;

    Server()
    {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }

    ~Server()
    {
        stop();
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

    void onClientConnected(ConnectCallback cb) { m_onConnect = std::move(cb); }
    void onClientDisconnected(DisconnectCallback cb) { m_onDisconnect = std::move(cb); }

    void setPort(uint16_t port) { m_port = port; }

    void start()
    {
        if (!createListenSocket())
            return;

        std::cout << "Server started, listening on port: " << m_port << std::endl;

        while (m_running)
        {
            poll();
        }
    }

    void stop()
    {
        m_running = false;
        if (m_listenFd != -1)
        {
#ifdef _WIN32
            closesocket(m_listenFd);
#else
            ::close(m_listenFd);
#endif
            m_listenFd = -1;
        }
    }

    RealmNet::PacketDispatcher& dispatcher() { return m_dispatcher; }

private:
    struct ClientEntry
    {
        std::shared_ptr<SocketType> socket;
        std::shared_ptr<RealmNet::IConnection> conn;
    };

    bool createListenSocket()
    {
        m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_listenFd == -1)
        {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        int opt = 1;
        setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR,
                   (const char*)&opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(m_port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(m_listenFd, (sockaddr*)&addr, sizeof(addr)) == -1)
        {
            std::cerr << "Failed to bind port: " << m_port << std::endl;
            return false;
        }

        if (listen(m_listenFd, 5) == -1)
        {
            std::cerr << "Listen failed" << std::endl;
            return false;
        }

        return true;
    }

    void poll()
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(m_listenFd, &readSet);
        int maxFd = m_listenFd;

        for (auto& client : m_clients)
        {
            FD_SET(client.socket->getFd(), &readSet);
            if (client.socket->getFd() > maxFd)
                maxFd = client.socket->getFd();
        }

        timeval timeout{ 0, 100000 };

        int result = select(maxFd + 1, &readSet, nullptr, nullptr, &timeout);
        if (result < 0) return;

        if (FD_ISSET(m_listenFd, &readSet))
            acceptClient();

        processClients(readSet);
    }

    void acceptClient()
    {
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);

        int fd = accept(m_listenFd, (sockaddr*)&clientAddr, &addrLen);
        if (fd == -1) return;

        std::cout
            << "New client connected: "
            << inet_ntoa(clientAddr.sin_addr)
            << ":" << ntohs(clientAddr.sin_port)
            << std::endl;

        auto socket = std::make_shared<SocketType>(fd);
        auto conn = std::make_shared<RealmNet::Connection<SocketType>>(socket);

        conn->setPacketCallback(
            [this](std::shared_ptr<RealmNet::IConnection> c,
                   std::shared_ptr<RealmNet::BasePacket> pkt)
            {
                m_dispatcher.dispatch(*c, *pkt);
            });

        m_clients.push_back({ socket, conn });

        if (m_onConnect)
            m_onConnect(conn);
    }

    void processClients(fd_set& readSet)
    {
        for (auto it = m_clients.begin(); it != m_clients.end();)
        {
            if (FD_ISSET(it->socket->getFd(), &readSet))
            {
                if (it->conn->onReadable())
                {
                    ++it;
                }
                else
                {
                    std::cout << "Client disconnected" << std::endl;
                    if (m_onDisconnect)
                        m_onDisconnect(it->conn);
                    it = m_clients.erase(it);
                }
            }
            else
            {
                ++it;
            }
        }
    }

    uint16_t m_port = 9000;
    int m_listenFd = -1;
    bool m_running = true;

    std::vector<ClientEntry> m_clients;
    RealmNet::PacketDispatcher m_dispatcher;

    ConnectCallback m_onConnect;
    DisconnectCallback m_onDisconnect;
};

#endif // COMMON_SERVER_H
