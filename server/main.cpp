// Server main program
// Uses select I/O multiplexing to handle multiple client connections

#include <iostream>
#include <memory>
#include <vector>

#include "core/PacketHandler.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "common/TcpSocket.h"
#include "common/Packets.h"
#include "core/Connection.h"
#include "core/PacketDispatcher.h"

class LoginHandler : public RealmNet::PacketHandler<LoginRequest>
{
public:
    void handle(RealmNet::Connection &conn, const LoginRequest &packet) override
    {
        auto& req = packet;

        std::cout
            << "Received login request, account: "
            << req.account
            << std::endl;

        LoginResponse resp;

        if (req.password == "123456")
        {
            resp.success = true;
            resp.message = "Login success";
        }
        else
        {
            resp.success = false;
            resp.message = "Wrong password";
        }

        conn.sendPacket(resp);
    }
};

struct ClientInfo
{
    std::shared_ptr<TcpSocket> socket;
    std::unique_ptr<RealmNet::Connection> conn;
};

RealmNet::PacketDispatcher g_dispatcher;

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (wsaResult != 0)
    {
        std::cerr << "WSAStartup failed: " << wsaResult << std::endl;
        return 1;
    }
#endif

    g_dispatcher.registerHandler<LoginRequest,LoginHandler>();

    int listenFd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenFd == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    int opt = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    int bindResult = bind(listenFd, (sockaddr*)&addr, sizeof(addr));

    if (bindResult == -1)
    {
        std::cerr << "Failed to bind port" << std::endl;
        return 1;
    }

    int listenResult = listen(listenFd, 5);

    if (listenResult == -1)
    {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }

    std::cout << "Server started, listening on port: 9000" << std::endl;

    std::vector<ClientInfo> clients;

    while (true)
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listenFd, &readSet);

        int maxFd = listenFd;

        for (auto& client : clients)
        {
            FD_SET(client.socket->getFd(), &readSet);
            if (client.socket->getFd() > maxFd)
                maxFd = client.socket->getFd();
        }

        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        int selectResult = select(maxFd + 1, &readSet, nullptr, nullptr, &timeout);

        if (selectResult < 0)
        {
            std::cerr << "Select failed" << std::endl;
            break;
        }

        if (FD_ISSET(listenFd, &readSet))
        {
            sockaddr_in clientAddr{};
            socklen_t clientAddrLen = sizeof(clientAddr);

            int clientFd = accept(
                listenFd,
                (sockaddr*)&clientAddr,
                &clientAddrLen);

            if (clientFd == -1)
            {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }

            std::cout
                << "New client connected: "
                << inet_ntoa(clientAddr.sin_addr)
                << ":"
                << ntohs(clientAddr.sin_port)
                << std::endl;

            ClientInfo info;
            info.socket = std::make_shared<TcpSocket>(clientFd);
            info.conn = std::make_unique<RealmNet::Connection>(info.socket);

            info.conn->setPacketCallback(
                [&](RealmNet::Connection& c, RealmNet::BasePacket& pkt)
                {
                    g_dispatcher.dispatch(c, pkt);
                });

            clients.push_back(std::move(info));
        }



        for (auto it = clients.begin(); it != clients.end();)
        {
            if (FD_ISSET(it->socket->getFd(), &readSet))
            {
                bool connected = it->conn->onReadable();
                if (!connected)
                {
                    std::cout << "Client disconnected" << std::endl;
                    it = clients.erase(it);
                }
                else
                {
                    it++;
                }
            }
            else
            {
                it++;
            }
        }
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
