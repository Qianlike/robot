import time

from hightorque_robot import CanPort


def main():
    kControlPeriod = 0.001
    can_port = CanPort(1, [1])

    can_port.motor_zero_pos_reset()

    next_tick = time.monotonic()
    while True:
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
