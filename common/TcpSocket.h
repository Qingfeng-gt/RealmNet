
#ifndef COMMON_TCPSOCKET_H
#define COMMON_TCPSOCKET_H

#include "socket/ISocket.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

class TcpSocket : public RealmNet::ISocket
{
public:

    explicit TcpSocket(int fd = -1);

    ~TcpSocket() override;

    bool send(
        const uint8_t* data,
        size_t len) override;

    int recv(
        uint8_t* buffer,
        size_t len) override;

    void close() override;

    int getFd() const { return m_fd; }

    void setFd(int fd) { m_fd = fd; }

private:

    int m_fd;
};

#endif //COMMON_TCPSOCKET_H
