//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_ISOCKET_H
#define REALMNET_ISOCKET_H

#include <cstdint>

namespace RealmNet
{
    class ISocket
    {
    public:
        virtual ~ISocket() = default;

        virtual bool send(
            const uint8_t* data,
            size_t len) = 0;

        virtual int recv(
            uint8_t* buffer,
            size_t len) = 0;

        virtual void close() = 0;
    };
}

#endif //REALMNET_ISOCKET_H
