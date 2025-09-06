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

# include_directories(
#   include
#   include/crc
#   include/hardware
#   msg/motor_msg
# )

add_library(hightorque_robot SHARED ${serial_SRCS})
target_link_libraries(hightorque_robot rt pthread ${SERIALPORT_LIBRARY} serial lcm yaml-cpp)
target_include_directories(${PROJECT_NAME} PUBLIC 
  # ${CMAKE_CURRENT_SOURCE_DIR}/include 
  # ${CMAKE_CURRENT_SOURCE_DIR}/msg
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/crc>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/hardware>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/msg/motor_msg>
  $<INSTALL_INTERFACE:include>
)

# 设置版本和属性
set_target_properties(hightorque_robot PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${VERSION_MAJOR}
    PUBLIC_HEADER "include/hardware/robot.hpp;include/hardware/motor.hpp;include/hardware/canport.hpp;include/hardware/canboard.hpp;msg/motor_msg/motor_msg.hpp"
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
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY msg/motor_msg/ DESTINATION include/motor_msg)

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