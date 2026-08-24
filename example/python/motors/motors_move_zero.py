import signal
import time

from hightorque_robot import Robot


kVelocity = 0.314  # rad/s
kAcceleration = 3.14  # rad/s^2
exit_flag = False


def signal_handler(signum, frame):
    global exit_flag
    exit_flag = True


def main():
    kControlPeriod = 0.001
    signal.signal(signal.SIGINT, signal_handler)

    robot = Robot("../robot_param/robot_config.yaml")

    next_tick = time.monotonic()

    # 通过 robot.motors 以受限速度和加速度持续下发零位目标。
    while not exit_flag:
        for motor in robot.motors:
            motor.pos_vel_acc(0.0, kVelocity, kAcceleration)
        robot.send()

        for motor in robot.motors:
            state = motor.get_motor_state()
            if state is not None:
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
