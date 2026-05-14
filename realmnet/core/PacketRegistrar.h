
#ifndef REALMNET_PACKETREGISTRAR_H
#define REALMNET_PACKETREGISTRAR_H
#include "PacketFactory.h"

namespace RealmNet {
    template<typename T>
    class PacketRegistrar
    {
    public:

        PacketRegistrar()
        {
            PacketFactory::instance()
                .registerPacket(
                    T::ID,
                    []()
                    {
                        return std::make_unique<T>();
                    });
        }
    };
}

#define REGISTER_PACKET(Class) \
namespace { \
    RealmNet::PacketRegistrar<Class> \
        g_registrar_##Class; \
}

#define FORCE_PACKET_REGISTRATION() \
do { \
    (void)g_registrar_##Class; \
} while(0)

#endif //REALMNET_PACKETREGISTRAR_H
