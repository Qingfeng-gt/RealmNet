// Client main program
// Connect to server and send login request

#include <iostream>
#include <memory>

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

class ClientLogic
{
public:
    void onPacket(
        RealmNet::Connection& conn,
        RealmNet::BasePacket& packet)
    {
        if (packet.type() == LoginResponse::ID)
        {
            auto& resp = static_cast<LoginResponse&>(packet);
            std::cout << "Server message: " << resp.message << std::endl;
        }
    }
};

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

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);

#ifdef _WIN32
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
#else
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif

    int connectResult = connect(fd, (sockaddr*)&addr, sizeof(addr));

    if (connectResult == -1)
    {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    auto socket = std::make_shared<TcpSocket>(fd);
    RealmNet::Connection conn(socket);

    ClientLogic logic;

    conn.setPacketCallback(
        [&](RealmNet::Connection& c, RealmNet::BasePacket& pkt)
        {
            logic.onPacket(c, pkt);
        });

    LoginRequest req;
    req.account = "admin";
    req.password = "123456";

    std::cout << "Sending login request..." << std::endl;
    conn.sendPacket(req);

    while (true)
    {
        conn.onReadable();
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
