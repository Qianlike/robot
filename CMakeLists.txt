cmake_minimum_required(VERSION 3.0.2)
project(hightorque_robot)

# 设置安装路径变量
if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install path prefix" FORCE)
endif()

# 设置库版本
set(VERSION_MAJOR 4)
set(VERSION_MINOR 4)
set(VERSION_PATCH 7)
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

find_package(lcm REQUIRED)
find_package(serial REQUIRED)
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

add_library(hightorque_robot SHARED ${serial_SRCS})

# 将依赖项链接为PUBLIC，这样使用此库的项目也会自动链接这些依赖
target_link_libraries(hightorque_robot 
    PUBLIC 
        lcm 
        serial 
        yaml-cpp
    PRIVATE 
        rt 
        pthread 
        ${SERIALPORT_LIBRARY}
)
target_include_directories(${PROJECT_NAME} PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/crc>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/hardware>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/msg>
  $<INSTALL_INTERFACE:include>
  $<INSTALL_INTERFACE:include/crc>
  $<INSTALL_INTERFACE:include/hardware>
  $<INSTALL_INTERFACE:include/motor_msg>
)

# 设置版本和属性
set_target_properties(hightorque_robot PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${VERSION_MAJOR}
    PUBLIC_HEADER "include/hardware/robot.hpp;include/hardware/motor.hpp;include/hardware/canport.hpp;include/hardware/canboard.hpp;include/parse_robot_params.hpp;include/serial_struct.hpp;include/serial_driver.hpp;include/crc/crc8.hpp;include/crc/crc16.hpp;msg/motor_msg/motor_msg.hpp"
)

add_executable(motor_run example/motor_run.cpp)
target_link_libraries(motor_run hightorque_robot)

add_executable(motor_msg_subscriber example/motor_msg_subscriber.cpp)
target_link_libraries(motor_msg_subscriber hightorque_robot)

add_executable(parse_demo example/parse_demo.cpp)
target_link_libraries(parse_demo hightorque_robot)

# 安装目标
install(TARGETS hightorque_robot
    EXPORT hightorque_robotTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# 安装头文件
install(DIRECTORY include/ DESTINATION include 
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
install(DIRECTORY msg/motor_msg/ DESTINATION include/motor_msg 
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

# 安装配置文件模板
install(DIRECTORY robot_param/ DESTINATION share/hightorque_robot/robot_param 
    FILES_MATCHING PATTERN "*.yaml")

# 创建并安装配置文件
install(EXPORT hightorque_robotTargets
    FILE hightorque_robotTargets.cmake
    NAMESPACE hightorque_robot::
    DESTINATION lib/cmake/hightorque_robot
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/hightorque_robotConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/hightorque_robotConfig.cmake
    INSTALL_DESTINATION lib/cmake/hightorque_robot
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/hightorque_robotConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/hightorque_robotConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/hightorque_robotConfigVersion.cmake
    DESTINATION lib/cmake/hightorque_robot
)