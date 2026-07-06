#include "TcpSocket.h"
#include <iostream>

TcpSocket::TcpSocket(int fd) : m_fd(fd) {}
TcpSocket::~TcpSocket() { close(); }

bool TcpSocket::send(const uint8_t* data, size_t len)
{
#ifdef _WIN32
    return ::send(m_fd, (const char*)data, len, 0) > 0;
#else
    return ::send(m_fd, data, len, 0) > 0;
#endif
}

int TcpSocket::recv(uint8_t* buffer, size_t len)
{
#ifdef _WIN32
    return ::recv(m_fd, (char*)buffer, len, 0);
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
