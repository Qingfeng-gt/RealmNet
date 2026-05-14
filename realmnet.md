# 工业级 C++ 网络通信框架设计文档

---

# 第一部分：框架整体设计

---

# 一、项目目标

本框架实现：

- 平台无关
- 网络无关
- 业务无关
- Socket 后端可插拔
- 客户端/服务端共享协议
- 无 switch / 无 enum
- 自动协议注册
- 可扩展协议系统

适用于：

- MMO 游戏
- 实时对战
- RPC
- 网关服务
- 微服务
- 长连接系统

---

# 二、整体架构

```text
┌────────────────────┐
│   Business Layer   │
├────────────────────┤
│ PacketDispatcher   │
├────────────────────┤
│   PacketFactory    │
├────────────────────┤
│    PacketCodec     │
├────────────────────┤
│     Connection     │
├────────────────────┤
│      ISocket       │
├────────────────────┤
│ BSD / epoll / iocp │
└────────────────────┘
```

---

# 三、设计思想

传统网络设计：

```cpp
switch(packetType)
{
    case LOGIN:
        ...
        break;
}
```

存在：

- 高耦合
- 不可扩展
- 难维护
- 中央逻辑膨胀

---

本框架采用：

```text
Packet
    ↓
Codec
    ↓
Factory
    ↓
Dispatcher
    ↓
Handler
```

实现：

- 协议与业务解耦
- 网络与协议解耦
- 网络后端可替换
- Packet 只负责数据

---

# 四、目录结构

```text
NetFramework/
│
├── common/
│   │
│   ├── core/
│   │   ├── BasePacket.h
│   │   ├── BinaryReader.h
│   │   ├── BinaryWriter.h
│   │   ├── Connection.h
│   │   ├── PacketCodec.h
│   │   ├── PacketDispatcher.h
│   │   ├── PacketFactory.h
│   │   ├── PacketHeader.h
│   │   ├── PacketRegistrar.h
│   │   └── TypeHash.h
│   │
│   ├── socket/
│   │   ├── ISocket.h
│   │   ├── TcpSocket.h
│   │   └── TcpSocket.cpp
│   │
│   └── protocol/
│       ├── LoginRequest.h
│       └── LoginResponse.h
│
├── server/
│   ├── handler/
│   │   └── LoginHandler.h
│   └── main.cpp
│
└── client/
    ├── ClientLogic.h
    └── main.cpp
```

---

# 五、公共模块职责

## common/core

负责：

- 协议系统
- 编解码
- Dispatcher
- Connection
- PacketFactory

---

## common/socket

负责：

- 网络传输抽象
- Socket 后端实现

---

## common/protocol

负责：

- 客户端服务端共享协议

---

# 六、客户端与服务端职责划分

## 服务端负责

- 登录验证
- 数据库存储
- 玩家管理
- 世界逻辑

---

## 客户端负责

- UI
- 输入
- 动画
- 本地状态

---

# 七、核心原则

## Packet 只负责数据

错误：

```cpp
class LoginPacket
{
    void handle(GameSession&);
};
```

正确：

```cpp
class LoginPacket
{
    serialize();
    deserialize();
};
```

---

## Dispatcher 才负责业务

业务逻辑：

```text
由 Handler 处理
```

不是 Packet。

---

# 八、TypeHash

## common/core/TypeHash.h

```cpp
#pragma once
#include <cstdint>

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
```

---

# 九、BasePacket

## common/core/BasePacket.h

```cpp
#pragma once

#include <cstdint>

class BinaryWriter;
class BinaryReader;

class BasePacket
{
public:

    using TypeID = uint32_t;

    virtual ~BasePacket() = default;

    virtual TypeID type() const = 0;

    virtual void serialize(
        BinaryWriter& writer) const = 0;

    virtual void deserialize(
        BinaryReader& reader) = 0;
};
```

---

# 十、BinaryWriter

## common/core/BinaryWriter.h

```cpp
#pragma once

#include <vector>
#include <string>

class BinaryWriter
{
public:

    template<typename T>
    void write(const T& value)
    {
        const uint8_t* ptr =
            reinterpret_cast<
                const uint8_t*>(&value);

        m_buffer.insert(
            m_buffer.end(),
            ptr,
            ptr + sizeof(T));
    }

    void writeString(
        const std::string& str)
    {
        uint32_t len =
            static_cast<uint32_t>(
                str.size());

        write(len);

        m_buffer.insert(
            m_buffer.end(),
            str.begin(),
            str.end());
    }

    const std::vector<uint8_t>&
    buffer() const
    {
        return m_buffer;
    }

private:

    std::vector<uint8_t> m_buffer;
};
```

---

# 十一、BinaryReader

## common/core/BinaryReader.h

```cpp
#pragma once

#include <string>
#include <cstring>

class BinaryReader
{
public:

    BinaryReader(
        const uint8_t* data,
        size_t size)
        :
        m_data(data),
        m_size(size)
    {
    }

    template<typename T>
    T read()
    {
        T value;

        memcpy(
            &value,
            m_data + m_offset,
            sizeof(T));

        m_offset += sizeof(T);

        return value;
    }

    std::string readString()
    {
        uint32_t len =
            read<uint32_t>();

        std::string str(
            reinterpret_cast<
                const char*>(
                m_data + m_offset),
            len);

        m_offset += len;

        return str;
    }

private:

    const uint8_t* m_data;

    size_t m_size;

    size_t m_offset = 0;
};
```

---

# 十二、PacketFactory

## common/core/PacketFactory.h

```cpp
#pragma once

#include <memory>
#include <unordered_map>
#include <functional>

#include "BasePacket.h"

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
```

---

# 十三、PacketRegistrar

## common/core/PacketRegistrar.h

```cpp
#pragma once

#include "PacketFactory.h"

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

#define REGISTER_PACKET(Class) \
    static PacketRegistrar<Class> \
        g_registrar_##Class;
```

---

# 十四、ISocket

## common/socket/ISocket.h

```cpp
#pragma once

#include <cstdint>

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
```

---

# 十五、PacketCodec

## common/core/PacketCodec.h

```cpp
#pragma once

#include <vector>
#include <functional>

#include "PacketFactory.h"

class PacketCodec
{
public:

    std::vector<uint8_t>
    encode(const BasePacket& packet);

    void process(
        std::vector<uint8_t>& buffer,
        std::function<
            void(std::unique_ptr<BasePacket>)>
            callback);
};
```

---

# 十六、Connection

## common/core/Connection.h

```cpp
#pragma once

#include <memory>
#include <vector>

#include "ISocket.h"
#include "PacketCodec.h"

class Connection
{
public:

    explicit Connection(
        std::shared_ptr<ISocket> socket);

    bool sendPacket(
        const BasePacket& packet);

    void onReadable();

private:

    std::shared_ptr<ISocket> m_socket;

    PacketCodec m_codec;

    std::vector<uint8_t> m_recvBuffer;
};
```

---

# 十七、PacketDispatcher

## common/core/PacketDispatcher.h

```cpp
#pragma once

#include <unordered_map>
#include <functional>

class PacketDispatcher
{
public:

    using Handler =
        std::function<
            void(Connection&, BasePacket&)>;

    void registerHandler(
        uint32_t type,
        Handler handler);

    void dispatch(
        Connection& conn,
        BasePacket& packet);

private:

    std::unordered_map<
        uint32_t,
        Handler> m_handlers;
};
```

---

# 第二部分：基于 BSD Socket 的完整客户端/服务端示例

---

# 一、完整通信流程

```text
Client
    ↓
LoginRequest
    ↓
PacketCodec.encode
    ↓
TcpSocket.send
    ↓
Server recv
    ↓
PacketCodec.decode
    ↓
PacketFactory.create
    ↓
PacketDispatcher
    ↓
LoginHandler
    ↓
LoginResponse
    ↓
Client recv
```

---

# 二、TcpSocket 实现

## common/socket/TcpSocket.h

```cpp
#pragma once

#include "ISocket.h"

class TcpSocket : public ISocket
{
public:

    explicit TcpSocket(int fd);

    bool send(
        const uint8_t* data,
        size_t len) override;

    int recv(
        uint8_t* buffer,
        size_t len) override;

    void close() override;

private:

    int m_fd;
};
```

---

## common/socket/TcpSocket.cpp

```cpp
#include "TcpSocket.h"

#include <unistd.h>
#include <sys/socket.h>

TcpSocket::TcpSocket(int fd)
    :
    m_fd(fd)
{
}

bool TcpSocket::send(
    const uint8_t* data,
    size_t len)
{
    return ::send(
        m_fd,
        data,
        len,
        0) > 0;
}

int TcpSocket::recv(
    uint8_t* buffer,
    size_t len)
{
    return ::recv(
        m_fd,
        buffer,
        len,
        0);
}

void TcpSocket::close()
{
    ::close(m_fd);
}
```

---

# 三、共享协议

## LoginRequest.h

```cpp
#pragma once

#include "../core/BasePacket.h"
#include "../core/PacketRegistrar.h"
#include "../core/TypeHash.h"

class LoginRequest : public BasePacket
{
public:

    static constexpr TypeID ID =
        fnv1a("LoginRequest");

    std::string account;

    std::string password;

    TypeID type() const override
    {
        return ID;
    }

    void serialize(
        BinaryWriter& writer) const override
    {
        writer.writeString(account);
        writer.writeString(password);
    }

    void deserialize(
        BinaryReader& reader) override
    {
        account =
            reader.readString();

        password =
            reader.readString();
    }
};

REGISTER_PACKET(LoginRequest)
```

---

## LoginResponse.h

```cpp
#pragma once

class LoginResponse : public BasePacket
{
public:

    static constexpr TypeID ID =
        fnv1a("LoginResponse");

    bool success = false;

    std::string message;

    TypeID type() const override
    {
        return ID;
    }

    void serialize(
        BinaryWriter& writer) const override
    {
        writer.write(success);

        writer.writeString(message);
    }

    void deserialize(
        BinaryReader& reader) override
    {
        success =
            reader.read<bool>();

        message =
            reader.readString();
    }
};

REGISTER_PACKET(LoginResponse)
```

---

# 四、服务端实现

## server/handler/LoginHandler.h

```cpp
#pragma once

#include "../../common/protocol/LoginRequest.h"
#include "../../common/protocol/LoginResponse.h"

class LoginHandler
{
public:

    void operator()(
        Connection& conn,
        BasePacket& packet)
    {
        auto& req =
            static_cast<LoginRequest&>(
                packet);

        std::cout
            << "client login: "
            << req.account
            << std::endl;

        LoginResponse resp;

        if (req.password == "123456")
        {
            resp.success = true;
            resp.message = "login success";
        }
        else
        {
            resp.success = false;
            resp.message = "wrong password";
        }

        conn.sendPacket(resp);
    }
};
```

---

## server/main.cpp

```cpp
#include <iostream>
#include <memory>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../common/socket/TcpSocket.h"

#include "../common/core/Connection.h"
#include "../common/core/PacketDispatcher.h"

#include "handler/LoginHandler.h"

int main()
{
    int listenFd =
        socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};

    addr.sin_family = AF_INET;

    addr.sin_port = htons(9000);

    addr.sin_addr.s_addr = INADDR_ANY;

    bind(
        listenFd,
        (sockaddr*)&addr,
        sizeof(addr));

    listen(listenFd, 5);

    std::cout
        << "server start..."
        << std::endl;

    int clientFd =
        accept(listenFd, nullptr, nullptr);

    auto socket =
        std::make_shared<TcpSocket>(
            clientFd);

    Connection conn(socket);

    PacketDispatcher dispatcher;

    dispatcher.registerHandler(
        LoginRequest::ID,
        LoginHandler());

    conn.setPacketCallback(
        [&](Connection& c, BasePacket& pkt)
        {
            dispatcher.dispatch(c, pkt);
        });

    while (true)
    {
        conn.onReadable();
    }

    return 0;
}
```

---

# 五、客户端实现

## client/ClientLogic.h

```cpp
#pragma once

#include <iostream>

#include "../common/protocol/LoginResponse.h"

class ClientLogic
{
public:

    void onPacket(
        Connection& conn,
        BasePacket& packet)
    {
        if (packet.type()
            == LoginResponse::ID)
        {
            auto& resp =
                static_cast<LoginResponse&>(
                    packet);

            std::cout
                << "server message: "
                << resp.message
                << std::endl;
        }
    }
};
```

---

## client/main.cpp

```cpp
#include <iostream>
#include <memory>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../common/socket/TcpSocket.h"

#include "../common/core/Connection.h"

#include "../common/protocol/LoginRequest.h"

#include "ClientLogic.h"

int main()
{
    int fd =
        socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};

    addr.sin_family = AF_INET;

    addr.sin_port = htons(9000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &addr.sin_addr);

    connect(
        fd,
        (sockaddr*)&addr,
        sizeof(addr));

    auto socket =
        std::make_shared<TcpSocket>(fd);

    Connection conn(socket);

    ClientLogic logic;

    conn.setPacketCallback(
        [&](Connection& c, BasePacket& pkt)
        {
            logic.onPacket(c, pkt);
        });

    LoginRequest req;

    req.account = "admin";

    req.password = "123456";

    conn.sendPacket(req);

    while (true)
    {
        conn.onReadable();
    }

    return 0;
}
```

---

# 六、执行结果

启动服务端：

```text
./server
```

输出：

```text
server start...
```

---

启动客户端：

```text
./client
```

服务端输出：

```text
client login: admin
```

客户端输出：

```text
server message: login success
```

---

# 第三部分：后续优化方向

---

# 一、epoll / IOCP

当前：

```cpp
while(true)
{
    recv();
}
```

后续升级：

- epoll
- kqueue
- IOCP

实现高并发事件驱动。

---

# 二、多线程

拆分：

## 网络线程

负责：

- recv
- send

---

## 逻辑线程

负责：

- Dispatcher
- Handler

---

# 三、内存池

优化：

- PacketPool
- BufferPool

减少：

- new/delete
- 内存碎片

---

# 四、protobuf / flatbuffers

替换：

```cpp
serialize()
deserialize()
```

实现：

- 自动代码生成
- 跨语言
- 版本兼容

---

# 五、压缩与加密

增加：

- zstd
- lz4
- aes
- rsa

---

# 六、RPC 支持

增加：

- requestId
- callback
- future/promise

即可演进为 RPC 框架。

---

# 七、Actor / ECS

Dispatcher 后续可升级：

- Actor
- ECS

适用于 MMO。

---

# 八、协程化

支持：

- asio coroutine
- C++20 coroutine

---

# 九、最终方向

最终可演进为：

```text
工业级 MMO 网络引擎
```

核心思想始终不变：

```text
Socket 只负责传输
Packet 只负责数据
Codec 只负责编解码
Dispatcher 才负责业务
```
