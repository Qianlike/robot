#ifndef HIGHTORQUE_CONVERT_H
#define HIGHTORQUE_CONVERT_H


#include "common_macros.h"
#include <stdint.h>
#include <string>


#define  MY_2PI  (6.28318530717f)
#define  MY_PI   (3.14159265358f)

#define  POS_SCALE   (10000.0f)
#define  VEL_SCALE   (4000.0f)
#define  TQE_SCALE   (100.0f)
#define  ACC_SCALE   (1000.0f)
#define  PID_SCALE   (10.0f)


typedef enum
{
    RADIAN_2PI = 0, // 弧度
    ANGLE_360,      // 角度
    TURNS           // 圈数
} angle_unit_t;


#define ANGLE_UNIT  RADIAN_2PI



int16_t pos_float2int(float data);
int16_t vel_float2int(float data);
int16_t tqe_float2int(float data);
int16_t acc_float2int(float data);
int16_t kp_float2int(float data);
int16_t kd_float2int(float data);


float pos_int2float(int16_t data);
float vel_int2float(int16_t data);
float tqe_int2float(int16_t data);


#endif
