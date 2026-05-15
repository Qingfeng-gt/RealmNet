cmake_minimum_required(VERSION 3.20)
project(RealmNetLib)

set(CMAKE_CXX_STANDARD 17)

add_library(RealmNetLib STATIC
        core/PacketCodec.cpp
        core/Connection.cpp
        core/PacketDispatcher.cpp
        socket/ISocket.h
        core/BasePacket.h
        core/TypeHash.h
        core/BinaryWriter.h
        core/BinaryReader.h
        core/PacketFactory.h
        core/PacketRegistrar.h
        core/PacketCodec.h
        core/Connection.h
        core/PacketDispatcher.h
        core/PacketHandler.h
)

target_include_directories(RealmNetLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})