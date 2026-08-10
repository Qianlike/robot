#include "convert.h"
#include <cstdlib>
#include <string>


static int16_t int16_limit(const int32_t data, const std::string &str)
{
    if (data >= 32760)
    {
        PRINT_ERROR("%s: data=%d out of range [-32760, 32760], clamped", str.c_str(), data);
        return static_cast<int16_t>(32760);
    }
    else if (data <= -32760)
    {
        PRINT_ERROR("%s: data=%d out of range [-32760, 32760], clamped", str.c_str(), data);
        return static_cast<int16_t>(-32760);
    }

    return static_cast<int16_t>(data);
}

static float conv_to_turns(const float in_data, const angle_unit_t type)
{
    switch (type)
    {
    case RADIAN_2PI:
        return in_data / MY_2PI;
    case ANGLE_360:
        return in_data / 360.0f;
    case TURNS:
        return in_data;
    default:
        PRINT_ERROR("conv_to_turns: invalid type=%d", static_cast<int>(type));
        exit(1);
        return 0.0f;
    }
}

static float turns_to_conv(const float in_data, const angle_unit_t type)
{
    switch (type)
    {
    case RADIAN_2PI:
        return in_data * MY_2PI;
    case ANGLE_360:
        return in_data * 360.0f;
    case TURNS:
        return in_data;
    default:
        PRINT_ERROR("turns_to_conv: invalid type=%d", static_cast<int>(type));
        exit(1);
        return 0.0f;
    }
}


int16_t pos_float2int(float data)
{
    data = conv_to_turns(data, static_cast<angle_unit_t>(ANGLE_UNIT));
    return int16_limit(static_cast<int32_t>(data * POS_SCALE), "pos");
}

float pos_int2float(int16_t data)
{
    const float turns = static_cast<float>(data) / POS_SCALE;
    return turns_to_conv(turns, static_cast<angle_unit_t>(ANGLE_UNIT));
}


int16_t vel_float2int(float data)
{
    data = conv_to_turns(data, static_cast<angle_unit_t>(ANGLE_UNIT));
    return int16_limit(static_cast<int32_t>(data * VEL_SCALE), "vel");
}

float vel_int2float(int16_t data)
{
    const float turns = static_cast<float>(data) / VEL_SCALE;
    return turns_to_conv(turns, static_cast<angle_unit_t>(ANGLE_UNIT));
}


int16_t tqe_float2int(float data)
{
    data = conv_to_turns(data, static_cast<angle_unit_t>(ANGLE_UNIT));
    return int16_limit(static_cast<int32_t>(data * TQE_SCALE), "tqe");
}

float tqe_int2float(int16_t data)
{
    const float turns = static_cast<float>(data) / TQE_SCALE;
    return turns_to_conv(turns, static_cast<angle_unit_t>(ANGLE_UNIT));
}

int16_t acc_float2int(float data)
{
    data = conv_to_turns(data, static_cast<angle_unit_t>(ANGLE_UNIT));
    return int16_limit(static_cast<int32_t>(data * ACC_SCALE), "acc");
}

int16_t kp_float2int(float data)
{
    return int16_limit(static_cast<int32_t>(data * PID_SCALE), "kp");
}

int16_t kd_float2int(float data)
{
    return int16_limit(static_cast<int32_t>(data * PID_SCALE), "kd");
}

