

# SyncTunnel

一个支持文件同步和远程管理的安全传输工具。

## 软件简介

SyncTunnel 是一款安全传输软件，支持文件同步和远程管理功能。该软件使用自主研发的 DTATC（双轨自适应传输控制）算法，在公网环境中表现出色。

### 主要特性

- **AES-256-CBC 加密**：数据传输全程采用 AES-256-CBC 加密，确保数据安全
- **DTATC 双轨自适应传输控制**：自主研发的拥塞控制算法，在高延迟、高丢包率的网络环境下仍能保持高效的传输性能
- **远程拉取文件**：可直接从运行 SyncTunnel 的远程电脑拉取任意文件
- **远程控制**：支持远程控制其他电脑
- **远程关机**：支持远程关闭其他电脑
- **文件挂起**：电脑关机前可挂起文件，其他电脑关机后仍可下载
- **DFHN 分布式文件挂起节点**：可将客户端作为分布式文件挂起节点，解决 GitHub 访问速度慢的问题，可作为 NAS 平替方案

## 技术架构

- **Qt 框架**：基于 Qt C++ 开发
- **通信模块**：支持 UDP、IPv6、STUN 穿透
- **信令模块**：基于 MQTT 的设备发现和连接管理
- **传输引擎**：可靠消息传输与文件同步
- **存储模块**：文件上传与检索

## 编译与运行

### 环境要求

- Qt 5.15 或更高版本
- 支持 C++17 的编译器
- MQTT 客户端库

### 编译步骤

```bash
# 使用 qmake 构建
qmake SyncTunnel.pro
make
```

### 运行

编译完成后，运行生成的可执行文件即可启动 SyncTunnel。

## 目录结构

```
SyncTunnel/
├── core/                  # 核心模块
│   ├── basic/            # 基础类（缓存数组等）
│   └── services/         # 服务类（定时调度等）
├── dialogs/              # 对话框界面
├── modules/              # 功能模块
│   ├── communication/   # 通信模块
│   ├── signalling/     # 信令模块
│   ├── storage/       # 存储模块
│   └── transmissionengine/ # 传输引擎
├── android/             # Android 支持
├── rc/                   # 资源文件
│   ├── audio/          # 音频资源
│   ├── img/           # 图片资源
│   ├── style/         # 样式主题
│   └── translations/  # 国际化翻译
└── mainwindow.cpp       # 主窗口实现
```

## 支持与反馈

- 问题反馈：发送邮件至 nnpyro2@outlook.com
- 欢迎提交 Issue 和 Pull Request
- 诚邀开发人员加入项目开发

## 开源许可

本项目基于 GPL 或类似开源许可证发布，详见 LICENSE 文件。

## 相关链接

- [Gitee 镜像](https://gitee.com/nnpyro/SyncTunnel)

---

*注意：本软件由开发者自主独立创作，与 SyncThing 或其他类似的 P2P 文件同步工具无关。*
