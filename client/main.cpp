#include <iostream>

#include "common/Client.h"
#include "common/Packets.h"
#include "core/PacketHandler.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    Client<TcpSocket> client;

    client.registerHandler<LoginResponse>(
        [](RealmNet::IConnection& conn, LoginResponse& packet)
        {
            std::cout << "服务器: " << packet.message << std::endl;
        });

    if (!client.connect("127.0.0.1", 9000))
        return 1;

    LoginRequest req;
    req.account = "admin";
    req.password = "123456";

    std::cout << "发送登录..." << std::endl;
    client.send(req);

    client.run();
    return 0;
}
