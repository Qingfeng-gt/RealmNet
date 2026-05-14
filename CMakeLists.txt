cmake_minimum_required(VERSION 3.20)
project(RealmNet)

set(CMAKE_CXX_STANDARD 20)

# 静态链接运行时库，避免依赖外部 DLL
if(MINGW)
    # 完全静态链接
    add_link_options(-static)
    add_compile_options(-static-libgcc -static-libstdc++)
    add_link_options(-static-libgcc -static-libstdc++ -Wl,-Bstatic)
elseif(MSVC)
    # MSVC 静态链接运行时库
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(realmnet)

add_executable(RealmNetServer 
    server/main.cpp 
    common/TcpSocket.cpp
)
target_link_libraries(RealmNetServer PRIVATE RealmNetLib)
target_include_directories(RealmNetServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(RealmNetClient 
    client/main.cpp 
    common/TcpSocket.cpp
)
target_link_libraries(RealmNetClient PRIVATE RealmNetLib)
target_include_directories(RealmNetClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    target_link_libraries(RealmNetServer PRIVATE ws2_32)
    target_link_libraries(RealmNetClient PRIVATE ws2_32)
endif()
