#include "PacketDispatcher.h"
#include "BasePacket.h"
#include "Connection.h"

namespace RealmNet
{
    void PacketDispatcher::dispatch(
        IConnection& conn,
        BasePacket& packet)
    {
        auto it =
            m_handlers.find(
                packet.type());

        if (it != m_handlers.end())
            it->second(conn, packet);
    }
}
