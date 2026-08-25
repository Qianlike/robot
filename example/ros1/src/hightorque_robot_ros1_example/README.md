# hightorque_robot ROS 1 examples

本目录是独立的 ROS 1 catkin 包，不参与项目根目录的普通 CMake 构建。它包含与 `example/cpp/` 同名的 `canport_*` 和 `motors_*` 例程，每个 ROS 例程都是一个独立的 `.cpp` 文件；`example/ros1` 已配置为可直接使用的 catkin 工作空间。

## 构建

直接在 `example/ros1` 目录执行：

```bash
cd /path/to/hightorque_robot/example/ros1
catkin_make
source devel/setup.bash
```

包会在 ROS 工作空间的构建目录中编译项目根目录的 `hightorque_robot` 目标，不会加入项目根目录的普通 CMake 构建。默认配置位于包共享目录；也可以通过 `robot_config` 参数指定入口 YAML：

```bash
rosrun hightorque_robot_ros1_example motors_feedback
rosrun hightorque_robot_ros1_example motors_run _robot_config:=/absolute/path/to/robot_config.yaml
```

## 状态话题

所有例程都会以 100 Hz 发布 `motor_states`，消息类型为 `hightorque_robot_ros1_example/MotorState`。每条消息对应一个电机，包含 `can_port_id`、`id`、`mode`、`fault`、`position`、`velocity` 和 `torque` 字段，可被其他 ROS 节点订阅：

```bash
rostopic echo /motor_states
```

`motors_*` 使用 `Robot + Motor` 和 YAML 配置；`canport_*` 直接创建一个 `CanPort` 并在代码中指定电机 ID。
