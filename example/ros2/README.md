# ROS2 示例工作空间

本目录可直接作为 ROS2 工作空间使用，ROS 包位于 `src/hightorque_robot_ros2_example`。

8 个示例命令分别对应包内与 `example/cpp/` 同名的 8 个独立 C++ 源文件，分别演示 `CanPort` 和 `Robot/Motor` 两种用法。

```bash
cd /path/to/hightorque_robot/example/ros2
source /opt/ros/humble/setup.bash  # 按实际 ROS2 发行版替换 humble
colcon build --packages-select hightorque_robot_ros2_example
source install/setup.bash
ros2 run hightorque_robot_ros2_example motors_feedback
```

所有例程以 100 Hz 发布单个电机状态到 `/motor_states`，消息类型为
`hightorque_robot_ros2_example/msg/MotorState`。
