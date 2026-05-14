
#include "PacketDispatcher.h"
#include "BasePacket.h"
#include "Connection.h"

namespace RealmNet {

    void PacketDispatcher::registerHandler(
        uint32_t type,
        Handler handler)
    {
        m_handlers[type] =
            std::move(handler);
    }

    void PacketDispatcher::dispatch(
        Connection& conn,
        BasePacket& packet)
    {
        auto it =
            m_handlers.find(
                packet.type());

        if (it != m_handlers.end())
            it->second(conn, packet);
    }

}
