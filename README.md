# KingYoungy's Chat Room

一个基于System V消息队列实现的本地聊天室程序，支持多用户创建房间、加入房间及实时聊天功能。

## 项目结构

```
chat_room/
├── exec/                 # 可执行文件输出目录
├── lib/                  # 共享库存放目录
├── source/
│   ├── cli/              # 客户端源代码
│   │   ├── connector.c   # 服务器连接功能
│   │   ├── creatroom.c   # 创建房间功能
│   │   ├── removeroom.c  # 删除房间功能
│   │   ├── talk.c        # 聊天核心功能
│   │   ├── tool.c        # 工具函数
│   │   ├── upgrade.c     # 信息更新功能
│   │   ├── welcome.c     # 欢迎界面
│   │   ├── sv_msq_client.c # 客户端主程序
│   │   ├── sv_msq_client.h # 客户端头文件
│   │   └── sv_msq.h      # 公共协议头文件
│   └── sev/              # 服务端源代码
│       ├── data_struct.c # 数据结构实现
│       ├── thread_1_msgsender.c # 消息发送线程
│       ├── thread_3_roommanager.c # 房间管理线程
│       ├── sv_msq_sev.c  # 服务端主程序
│       └── sv_msq_sev.h  # 服务端头文件
├── automake.sh           # 自动编译脚本
└── .gitignore            # Git忽略文件
```

## 功能特点

- 多用户同时在线聊天
- 支持创建和删除房间
- 实时更新在线用户和房间列表
- 支持窗口大小自适应
- 基于消息队列的进程间通信
- 线程安全的操作处理

## 编译方法

1. 确保系统安装了`gcc(GNU Compiler Collection)`
2. 运行自动编译脚本：

```bash
chmod +x automake.sh
./automake.sh
```

编译完成后，可执行文件将生成在`exec/`目录下：
- 服务端：`sv_msq_server`
- 客户端：`sv_msq_client`

## 使用方法

1. 首先启动服务端：

```bash
./exec/sv_msq_server
```

2. 启动客户端（可同时启动多个）：

```bash
./exec/sv_msq_client
```

3. 客户端操作流程：
   - 输入用户名
   - 连接服务器
   - 选择操作：
     - A: 更新在线用户和房间列表
     - B: 创建新房间
     - C: 删除已有房间
     - D: 加入房间聊天
     - 其他: 退出程序

4. 聊天操作：
   - 在聊天界面中直接输入消息并按回车发送
   - 输入`!exit`退出当前聊天房间

## 技术细节

- 采用System V消息队列实现进程间通信
- 使用多线程处理并发消息发送和接收
- 采用ncurses库实现终端界面和交互
- 使用链表数据结构管理用户和房间信息
- 采用互斥锁保证多线程操作的数据安全性

## 协议说明

系统定义了8种消息类型用于客户端与服务端通信：

- TYPE1: 用户注册信息（用户名和客户端消息队列ID）
- TYPE2: 操作结果响应（成功/失败）
- TYPE3: 请求更新用户和房间信息
- TYPE4: 回复用户和房间列表信息
- TYPE5: 房间操作（创建/删除/加入）
- TYPE6: 发送聊天消息
- TYPE7: 接收聊天消息
- TYPE8: 退出房间请求

每种消息类型都有特定的数据结构和大小定义，确保通信的一致性和正确性。
