# Ticket System

一个基于文件持久化与 B+ 树索引的命令行火车票管理系统，对应 ACM 班 2024 Ticket System 课程作业。

## 项目简介

本项目实现了一个支持多用户、多车次、购票、候补、退票、订单查询的命令行票务系统。  
系统核心数据均持久化在磁盘文件中，并使用自实现的 B+ 树作为索引结构，以支持较高效的查询与更新。

## 已实现功能

- 用户管理
  - `add_user`
  - `login`
  - `logout`
  - `query_profile`
  - `modify_profile`
- 车次管理
  - `add_train`
  - `delete_train`
  - `release_train`
  - `query_train`
  - `query_ticket`
  - `query_transfer`
- 订单管理
  - `buy_ticket`
  - `query_order`
  - `refund_ticket`
- 系统管理
  - `clean`
  - `exit`

## 技术实现

### 1. 文件持久化

系统中的用户、车站、车次、座位、订单、元数据等信息均保存在磁盘文件中，程序重启后可继续恢复状态。

### 2. 自实现 B+ 树

项目实现了模板化的磁盘 B+ 树：

- 每个结点大小固定为 `4 KiB`
- 根据键值类型自动计算页容量
- 支持以下键类型：
  - `std::string`
  - `int`
  - `std::pair<int, int>`
- 支持重复键查询
- 支持磁盘读写与页缓存

### 3. 元数据管理

通过统一的 `MetaManager` 维护：

- `user_count`
- `station_count`
- `seat_count`
- `train_count`
- `order_count`

用于保证记录编号在程序重启后仍然正确递增。

### 4. 页缓存

B+ 树实现了页缓存机制，用于减少重复磁盘 I/O。  
在实际压力测试中，相比关闭缓存的版本，运行时间有明显下降。

## 代码结构

- [main.cpp](main.cpp)
  - 指令解析与程序入口
- [bplustree.h](bplustree.h)
  - 模板化磁盘 B+ 树与页缓存
- [user.h](user.h), [user.cpp](user.cpp)
  - 用户模块
- [station.h](station.h), [station.cpp](station.cpp)
  - 车站模块
- [seat.h](seat.h), [seat.cpp](seat.cpp)
  - 座位模块
- [train.h](train.h), [train.cpp](train.cpp)
  - 车次模块
- [order.h](order.h), [order.cpp](order.cpp)
  - 订单与候补模块
- [file.h](file.h), [file.cpp](file.cpp)
  - 元数据管理
- [info.h](info.h)
  - 常量、工具函数与基础类型

## 编译方式

在项目根目录下使用：

```bash
g++ -std=c++17 -O2 main.cpp user.cpp station.cpp seat.cpp train.cpp order.cpp file.cpp -o code.exe
```

## 运行方式

```bash
code.exe
```

程序运行过程中会在 `data/` 目录下生成持久化文件。

## 测试说明

课程测试数据位于 [testcases](testcases) 目录。  
可配合 [run-test.txt](run-test.txt) 中的说明进行测试。

项目已完成基础测试与多组压力测试验证，覆盖：

- `basic_1 ~ basic_6`
- `basic_extra`
- `pressure_1_easy ~ pressure_3_easy`
- `pressure_1_hard ~ pressure_3_hard`

## 项目特点

- 自实现磁盘 B+ 树，而非依赖 STL 平衡树
- 支持重复键、范围扫描与多索引场景
- 支持候补订单与退票后自动补票
- 支持程序重启后的状态恢复
- 引入页缓存机制，提升磁盘索引性能

## 说明

本项目主要用于课程作业与数据结构/数据库文件系统相关训练，重点在于：

- 外存数据结构设计
- 持久化索引实现
- 多模块协同与状态一致性维护

Built a C++ command-line railway ticket system with user, train, ticket, order, pending, and refund management.

Designed a file-based persistent storage layer with metadata recovery to preserve system state across restarts.

Implemented a disk-based template B+ tree with fixed 4 KiB pages and added a page cache, improving pressure-test performance by about 34%–45%.
