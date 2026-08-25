# hightorque_robot ROS 2 examples

本目录是独立的 ROS 2 `ament_cmake` 包，不参与项目根目录的普通 CMake 构建。它包含与
`example/cpp/` 同名的 8 个独立 `canport_*` 和 `motors_*` 例程；`example/ros2` 已配置为可直接使用的 ROS 2 工作空间。

## 构建

直接在 `example/ros2` 目录执行：

```bash
cd /path/to/hightorque_robot/example/ros2
source /opt/ros/humble/setup.bash  # 按实际 ROS2 发行版替换 humble
colcon build --packages-select hightorque_robot_ros2_example
source install/setup.bash
```

包会在自身构建目录中编译项目根目录的 `hightorque_robot` 目标，不会加入项目根目录的普通 CMake 构建。安装后默认配置位于包共享目录；也可以通过 `robot_config` 参数指定入口 YAML：

```bash
ros2 run hightorque_robot_ros2_example motors_feedback
ros2 run hightorque_robot_ros2_example motors_run --ros-args \
  -p robot_config:=/absolute/path/to/robot_config.yaml
```

## 状态话题

所有例程都会以 100 Hz 发布 `motor_states`，消息类型为
`hightorque_robot_ros2_example/msg/MotorState`。每条消息对应一个电机，字段与 ROS1 示例一致：
`can_port_id`、`id`、`mode`、`fault`、`position`、`velocity` 和 `torque`。
其他 ROS 2 节点可以直接订阅该话题：

```bash
ros2 topic echo /motor_states
```

`motors_*` 使用 `Robot + Motor` 和 YAML 配置；`canport_*` 直接创建一个 `CanPort` 并在代码中指定电机 ID。

8 个可执行程序如下：

| 可执行程序 | 作用 |
| --- | --- |
| `canport_feedback` | 直接创建 CAN 通道 1，查询电机 ID 1 的状态 |
| `canport_move_zero` | 直接创建 CAN 通道 1，执行电机零位重置并查询状态 |
| `canport_run` | 控制 CAN 通道 1 的电机 1、2、3 在 ±0.314 rad 间往复 |
| `canport_set_zero` | 控制 CAN 通道 1 的电机 1、2、3 持续发送零位置目标 |
| `motors_feedback` | 从 YAML 构造 `Robot`，查询所有电机状态 |
| `motors_move_zero` | 从 YAML 构造 `Robot`，以限制速度移动所有电机到零位置 |
| `motors_run` | 从 YAML 构造 `Robot`，让所有电机在 ±0.314 rad 间往复 |
| `motors_set_zero` | 从 YAML 构造 `Robot`，执行所有电机零位重置并查询状态 |

控制示例会访问真实电机，同一时间只运行一个控制节点；运行前确认设备连接和机械结构安全。
