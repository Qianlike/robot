import time

from hightorque_robot import Robot


def main():
    kControlPeriod = 0.001
    robot = Robot("../robot_param/robot_config.yaml")

    robot.motor_zero_pos_reset()

    next_tick = time.monotonic()
    while True:
        robot.request_motor_state()

        for motor in robot.motors:
            state = motor.get_motor_state()
            print(
                "ID: {:2d}, mode: {:2d}, fault: {:2d}, pos: {:2.3f}, "
                "vel: {:2.3f}, tor: {:2.3f}".format(
                    motor.get_id(),
                    state.mode,
                    state.fault,
                    state.position,
                    state.velocity,
                    state.torque,
                )
            )

        next_tick += kControlPeriod
        time.sleep(max(0.0, next_tick - time.monotonic()))

    robot.stop()
    robot.send()


if __name__ == "__main__":
    main()
