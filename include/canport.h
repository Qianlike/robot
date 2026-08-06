
#include "serial_struct.h"
#include "serial/serial.h"
#include "common_macros.h"
#include "parse_robot_params.h"
#include "version.h"

#include <iostream>
#include <vector>
#include <thread>
#include <map>



typedef struct 
{
    uint8_t mode;
    uint8_t fault;

    float position;
    float velocity;
    float torque;

    uint32_t num;
    std::chrono::steady_clock::time_point time;

    version_s fw_version;
    std::string model;
    uint8_t flag;
} motor_state_t;


class canport
{
public:
    fdcan_state_s canport_state;
    std::map<int, motor_state_t> map_motors_state;

    canport(uint8_t _canport_id, std::string _ser_name, const RobotParams robot_params);
    canport(uint8_t _canport_id, std::initializer_list<int> id_list);
    ~canport();

    canport(const canport&) = delete;
    canport& operator=(const canport&) = delete;
    canport(canport&&) = delete;
    canport& operator=(canport&&) = delete;

    void position(uint8_t id, float pos);
    void velocity(uint8_t id, float vel);
    void turque(uint8_t id, float tqe);
    void vel_acc(uint8_t id, float vel, float acc);
    void pos_vel_acc(uint8_t id, float pos, float vel, float acc);
    void pos_vel_MAXtqe(uint8_t id, float pos, float vel, float max_tqe);
    void pos_vel_tqe_kp_kd(uint8_t id, float pos, float vel, float tqe, float kp, float kd);
    
    void stop();
    void stop(uint8_t id);

    void brake();
    void brake(uint8_t id);

    void request_motor_state(void);

    motor_state_t *get_motor_state(uint8_t id);

    void send(void);
    fdcan_state_s *get_canport_state(void);
    prot_cdc2comm_s *get_tdata(void);
private:
    version_s comm_version  // 通信板版本号
    {
        .data32 = 0,
    };
    version_s sdk_version   // SDK 版本号
    {
        .data32 = HIGHTORQUE_FDCAN_VERSION,
    };
    uint8_t canport_id = 0;

    prot_cdc2comm_s prot_tdata;
    serial::Serial ser_dev;
    std::thread ser_recv_thread;
    int id_max = 0;

    uint8_t comm_init_flag = 0;
    uint8_t set_cache_num_flag = 0;
    
    std::vector<std::string> get_ser_list(std::string serial_full_prefix);
    void ser_init(std::string port_name);
    void port_tdata_clean(const uint8_t mode, const uint16_t len);
    void comm_init(void);
    void get_comm_version();
    void set_cache_num(uint8_t num);

    uint16_t get_data_len(uint8_t mode, uint16_t num);
    void motor_tdata_clean(uint8_t cmd);

    void request_motor_version(void);
    void check_motor_version(void);

    void request_motor_model(void);
    void check_motor_model(void);

    void recv();
};

