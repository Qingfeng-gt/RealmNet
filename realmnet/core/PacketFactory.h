//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_PACKETFACTORY_H
#define REALMNET_PACKETFACTORY_H
#include <memory>
#include <unordered_map>
#include <functional>

#include "BasePacket.h"

namespace RealmNet {
    class PacketFactory
    {
    public:

        using Creator =
            std::function<
                std::unique_ptr<BasePacket>()>;

        static PacketFactory& instance()
        {
            static PacketFactory f;
            return f;
        }

        void registerPacket(
            BasePacket::TypeID id,
            Creator creator)
        {
            m_registry[id] =
                std::move(creator);
        }

        std::unique_ptr<BasePacket>
        create(BasePacket::TypeID id)
        {
            auto it =
                m_registry.find(id);

            if (it == m_registry.end())
                return nullptr;

            return it->second();
        }

    private:

        std::unordered_map<
            BasePacket::TypeID,
            Creator> m_registry;
    };

}
#endif //REALMNET_PACKETFACTORY_H