# RealmNet (墟界)

> 可扩展的网络数据包框架

---

## 项目简介

**RealmNet（中文名：墟界）** 是一个**可扩展的网络数据包框架**，专注于数据包的编解码、分发和协议管理。通过**少量胶水代码**即可与任意 Socket 后端（BSD Socket、epoll、kqueue、IOCP）连接，快速构建高效的网络通信系统。

**核心框架**位于 `realmnet/` 目录下，提供完整的数据包处理基础设施。根目录下的 `server/` 和 `client/` 只是使用该框架结合 BSD Socket 实现的**简单登录示例**，用于演示框架的基本用法。

### 核心设计理念

```text
┌─────────────────────────────────────────────────────┐
│           业务层 (Business Layer)                   │
├─────────────────────────────────────────────────────┤
│  ┌─────────────┐                                   │
│  │ Dispatcher  │ ←── Handler 注册与分发            │
│  └──────┬──────┘                                   │
│         ↓                                          │
│  ┌─────────────┐                                   │
│  │  Factory    │ ←── 自动协议注册与创建            │
│  └──────┬──────┘                                   │
│         ↓                                          │
│  ┌─────────────┐                                   │
│  │   Codec     │ ←── 序列化/反序列化              │
│  └──────┬──────┘                                   │
│         ↓                                          │
│  ┌─────────────┐     ┌─────────────────────────┐   │
│  │ Connection  │────→│   ISocket (接口抽象)     │   │
│  └─────────────┘     └─────────────────────────┘   │
│                           ↓                        │
│              ┌───────────┼───────────┐             │
│              ▼           ▼           ▼             │
│         BSD Socket   epoll/io_uring   IOCP         │
│       (同步阻塞)   (Linux 高并发)  (Windows 高并发) │
└─────────────────────────────────────────────────────┘
```

### 核心优势

| 特性 | 说明 |
|------|------|
| **协议与传输解耦** | 数据包处理与 Socket 实现完全分离 |
| **可扩展架构** | 通过 ISocket 接口轻松切换后端 |
| **自动协议注册** | 基于 TypeHash 的零配置协议管理 |
| **高性能分发** | 无 switch-case 的高效数据包路由 |
| **少量胶水代码** | 快速接入任意 Socket 后端 |

### 适用场景

- **MMO 游戏**：大规模并发连接管理
- **实时对战**：低延迟数据包传输
- **RPC 服务**：高效远程调用
- **网关服务**：协议转发与路由
- **微服务**：轻量级跨服务通信
- **长连接系统**：稳定的持久连接管理

---

## 目录结构

```text
RealmNet/
│
├── realmnet/           # 【核心框架】网络数据包处理基础设施
│   ├── core/           # 核心模块
│   │   ├── BasePacket.h        # 数据包基类（定义协议接口）
│   │   ├── BinaryReader.h      # 二进制读取器
│   │   ├── BinaryWriter.h      # 二进制写入器
│   │   ├── Connection.h/cpp    # 连接管理（胶水层）
│   │   ├── PacketCodec.h/cpp   # 编解码器
│   │   ├── PacketDispatcher.h/cpp  # 数据包分发器
│   │   ├── PacketFactory.h     # 数据包工厂
│   │   ├── PacketRegistrar.h   # 协议自动注册器
│   │   └── TypeHash.h          # 类型哈希（无 enum 协议 ID）
│   │
│   └── socket/         # Socket 抽象层（扩展点）
│       └── ISocket.h   # Socket 接口定义（核心扩展接口）
│
├── common/             # 【示例共享代码】
│   ├── Packets.h       # 示例协议定义（LoginRequest/Response）
│   ├── TcpSocket.h     # BSD Socket 实现（胶水示例）
│   └── TcpSocket.cpp
│
├── server/             # 【示例服务端】使用 BSD Socket 的服务端
│   └── main.cpp
│
├── client/             # 【示例客户端】使用 BSD Socket 的客户端
│   └── main.cpp
│
└── CMakeLists.txt      # 构建配置
```

### 目录职责说明

| 目录 | 职责 | 说明 |
|------|------|------|
| `realmnet/core/` | **数据包核心** | 协议、编解码、分发（核心价值） |
| `realmnet/socket/` | **Socket 抽象** | ISocket 接口（扩展点） |
| `common/` | **示例实现** | TcpSocket 胶水代码示例 |
| `server/client/` | **演示代码** | 使用框架的示例程序 |

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

### 第一步：定义协议

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

REGISTER_PACKET(LoginRequest)  // 自动注册到工厂
```

### 第二步：实现 Handler

```cpp
class LoginHandler
{
public:
    void operator()(Connection& conn, BasePacket& packet) {
        auto& req = static_cast<LoginRequest&>(packet);
        
        LoginResponse resp;
        resp.success = (req.password == "123456");
        resp.message = resp.success ? "登录成功" : "密码错误";
        
        conn.sendPacket(resp);
    }
};
```

### 第三步：接入 Socket 后端

```cpp
// 1. 创建 Socket 实例（可替换为 epoll/IOCP 实现）
auto socket = std::make_shared<TcpSocket>(clientFd);

// 2. 创建 Connection（胶水层）
Connection conn(socket);

// 3. 注册 Handler 并启动
PacketDispatcher dispatcher;
dispatcher.registerHandler(LoginRequest::ID, LoginHandler());

conn.setPacketCallback([&](Connection& c, BasePacket& pkt) {
    dispatcher.dispatch(c, pkt);
});
```

---

## 扩展新的 Socket 后端

只需实现 `ISocket` 接口，即可接入任意 Socket 后端：

```cpp
class EpollSocket : public ISocket
{
public:
    bool send(const uint8_t* data, size_t len) override {
        // epoll 发送实现
    }
    
    int recv(uint8_t* buffer, size_t len) override {
        // epoll 接收实现
    }
    
    void close() override {
        // 关闭实现
    }
};
```

无需修改核心框架代码，**零侵入式扩展**。

---

## 核心设计原则

### 原则一：协议与传输分离

```text
Packet ──(数据)──→ Codec ──(字节流)──→ Socket
                        ↑
                   解耦边界
```

**好处**：
- 协议变更不影响网络层
- 网络层优化不影响协议定义
- 支持多种序列化方案（Protobuf/FlatBuffers）

### 原则二：无 Enum 协议 ID

使用 **TypeHash** 替代传统 enum：

```cpp
// 传统方式（需要维护枚举）
enum PacketType { LOGIN, CHAT, MOVE };

// RealmNet 方式（自动计算）
static constexpr TypeID ID = fnv1a("LoginRequest");
```

**好处**：
- 无需集中维护枚举
- 编译期自动计算 ID
- 避免 ID 冲突
- 更好的代码组织

### 原则三：自动协议注册

```cpp
REGISTER_PACKET(LoginRequest)  // 一行代码完成注册
```

**好处**：
- 零配置启动
- 编译期检查
- 避免运行时注册遗漏

---

## 架构演进路线

```text
阶段一：基础数据包框架（当前）
    └── 协议定义、编解码、分发
    
阶段二：多后端支持
    ├── BSD Socket（已实现）
    ├── epoll/kqueue
    └── IOCP
    
阶段三：高性能优化
    ├── 内存池
    ├── 零拷贝
    └── 协程化
    
阶段四：完整解决方案
    ├── RPC 支持
    ├── 负载均衡
    └── 监控与追踪
```

---

## 许可证

MIT License

---

## 联系方式

如有问题或建议，欢迎提交 Issue 或 PR。
