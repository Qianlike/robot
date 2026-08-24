import signal
import time

from hightorque_robot import CanPort


exit_flag = False


def signal_handler(signum, frame):
    global exit_flag
    exit_flag = True


def main():
    kControlPeriod = 0.001
    signal.signal(signal.SIGINT, signal_handler)

    can_port = CanPort(1, [1])

    next_tick = time.monotonic()

    # 只查询并打印反馈，不下发任何电机控制指令。
    while not exit_flag:
        can_port.request_motor_state()

        for item in can_port.map_motors_state.items():
            id = item[0]
            motor = item[1]
            print(
                "ID: {:2d}, mode: {:2d}, fault: {:2d}, pos: {:2.3f}, "
                "vel: {:2.3f}, tor: {:2.3f}".format(
                    int(id),
                    motor.mode,
                    motor.fault,
                    motor.position,
                    motor.velocity,
                    motor.torque,
                )
            )

        next_tick += kControlPeriod
        time.sleep(max(0.0, next_tick - time.monotonic()))

    can_port.stop()
    can_port.send()


if __name__ == "__main__":
    main()
