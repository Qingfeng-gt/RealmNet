# RealmNet (墟界)

> 工业级 C++ 网络通信框架

---

## 项目简介

**RealmNet（中文名：墟界）** 是一个高性能、跨平台的 C++ 网络通信框架。

**核心框架**位于 `realmnet/` 目录下，提供完整的网络通信基础设施。根目录下的 `server/` 和 `client/` 只是使用该框架实现的**简单登录示例**，用于演示框架的基本用法。

### 核心特性

- **平台无关**：支持 Windows、Linux、macOS
- **网络无关**：Socket 后端可插拔
- **业务无关**：协议与业务完全解耦
- **自动协议注册**：无需手动管理协议类型
- **无 switch / 无 enum**：基于 TypeHash 的协议分发
- **客户端/服务端共享协议**：一套协议，两端使用

### 适用场景

- MMO 游戏
- 实时对战
- RPC 服务
- 网关服务
- 微服务
- 长连接系统

---

## 整体架构

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

## 目录结构

```text
RealmNet/
│
├── realmnet/           # 【核心框架】网络通信基础设施
│   ├── core/           # 核心模块
│   │   ├── BasePacket.h        # 数据包基类
│   │   ├── BinaryReader.h      # 二进制读取器
│   │   ├── BinaryWriter.h      # 二进制写入器
│   │   ├── Connection.h/cpp    # 连接管理
│   │   ├── PacketCodec.h/cpp   # 编解码器
│   │   ├── PacketDispatcher.h/cpp  # 数据包分发器
│   │   ├── PacketFactory.h     # 数据包工厂
│   │   ├── PacketRegistrar.h   # 协议注册器
│   │   └── TypeHash.h          # 类型哈希
│   │
│   └── socket/         # Socket 抽象层
│       └── ISocket.h   # Socket 接口定义
│
├── common/             # 【示例共享代码】客户端/服务端共用
│   ├── Packets.h       # 示例协议定义（LoginRequest/Response）
│   ├── TcpSocket.h     # TCP Socket 实现
│   └── TcpSocket.cpp
│
├── server/             # 【示例服务端】登录服务端实现
│   └── main.cpp
│
├── client/             # 【示例客户端】登录客户端实现
│   └── main.cpp
│
└── CMakeLists.txt      # 构建配置
```

### 目录职责说明

| 目录 | 职责 | 说明 |
|------|------|------|
| `realmnet/` | **核心框架** | 可独立作为库使用 |
| `common/` | **示例共享代码** | 仅用于示例演示 |
| `server/` | **示例服务端** | 使用框架的服务端示例 |
| `client/` | **示例客户端** | 使用框架的客户端示例 |

---

## 快速开始

### 编译环境

- CMake 3.16+
- C++17 编译器（MSVC 2022 / MinGW 8.0+ / GCC 8.0+）

### 编译步骤

```bash
# 创建构建目录
mkdir cmake-build-debug-msvc
cd cmake-build-debug-msvc

# 配置项目
cmake .. -G "Visual Studio 17 2022" -A x64

# 构建项目
cmake --build . --config Debug
```

### 运行示例

**启动服务端：**
```bash
cmake-build-debug-msvc/Debug/RealmNetServer.exe
```

**启动客户端：**
```bash
cmake-build-debug-msvc/Debug/RealmNetClient.exe
```

---

## 使用框架开发

### 1. 定义协议

```cpp
#include "realmnet/core/BasePacket.h"
#include "realmnet/core/PacketRegistrar.h"

class LoginRequest : public BasePacket
{
public:
    static constexpr TypeID ID = fnv1a("LoginRequest");
    
    std::string account;
    std::string password;
    
    TypeID type() const override { return ID; }
    
    void serialize(BinaryWriter& writer) const override {
        writer.writeString(account);
        writer.writeString(password);
    }
    
    void deserialize(BinaryReader& reader) override {
        account = reader.readString();
        password = reader.readString();
    }
};

REGISTER_PACKET(LoginRequest)
```

### 2. 实现 Handler

```cpp
class LoginHandler
{
public:
    void operator()(Connection& conn, BasePacket& packet) {
        auto& req = static_cast<LoginRequest&>(packet);
        
        LoginResponse resp;
        resp.success = (req.password == "123456");
        conn.sendPacket(resp);
    }
};
```

### 3. 注册并启动

```cpp
PacketDispatcher dispatcher;
dispatcher.registerHandler(LoginRequest::ID, LoginHandler());

conn.setPacketCallback([&](Connection& c, BasePacket& pkt) {
    dispatcher.dispatch(c, pkt);
});
```

---

## 核心设计思想

### 传统问题

传统网络设计使用大量 switch-case：

```cpp
switch(packetType)
{
    case LOGIN: ... break;
}
```

存在**高耦合、不可扩展、难维护**等问题。

### 本框架方案

```text
Packet → Codec → Factory → Dispatcher → Handler
```

实现：
- **协议与业务解耦**：Packet 只负责数据
- **网络与协议解耦**：Socket 只负责传输
- **网络后端可替换**：ISocket 接口抽象

---

## 后续优化方向

1. **高性能 IO**：epoll、kqueue、IOCP 支持
2. **多线程架构**：网络线程与逻辑线程分离
3. **内存池**：PacketPool、BufferPool
4. **序列化框架**：Protobuf、FlatBuffers
5. **压缩与加密**：zstd、lz4、AES、RSA
6. **RPC 支持**：requestId、callback、future/promise
7. **Actor/ECS**：适用于 MMO 游戏
8. **协程化**：C++20 coroutine 支持

---

## 许可证

MIT License

---

## 联系方式

如有问题或建议，欢迎提交 Issue 或 PR。
