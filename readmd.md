# 高擎电机控制SDK
高擎机器人电机控制SDK，提供C++控制接口，用于控制高擎关节电机。

## 一、介绍

本项目是一个用于控制高擎机器人电机的SDK，通过USB虚拟串口完成到CANFD协议的通信转发板与电机通信。本SDK提供了完整的C++库，可以方便地集成到各种机器人控制系统中实现电机控制。

### 主要特性

* 支持多种电机型号（4438、5046、5047、6056等系列）；
* 多种控制模式：位置控制、速度控制、力矩控制、混合控制；
* 支持多个CAN总线板卡和多个电机；
* 提供LCM（Lightweight Communications and Marshalling）消息发布；
* 实时电机状态反馈；

## 二、安装依赖

### 依赖库

* CMake >= 3.0.2
* C++11 编译器
* libserialport（串口通信）
* yaml-cpp（YAML配置文件解析）

1. 安装串口依赖
```
sudo apt-get install libserialport-dev
```
1. 安装yaml解析器
```
sudo apt-get install libyaml-cpp-dev
```

## 三、编译

1. 获取代码
```
git clone http://git.clicki.cn/livelybot/hightorque_robot.git
```

2. 编译
```
cd hightorque_robot
mkdir build
cd build
cmake ..
make -j8
```