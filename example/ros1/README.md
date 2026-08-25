# ROS1 示例工作空间

本目录可直接作为 catkin 工作空间使用，ROS 包位于 `src/hightorque_robot_ros1_example`。

8 个示例命令分别对应包内与 `example/cpp/` 同名的 8 个 C++ 源文件，分别演示 `CanPort` 和 `Robot/Motor` 两种用法。

```bash
cd /path/to/hightorque_robot/example/ros1
catkin_make
source devel/setup.bash
rosrun hightorque_robot_ros1_example motors_feedback
```

状态话题：`/motor_states`。
