# C++ IM聊天服务器

基于C++17和muduo网络库实现的IM聊天服务器练手项目。

## 项目简介

本项目是一个用于学习C++网络编程的练手项目，实现了用户管理、单聊消息、好友管理等核心功能。代码结构清晰，采用现代C++特性和工程化实践，适合作为学习C++网络编程和后端开发的入门参考。

## 技术栈

| 分类 | 技术 | 说明 |
|-----|------|------|
| 语言 | C++17 | 智能指针、RAII、多线程等现代C++特性 |
| 网络 | muduo | 基于Reactor模式的高性能C++网络库 |
| 数据库 | MySQL 8.0 | 存储用户、好友、离线消息数据（已实现） |
| 缓存 | Redis 7.x | 基础接口已实现，待业务集成 |
| 序列化 | Protobuf | 消息编解码已实现，替代JSON提升性能 |
| 日志 | spdlog | 分级日志和日志文件切割（已实现） |
| 配置 | INI配置文件 | 集中管理服务器、数据库连接参数 |
| 加密 | bcrypt | 密码安全存储（已实现） |
| 构建 | CMake | 支持Debug/Release双模式编译 |
| 部署 | Docker | 容器化部署支持 |

## 核心功能（已实现）

- **用户管理**：注册、登录、注销、状态维护
- **单聊消息**：实时消息发送、ACK应答机制
- **好友管理**：好友添加、好友列表查询
- **离线消息**：离线消息存储（MySQL）、登录后拉取
- **心跳机制**：定时心跳检测、超时离线标记
- **安全防护**：SQL预编译防注入、bcrypt密码加密

## 系统架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        客户端 (Client)                         │
└─────────────────────────────────────────────────────────────────┘
                              │ TCP/IP
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    ChatServer (muduo Reactor)                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐         │
│  │  EventLoop   │→│  TcpServer   │→│ MessageCodec │         │
│  └──────────────┘  └──────────────┘  └──────┬───────┘         │
│                                             │                  │
│  ┌───────────────────────────────────────────▼──────────────┐   │
│  │                    ChatService                           │   │
│  │  ┌────────────┐ ┌────────────┐ ┌─────────────────────┐   │   │
│  │  │ UserModel  │ │FriendModel │ │ OfflineMsgModel     │   │   │
│  │  └─────┬──────┘ └─────┬──────┘ └─────────┬───────────┘   │   │
│  └───────┬───────────────┼─────────────────┼──────────────┘   │
│          │               │                 │                   │
└──────────┼───────────────┼─────────────────┼──────────────────┘
           │               │                 │
           ▼               ▼                 ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│    MySQL 8.0    │ │    Redis 7.x    │ │   spdlog        │
│ 用户/好友/消息  │ │ 基础接口已实现   │ │  分级日志       │
└─────────────────┘ └─────────────────┘ └─────────────────┘
```

## 项目结构

```
chatserver/
├── config/                 # 配置文件
│   └── server.conf         # INI格式配置文件
├── include/server/         # 头文件目录
│   ├── chatserver.hpp      # 服务器主类（含心跳检测）
│   ├── chatservice.hpp     # 业务逻辑类（单例模式）
│   ├── db/                 # 数据库相关
│   │   └── mysql_pool.h    # MySQL连接池（健康检查+连接复用）
│   ├── model/              # 数据模型
│   │   ├── user.hpp        # 用户模型
│   │   ├── usermodel.hpp   # 用户操作类
│   │   ├── friendmodel.hpp # 好友操作类
│   │   └── offlinemessagemodel.hpp # 离线消息操作类
│   ├── net/                # 网络模块（muduo封装）
│   │   ├── TcpServer.h     # TCP服务器封装
│   │   └── MessageCodec.h  # Protobuf消息编解码器
│   ├── redis/              # Redis相关
│   │   └── redis.hpp       # Redis操作封装（基础接口）
│   └── util/               # 工具类
│       ├── logging.h       # 日志封装
│       ├── config.h        # 配置解析类
│       └── bcrypt.h        # bcrypt密码加密
├── src/server/             # 源文件目录
│   ├── main.cpp            # 主函数
│   ├── chatserver.cpp      # 服务器实现
│   ├── chatservice.cpp     # 业务逻辑实现
│   └── ...                 # 其他模块实现
├── proto/                  # Protobuf定义
│   └── chat.proto          # 消息协议定义
├── test/                   # 单元测试
└── CMakeLists.txt          # 构建配置
```

## 编译运行

### 1. 原生编译

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
./bin/ChatServer 127.0.0.1 6000
```

### 2. Docker容器化部署

```bash
docker-compose up -d
docker-compose logs -f chatserver
docker-compose down
```

### 3. 单元测试

```bash
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
make -j4
cd test && ./TestMessageCodec
```

## 配置说明

配置文件位于 `config/server.conf`，包含服务器、MySQL、Redis、日志等配置项。

## 登录流程时序图

```
Client                    ChatServer               ChatService               MySQL
  │                           │                         │                      │
  │── LoginRequest(Protobuf)─▶│                         │                      │
  │                           │── handleMessage() ─────▶│                      │
  │                           │                         │── query(user) ──────▶│
  │                           │                         │◀── User ────────────│
  │                           │                         │── verifyPassword() ─▶│
  │                           │                         │◀── OK ─────────────│
  │                           │                         │── updateState() ───▶│
  │                           │◀── LoginResponse ──────│                      │
  │◀── LoginResponse(Protobuf)│                         │                      │
```

## 实现细节

### 1. Protobuf序列化实现

在 `MessageCodec.cpp` 中实现了Protobuf编解码：

```cpp
// 序列化：将Protobuf对象转为字节流
bool MessageCodec::encodeMessage(const ChatMessage& msg, Buffer* buf) {
    string serialized_msg;
    msg.SerializeToString(&serialized_msg);  // Protobuf序列化
    encode(serialized_msg, buf);
    return true;
}

// 反序列化：将字节流转为Protobuf对象
bool MessageCodec::decodeMessage(Buffer* buf, ChatMessage& msg) {
    string serialized_msg;
    decode(buf, serialized_msg);
    msg.ParseFromString(serialized_msg);  // Protobuf反序列化
    return true;
}
```

**选择Protobuf的原因**：
- 相比JSON，Protobuf序列化后数据体积更小（约减少30-40%）
- 序列化/反序列化速度更快（约提升20%）
- 强类型定义，编译期类型检查
- 更好的版本兼容性

### 2. MySQL连接池实现

在 `mysql_pool.cpp` 中实现了连接池：
- 支持连接复用，减少连接建立开销
- 连接健康检查，自动剔除失效连接
- 空闲连接回收机制

### 3. TCP粘包拆包处理

采用固定包头协议：
- 包头：4字节（网络字节序），存储消息长度
- 包体：Protobuf序列化后的消息数据

```cpp
// 解码流程
1. 读取4字节包头 → 获取消息长度
2. 检查缓冲区是否有足够数据
3. 读取完整消息体 → 反序列化为Protobuf对象
```

## 功能实现状态

| 功能 | 状态 | 说明 |
|------|------|------|
| 用户注册/登录 | ✅ 已实现 | 包含密码加密验证 |
| 用户状态维护 | ✅ 已实现 | 在线/离线状态切换 |
| 单聊消息 | ✅ 已实现 | 实时发送+ACK应答 |
| 离线消息 | ✅ 已实现 | MySQL存储，登录拉取 |
| 好友管理 | ✅ 已实现 | 添加好友+列表查询 |
| 心跳检测 | ✅ 已实现 | 超时离线标记 |
| Redis缓存 | ⚠️ 待集成 | 基础接口已实现 |
| 群组功能 | ❌ 未实现 | 预留扩展 |
| 高可用 | ❌ 未实现 | 单节点部署 |

## 学习收获

通过本项目学习到：
- muduo网络库使用和Reactor模式理解
- TCP粘包拆包处理方案
- MySQL连接池设计与实现
- Protobuf序列化技术及性能优势
- C++17现代特性实践（智能指针、RAII、多线程）
- 工程化配置管理和日志系统
- 基础网络安全防护知识（SQL注入、密码加密）

## 待改进项

1. **Redis集成**：将Redis用于缓存用户状态、离线消息
2. **高可用方案**：实现多节点部署和负载均衡
3. **单元测试**：增加核心模块测试覆盖率
4. **监控告警**：添加服务监控和异常告警

## 总结

本项目是一个学习性质的IM聊天服务器练手项目，重点实现了网络通信、数据库操作和安全防护等核心功能。通过实践学习，可以掌握C++后端开发的基础技能和工程化实践方法。