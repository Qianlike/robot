#include "canport.h"
#include "crc.h"
#include "convert.h"

#include <algorithm>
#include <string>



static std::vector<std::string> ser_list{};

canport::canport(uint8_t _canport_id, std::string _ser_name, const RobotParams robot_params)
{
    PRINT_INFO_G("FDCAN_PASS version: %d.%d.%d", sdk_version.major, sdk_version.minor, sdk_version.patch);
    canport_id = _canport_id;

    ser_init(_ser_name);
    get_comm_version();
    if (comm_version.data32 < VER_COMBINE(6, 0, 0))
    {
        PRINT_ERROR("The communication board version is too low!!!");
        exit(1);
    }

    const int motor_num = robot_params.canports[canport_id - 1].motor_num;
    if (motor_num < 1 || motor_num > 30)
    {
        PRINT_ERROR("canport%d motor_num err!! got %d, valid range [1, 30]",
                    canport_id, motor_num);
        exit(1);
    }

    comm_init();
    set_cache_num(static_cast<uint8_t>(motor_num));
}


canport::canport(uint8_t _canport_id, std::initializer_list<int> _id_list)
{
    if (_canport_id < 1)
    {
        PRINT_ERROR("_canport_id err!! got %d, must be >= 1", _canport_id);
        exit(1);
    }
    canport_id = _canport_id;

    if (_id_list.size() == 0)
    {
        PRINT_ERROR("_id_list err!! id list is empty");
        exit(1);
    }

    for (auto id : _id_list)
    {
        if (id < 1 || id > 30)
        {
            PRINT_ERROR("_id_list err!! got %d, valid range [1, 30]", id);
            exit(1);
        }
        if (id_max < id)
        {
            id_max = id;
        }
        map_motors_state.insert({id, {}});
    }

    if (ser_list.size() == 0)
    {
        ser_list = get_ser_list("/dev/ttyACM");
        if (ser_list.size() < 1)
        {
            PRINT_ERROR("No serial ports found for prefix: /dev/ttyACM");
            exit(1);
        }
    }

    if (_canport_id > ser_list.size())
    {
        PRINT_ERROR("Not enough serial ports: found %zu, need %d (canport_id)", ser_list.size(), _canport_id);
        exit(1);
    }
    PRINT_INFO_G("canport%d init", canport_id);
    ser_init(ser_list[canport_id - 1]);
    get_comm_version();
    if (comm_version.data32 < VER_COMBINE(6, 0, 0))
    {
        PRINT_ERROR("The communication board version is too low!!!");
        exit(1);
    }

    comm_init();
    set_cache_num(map_motors_state.size());

    check_motor_version();
    check_motor_model();

    request_motor_state();
    request_motor_state();

    PRINT_INFO_G("canport%d init ok\n", canport_id);
}

canport::~canport()
{
    PRINT_INFO_G("canport%d close", canport_id);
    ser_dev.close();

    if (ser_recv_thread.joinable())
    {
        ser_recv_thread.join();
    }
}


void canport::port_tdata_clean(const uint8_t mode, const uint16_t len)
{
    if (prot_tdata.head.s.cmd != mode)
    {
        prot_tdata.head.s.head = PROT_HEAD;
        prot_tdata.head.s.cmd = mode;
        prot_tdata.head.s.len = len;
        memset(&prot_tdata.data, 0, len);
    }
}


std::vector<std::string> canport::get_ser_list(std::string serial_full_prefix)
{
    std::vector<serial::PortInfo> all_ports = serial::list_ports();
    std::vector<std::string> com_board_ports;

    for (const auto& port_info : all_ports)
    {
        if (port_info.port.find(serial_full_prefix) == std::string::npos)
        {
            continue;
        }

        std::string hw = port_info.hardware_id;
        auto pos = hw.find("VID:PID=");
        if (pos == std::string::npos)
        {
            continue;
        }

        std::string vid_pid = hw.substr(pos + 8);
        auto colon = vid_pid.find(':');
        if (colon == std::string::npos)
        {
            continue;
        }

        std::string vid = vid_pid.substr(0, colon);
        std::string pid_part = vid_pid.substr(colon + 1);
        auto space = pid_part.find(' ');
        if (space != std::string::npos)
        {
            pid_part = pid_part.substr(0, space);
        }
        std::string pid = pid_part;

        std::transform(vid.begin(), vid.end(), vid.begin(), ::toupper);
        std::transform(pid.begin(), pid.end(), pid.begin(), ::toupper);
        if (vid == "CAF1" && pid == "FFFF")
        {
            
            com_board_ports.push_back(port_info.port);
        }
    }

    if (com_board_ports.empty())
    {
        PRINT_ERROR("No serial ports found for prefix: %s", serial_full_prefix.c_str());
        exit(1);
    }

    PRINT_INFO("Detected serial ports: ");
    for (const auto& port_name : com_board_ports)
    {
        PRINT_INFO("%s", port_name.c_str());
    }

    return com_board_ports;
}


void canport::ser_init(std::string port_name)
{
    PRINT_INFO_G("port_name: %s", port_name.c_str());

    ser_dev.setPort(port_name.c_str());
    ser_dev.setBaudrate(4000000);
    serial::Timeout timeout = serial::Timeout::simpleTimeout(100);
    ser_dev.setTimeout(timeout);

    try
    {
        ser_dev.open();
    }
    catch(const std::exception& e)
    {
        PRINT_ERROR("serial open err!!!");
        exit(1);
    }
    
    PRINT_INFO_G("Serial Port %s initialized", port_name.c_str());
    ser_recv_thread = std::thread(&canport::recv, this);
}


void canport::get_comm_version()
{
    port_tdata_clean(MODE_COMM_VERSION, 0);

    for (int i = 0; i < 5; i++)
    {
        send();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        if (comm_version.data32 >= VER_COMBINE(3, 0, 0))
        {
            PRINT_INFO_G("board version: v%d.%d.%d", comm_version.major, comm_version.minor, comm_version.patch);
            return;
        }
    }

    PRINT_ERROR("Failed to retrieve the communication board version number!!!");
    exit(1);
}


void canport::comm_init()
{
    port_tdata_clean(MODE_COMM_INIT, 0);

    prot_tdata.head.s.len = 0;

    comm_init_flag = 0;
    for (int i = 0; i < 5; i++)
    {
        send();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        if (comm_init_flag != 0)
        {
            PRINT_INFO_G("comm init");
            return;
        }
    }

    PRINT_ERROR("comm init error!");
    exit(1);
}


void canport::set_cache_num(uint8_t num)
{
    port_tdata_clean(MODE_CACHE_NUM, 1);

    prot_tdata.data.raw[0] = num;

    set_cache_num_flag = 0;
    for (int i = 0; i < 5; i++)
    {
        send();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        if (set_cache_num_flag != 0)
        {
            PRINT_INFO_G("set rdata max num");
            return;
        }
    }

    PRINT_ERROR("set rdata max num");
    exit(1);
}


uint16_t canport::get_data_len(uint8_t mode, uint16_t num)
{
    uint8_t motor_one_len = 0;
    switch (mode)
    {
        case MODE_STOP:
        case MODE_BRAKE:
            motor_one_len = 1;
            break;
        case MODE_POSITION:
        case MODE_VELOCITY:
        case MODE_TORQUE:
        case MODE_VOLTAGE:
        case MODE_CURRENT:
            motor_one_len = 2;
            break;
        case MODE_VEL_ACC:
            motor_one_len = 4;
            break;
        case MODE_POS_VEL_TQE:
        case MODE_POS_VEL_ACC:
            motor_one_len = 6;
            break;
        case MODE_POS_VEL_TQE_KP_KD:
            motor_one_len = 10;
            break;
        default:
            motor_one_len = 0;
            PRINT_ERROR("This mode has beenThis mode is deprecated.");
            exit(0);
    }

    uint8_t fdcan_one_len = 60;
    const uint16_t len = motor_one_len * num;
    const uint8_t mul = len / fdcan_one_len;
    const uint8_t rem = len % fdcan_one_len;
    uint8_t rem_len = 0;

    if (rem <= 7)
    {
        rem_len = rem;
    }
    else if (rem <= 11)
    {
        rem_len = 11;
    }
    else if (rem <= 15)
    {
        rem_len = 15;
    }
    else if (rem <= 19)
    {
        rem_len = 19;
    }
    else if (rem <= 23)
    {
        rem_len = 23;
    }
    else if (rem <= 31)
    {
        rem_len = 31;
    }
    else if (rem <= 47)
    {
        rem_len = 47;
    }
    else 
    {
        rem_len = fdcan_one_len;
    }

    return mul * fdcan_one_len + rem_len + 2;
}


void canport::motor_tdata_clean(uint8_t cmd)
{
    if (prot_tdata.head.s.cmd != cmd)
    {
        prot_tdata.head.s.head = PROT_HEAD;
        prot_tdata.head.s.cmd = cmd;
        prot_tdata.head.s.len = get_data_len(cmd, id_max);

        if (cmd >= MODE_STOP && cmd <= MODE_BRAKE)
        {
            for (int i = 0; i < id_max; i++)
            {
                prot_tdata.data.contr.raw8[i] = 0;
            }
        }
        else
        {
            for (int i = 0; i < id_max; i++)
            {
                prot_tdata.data.contr.raw16[i] = 0x8000;
            }
        }
        prot_tdata.data.contr.data_type = TINT16_NOHDR;
        prot_tdata.data.contr.query = QUERY_MODE_FAULT_POS_VEL_TQE;
    }
}


void canport::position(uint8_t id, float pos)
{
    motor_tdata_clean(MODE_POSITION);
    prot_tdata.data.contr.raw16[id -1] = pos_float2int(pos);
}


void canport::velocity(uint8_t id, float vel)
{
    motor_tdata_clean(MODE_VELOCITY);
    prot_tdata.data.contr.raw16[id -1] = vel_float2int(vel);
}


void canport::turque(uint8_t id, float tqe)
{
    motor_tdata_clean(MODE_TORQUE);
    prot_tdata.data.contr.raw16[id -1] = tqe_float2int(tqe);
}


void canport::vel_acc(uint8_t id, float vel, float acc)
{
    motor_tdata_clean(MODE_VEL_ACC);
    prot_tdata.data.contr.va[id -1].vel = vel_float2int(vel);
    prot_tdata.data.contr.va[id -1].acc = acc_float2int(acc);
}


void canport::pos_vel_acc(uint8_t id, float pos, float vel, float acc)
{
    motor_tdata_clean(MODE_POS_VEL_ACC);
    prot_tdata.data.contr.pva[id -1].pos = pos_float2int(pos);
    prot_tdata.data.contr.pva[id -1].vel = vel_float2int(vel);
    prot_tdata.data.contr.pva[id -1].acc = acc_float2int(acc);
}


void canport::pos_vel_MAXtqe(uint8_t id, float pos, float vel, float max_tqe)
{   
    motor_tdata_clean(MODE_POS_VEL_TQE);
    prot_tdata.data.contr.pvt[id -1].pos = pos_float2int(pos);
    prot_tdata.data.contr.pvt[id -1].vel = vel_float2int(vel);
    prot_tdata.data.contr.pvt[id -1].tqe = tqe_float2int(max_tqe);
}   


void canport::pos_vel_tqe_kp_kd(uint8_t id, float pos, float vel, float tqe, float kp, float kd)
{
    motor_tdata_clean(MODE_POS_VEL_TQE_KP_KD);
    prot_tdata.data.contr.pvtpd[id -1].pos = pos_float2int(pos);
    prot_tdata.data.contr.pvtpd[id -1].vel = vel_float2int(vel);
    prot_tdata.data.contr.pvtpd[id -1].tqe = tqe_float2int(tqe);
    prot_tdata.data.contr.pvtpd[id -1].kp = kp_float2int(kp);
    prot_tdata.data.contr.pvtpd[id -1].kd = kd_float2int(kd);
}


void canport::stop(uint8_t id)
{
    motor_tdata_clean(MODE_STOP);
    prot_tdata.data.contr.raw8[id - 1] = 1;
}


void canport::stop()
{
    motor_tdata_clean(MODE_STOP);
    for (auto it : map_motors_state)
    {
        prot_tdata.data.contr.raw8[it.first - 1] = 1;
    }
}


void canport::reset(uint8_t id)
{
    motor_tdata_clean(MODE_RESET);
    prot_tdata.data.contr.raw8[id - 1] = 1;
}


void canport::reset()
{
    motor_tdata_clean(MODE_RESET);
    for (auto it : map_motors_state)
    {
        prot_tdata.data.contr.raw8[it.first - 1] = 1;
    }
}


void canport::brake(uint8_t id)
{
    motor_tdata_clean(MODE_BRAKE);
    prot_tdata.data.contr.raw8[id - 1] = 1;
}


void canport::brake()
{
    motor_tdata_clean(MODE_BRAKE);
    for (auto it : map_motors_state)
    {
        prot_tdata.data.contr.raw8[it.first - 1] = 1;
    }
}


void canport::request_motor_version()
{
    port_tdata_clean(MODE_MOTOR_VERSION, 0);
    send();
}


void canport::request_motor_state()
{
    port_tdata_clean(MODE_MOTOR_STATE, 2);
    prot_tdata.data.contr.data_type = TINT16_NOHDR;
    prot_tdata.data.contr.query = QUERY_MODE_FAULT_POS_VEL_TQE;
    send();
}


void canport::request_pos_reset()
{
    port_tdata_clean(MODE_MOTOR_POS_RESET, id_max);

    for (auto it : map_motors_state)
    {
        prot_tdata.data.raw[it.first - 1] = 1;
    }

    send();
}

void canport::check_motor_pos_reset()
{
    std::vector<uint8_t> failed_id_list;

    for (auto it : map_motors_state)
    {
        it.second.flag = 0;
    }

    for (int i = 0; i < 10; i++)
    {
        request_pos_reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        failed_id_list.clear();
        for (auto it : map_motors_state)
        {
            if (it.second.flag == 0)
            {
                failed_id_list.push_back(it.first);
            }
        }

        printf("size = %ld\n", failed_id_list.size());
        if (failed_id_list.size() == 0)
        {
            for (auto it : map_motors_state)
            {
                // PRINT_INFO("canport%d motor%d version = v%d.%d.%d", canport_id, it.first, 
                //     it.second.fw_version.major, it.second.fw_version.minor, it.second.fw_version.patch);
                PRINT_INFO_G("pos resset ok");
            }
            return;
        }
    }

    PRINT_ERROR("canport%d error", canport_id);
    for (auto it: failed_id_list)
    {
        PRINT_ERROR("canport%d motor%d", canport_id, it);
    }
}


void canport::check_motor_version()
{
    std::vector<uint8_t> failed_id_list;

    for (int i = 0; i < 100; i++)
    {
        request_motor_version();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        failed_id_list.clear();
        for (auto it : map_motors_state)
        {
            if (it.second.fw_version.data32 == 0)
            {
                failed_id_list.push_back(it.first);
            }
        }

        printf("size = %ld\n", failed_id_list.size());
        if (failed_id_list.size() == 0)
        {
            for (auto it : map_motors_state)
            {
                PRINT_INFO("canport%d motor%d version = v%d.%d.%d", canport_id, it.first, 
                    it.second.fw_version.major, it.second.fw_version.minor, it.second.fw_version.patch);
            }
            return;
        }
    }

    PRINT_ERROR("canport%d error", canport_id);
    for (auto it: failed_id_list)
    {
        PRINT_ERROR("canport%d motor%d", canport_id, it);
    }
}


void canport::request_motor_model()
{
    port_tdata_clean(MODE_MOTOR_MODEL, 0);
    send();
}


void canport::check_motor_model()
{
    std::vector<uint8_t> failed_id_list;

    for (int i = 0; i < 100; i++)
    {
        request_motor_model();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        failed_id_list.clear();
        for (auto it : map_motors_state)
        {
            if (it.second.model.size() == 0)
            {
                failed_id_list.push_back(it.first);
            }
        }

        printf("size = %ld\n", failed_id_list.size());
        if (failed_id_list.size() == 0)
        {
            for (auto it : map_motors_state)
            {
                PRINT_INFO("canport%d motor%d model = %s", canport_id, it.first, 
                    it.second.model.c_str());
            }
            return;
        }
    }

    PRINT_ERROR("canport%d error", canport_id);
    for (auto it: failed_id_list)
    {
        PRINT_ERROR("canport%d motor%d", canport_id, it);
    }
}


motor_state_t *canport::get_motor_state(uint8_t id)
{
    auto it = map_motors_state.find(id);
    if (it == map_motors_state.end())
    {
        nullptr;
    }

    return &(it->second);
}


fdcan_state_s *canport::get_canport_state()
{
    return &canport_state;
}


prot_cdc2comm_s *canport::get_tdata()
{
    return &prot_tdata;
}


void canport::send()
{
    prot_tdata.head.s.crc_head = crc8_ccitt(&(prot_tdata.head.raw[1]), 3);
    prot_tdata.head.s.crc_data = crc8_ccitt(&(prot_tdata.data.raw[0]), prot_tdata.head.s.len);

    #if 0
    uint8_t *byte_ptr = (uint8_t *)&prot_tdata.head.s.head;
    printf("send:\n");
    for (size_t i = 0; i < prot_tdata.head.s.len + 6; i++)
    {
        printf("0x%.2X ", byte_ptr[i]);
    }
    printf("\n\n");
    #endif

    try
    {
        if(ser_dev.isOpen())
        {
            ser_dev.write((const uint8_t *)&prot_tdata.head.s.head, prot_tdata.head.s.len + sizeof(prot_head_s));
        }
    }
    catch(const std::exception& e)
    {
        PRINT_ERROR("serial send err: %s", e.what());
        ser_dev.close();
    }
}


void canport::recv()
{
    prot_comm2cdc_s prot_rdata = {0};

    while(ser_dev.isOpen())
    {
        prot_rdata.head.s.head = 0;
        prot_rdata.head.s.cmd = 0;
        prot_rdata.head.s.len = 0;
        try
        {  
            ser_dev.read(&prot_rdata.head.raw[0], 1);
            if (prot_rdata.head.s.head != PROT_HEAD)
            {
                continue;
            }

            ser_dev.read(&prot_rdata.head.raw[1], 4);
            if (prot_rdata.head.s.crc_head != crc8_ccitt(&prot_rdata.head.raw[1], 3))
            {
                continue;
            }

            ser_dev.read(&prot_rdata.head.raw[5], prot_rdata.head.s.len + 1);
            if (prot_rdata.head.s.crc_data != crc8_ccitt(&prot_rdata.data.raw[0], prot_rdata.head.s.len))
            {
                continue;
            }

            #if 0
            printf("cmd %02X, len = %d   ", prot_rdata.head.s.cmd, prot_rdata.head.s.len);
            for (int i = 0; i < prot_rdata.head.s.len; i++)
            {
                printf("0x%02X ", prot_rdata.data.raw[i]);
            }
            printf("\n");
            #endif

            canport_state = prot_rdata.data.s.fdcan_state;
            if (canport_state.fault > FDCAN_STATUS_ERROR_WARNING || canport_state.fault == FDCAN_STATUS_UNKNOWN)
            {
                PRINT_ERROR("canport[%d] flaut = %d, rx = %d, tx = %d", canport_id, canport_state.fault, canport_state.rx_err_num, canport_state.tx_err_num);
            }
            else if (canport_state.fault == FDCAN_STATUS_ERROR_WARNING)
            {
                PRINT_INFO("canport[%d] flaut = %d, rx = %d, tx = %d", canport_id, canport_state.fault, canport_state.rx_err_num, canport_state.tx_err_num);
            }

            switch (prot_rdata.head.s.cmd)
            {
            case MODE_COMM_VERSION:
                comm_version.major = prot_rdata.data.s.version.major;
                comm_version.minor = prot_rdata.data.s.version.minor;
                comm_version.patch = prot_rdata.data.s.version.patch;
                break;
            case MODE_COMM_INIT:
                comm_init_flag = prot_rdata.data.s.raw[0];
                break;
            case MODE_CACHE_NUM:
                set_cache_num_flag = prot_rdata.data.s.raw[0];
                break;
            case MODE_MOTOR_STATE:
                switch (prot_rdata.data.s.motor_state.query)
                {
                case QUERY_MODE_FAULT_POS_VEL_TQE:
                    for (int i = 0; i < (prot_rdata.head.s.len - 1 - 3) / sizeof(query_mode_fault_pos_vel_tqe_t); i++)
                    {
                        auto it = map_motors_state.find(prot_rdata.data.s.motor_state.mfpvt[i].id);
                        if (it != map_motors_state.end())
                        {
                            it->second.mode = prot_rdata.data.s.motor_state.mfpvt[i].mode;
                            it->second.fault = prot_rdata.data.s.motor_state.mfpvt[i].fault;

                            it->second.position = pos_int2float(prot_rdata.data.s.motor_state.mfpvt[i].pos);
                            it->second.velocity = vel_int2float(prot_rdata.data.s.motor_state.mfpvt[i].vel);
                            it->second.torque = tqe_int2float(prot_rdata.data.s.motor_state.mfpvt[i].tqe);
                            it->second.num++;
                            it->second.time = std::chrono::steady_clock::now();
                        }
                    }
                    break;
                }
                break;
            case MODE_MOTOR_VERSION:
                for (int i = 0; i < (prot_rdata.head.s.len - 3) / sizeof(prot_motor_version_t); i++)
                {
                    auto it = map_motors_state.find(prot_rdata.data.s.motor_version[i].id);
                    if (it != map_motors_state.end())
                    {
                        it->second.fw_version.major = prot_rdata.data.s.motor_version->major;
                        it->second.fw_version.minor = prot_rdata.data.s.motor_version->minor;
                        it->second.fw_version.patch = prot_rdata.data.s.motor_version->patch;
                    }
                }
                break;
            case MODE_MOTOR_MODEL:
                for (int i = 0; i < (prot_rdata.head.s.len - 3) / sizeof(prot_motor_model_t); i++)
                {
                    auto it = map_motors_state.find(prot_rdata.data.s.motor_model[i].id);
                    if (it != map_motors_state.end())
                    {
                        it->second.model = std::string(prot_rdata.data.s.motor_model[i].data, prot_rdata.data.s.motor_model[i].len);
                    }
                }
                break;
            case MODE_MOTOR_POS_RESET:
                for (int i = 0; i < (prot_rdata.head.s.len - 3) / sizeof(prot_motor_flag_t); i++)
                {
                    auto it = map_motors_state.find(prot_rdata.data.s.motor_flag[i].id);
                    if (it != map_motors_state.end())
                    {
                        it->second.flag = !prot_rdata.data.s.motor_flag[i].flag;
                    }
                }
                break;
            default:
                break;
            }
        }
        catch(const std::exception& e)
        {
            ser_dev.close();
        }
    }
}

