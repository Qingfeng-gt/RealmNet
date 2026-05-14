//
// Created by Administrator on 2026/5/14.
//

#ifndef REALMNET_TYPEHASH_H
#define REALMNET_TYPEHASH_H
#include <cstdint>

namespace RealmNet {
    constexpr uint32_t fnv1a(
        const char* str,
        uint32_t hash = 2166136261u)
    {
        return (*str)
            ? fnv1a(
                str + 1,
                (hash ^ uint32_t(*str))
                * 16777619u)
            : hash;
    }
}
#endif //REALMNET_TYPEHASH_H