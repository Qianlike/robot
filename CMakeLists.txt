cmake_minimum_required(VERSION 3.0.2)
project(hightorque_robot)

# Find catkin
add_subdirectory(lib/serial_cmake)
add_subdirectory(lib/lcm)
find_package(yaml-cpp REQUIRED)



set(rt_LIBRARIES rt)
set(pthread_LIBRARIES pthread)
set(serial_SRCS
    src/serial_driver.cpp
    src/parse_robot_params.cpp
    src/hardware/motor.cpp
    src/hardware/canport.cpp
    src/hardware/canboard.cpp
    src/hardware/robot.cpp
    src/crc/crc8.cpp
    src/crc/crc16.cpp
)

find_library(SERIALPORT_LIBRARY serialport)

if(NOT SERIALPORT_LIBRARY)
  message(FATAL_ERROR "libserialport not found")
endif()

include_directories(
  include
  include/crc
  include/hardware
  msg/motor_msg
)

add_library(hightorque_robot SHARED ${serial_SRCS})
target_link_libraries(hightorque_robot rt pthread ${SERIALPORT_LIBRARY} serial lcm yaml-cpp)
target_include_directories(${PROJECT_NAME} PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/include 
  ${CMAKE_CURRENT_SOURCE_DIR}/msg)

add_executable(motor_run example/motor_run.cpp)
target_link_libraries(motor_run hightorque_robot)

add_executable(motor_msg_subscriber example/motor_msg_subscriber.cpp)
target_link_libraries(motor_msg_subscriber hightorque_robot)

