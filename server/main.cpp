#include <iostream>

#include "common/Server.h"
#include "common/Packets.h"
#include "core/PacketHandler.h"

#ifdef _WIN32
#include <windows.h>
#endif

class LoginHandler : public RealmNet::PacketHandler<RealmNet::IConnection&, LoginRequest&>
{
public:
    void handle(RealmNet::IConnection& conn, LoginRequest& packet) override
    {
        std::cout << "登录请求，账号: " << packet.account << std::endl;

        LoginResponse resp;
        if (packet.password == "123456")
        {
            resp.success = true;
            resp.message = "ok";
        }
        else
        {
            resp.success = false;
            resp.message = "密码错误";
        }
        conn.sendPacket(resp);
    }
};

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    Server<TcpSocket> server;
    server.setPort(9000);

    // server.registerHandler<LoginRequest, LoginHandler>();
    server.registerHandler<LoginRequest>(
        [](RealmNet::IConnection& conn, LoginRequest& packet)
        {
            std::cout << "登录请求，账号: " << packet.account << std::endl;

            LoginResponse resp;
            resp.success = (packet.password == "123456");
            resp.message = resp.success ? "ok" : "密码错误";
            conn.sendPacket(resp);
        });

    server.start();
    return 0;
}
