#include <rclcpp/rclcpp.hpp>
#include <hightorque_robot_ros2_example/msg/motor_state.hpp>

#include "canport.h"

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("canport_move_zero");
    auto publisher = node->create_publisher<hightorque_robot_ros2_example::msg::MotorState>(
        "motor_states", 10);
    CanPort can_port(1, {1});
    can_port.motor_zero_pos_reset();
    rclcpp::WallRate rate(100.0);

    while (rclcpp::ok())
    {
        can_port.request_motor_state();
        for (const auto& item : can_port.map_motors_state)
        {
            const motor_state_t& state = item.second;
            hightorque_robot_ros2_example::msg::MotorState motor;
            motor.can_port_id = 1;
            motor.id = item.first;
            motor.mode = state.mode;
            motor.fault = state.fault;
            motor.position = state.position;
            motor.velocity = state.velocity;
            motor.torque = state.torque;
            publisher->publish(motor);
        }
        rclcpp::spin_some(node);
        rate.sleep();
    }

    can_port.stop();
    can_port.send();
    rclcpp::shutdown();
    return 0;
}
