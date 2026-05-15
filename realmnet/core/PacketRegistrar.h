#ifndef REALMNET_PACKETREGISTRAR_H
#define REALMNET_PACKETREGISTRAR_H
#include "PacketFactory.h"

namespace RealmNet
{
    template <typename T>
    class PacketRegistrar
    {
    public:
        PacketRegistrar()
        {
            PacketFactory::instance()
                .registerPacket<T>();
        }
    };
}

#define REGISTER_PACKET(Class) \
namespace { \
    RealmNet::PacketRegistrar<Class> \
        g_registrar_##Class; \
}


#endif //REALMNET_PACKETREGISTRAR_H
