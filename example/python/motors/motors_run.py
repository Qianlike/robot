import signal
import time

from hightorque_robot import Robot


exit_flag = False


def signal_handler(signum, frame):
    global exit_flag
    exit_flag = True


def main():
    kControlPeriod = 0.001
    signal.signal(signal.SIGINT, signal_handler)

    robot = Robot("../robot_param/robot_config.yaml")

    pos = 0.314
    tick = time.monotonic()
    next_tick = time.monotonic()
    while not exit_flag:
        if time.monotonic() - tick >= 1.0:
            tick = time.monotonic()
            pos *= -1

        for motor in robot.motors:
            motor.pos_vel_acc(pos, 0.314, 3.14)
        robot.send()

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
