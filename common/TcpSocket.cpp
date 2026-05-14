// TcpSocket implementation
// Windows socket wrapper for TCP connections

#include "TcpSocket.h"
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

TcpSocket::TcpSocket(int fd)
    : m_fd(fd)
{
}

TcpSocket::~TcpSocket()
{
    close();
}

bool TcpSocket::send(const uint8_t* data, size_t len)
{
#ifdef _WIN32
    int result = ::send(m_fd, (const char*)data, len, 0);

    std::cout << "TcpSocket::send, len=" << len << ", result=" << result << std::endl;

    return result > 0;
#else
    return ::send(m_fd, data, len, 0) > 0;
#endif
}

int TcpSocket::recv(uint8_t* buffer, size_t len)
{
#ifdef _WIN32
    int result = ::recv(m_fd, (char*)buffer, len, 0);

    std::cout << "TcpSocket::recv, len=" << len << ", result=" << result << std::endl;

    return result;
#else
    return ::recv(m_fd, buffer, len, 0);
#endif
}

void TcpSocket::close()
{
    if (m_fd != -1)
    {
#ifdef _WIN32
        closesocket(m_fd);
#else
        ::close(m_fd);
#endif
        m_fd = -1;
    }
}
