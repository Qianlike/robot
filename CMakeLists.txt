cmake_minimum_required(VERSION 3.5...3.30)
project(hightorque_fdcan)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ==================== 禁用 yaml-cpp 多余功能 ====================
# 加快编译，避免生成测试和工具
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable yaml-cpp tests" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable yaml-cpp tools" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Disable yaml-cpp contrib" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Build yaml-cpp as static library" FORCE)

# ==================== 添加子目录 ====================
add_subdirectory(lib/serial)
add_subdirectory(lib/yaml-cpp)

# ==================== 源文件 ====================
set(SOURCE_FILES
    src/robot.cpp
    src/canport.cpp
    src/motor.cpp
    src/crc.cpp
    src/convert.cpp
    src/parse_robot_params.cpp
)

# ==================== 静态库 ====================
# 外部项目（如 ROS 包）add_subdirectory 引入本目录后，
# 只需 target_link_libraries(... hightorque_robot) 即可（include 与依赖通过 PUBLIC 传递）
add_library(hightorque_robot STATIC ${SOURCE_FILES})

target_compile_features(hightorque_robot PUBLIC cxx_std_11)

target_include_directories(hightorque_robot PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/serial/include
)

find_package(Threads REQUIRED)

target_link_libraries(hightorque_robot PUBLIC
    serial
    yaml-cpp
    Threads::Threads
)

# ==================== 可执行文件 ====================
# 被外部项目（如 ROS 包）add_subdirectory 引入时可设 OFF，跳过 demo 构建
option(HIGHTORQUE_BUILD_DEMO "Build demo executables" ON)

set(DEMO_FILES
    example/canport/canport_feedback.cpp
    example/canport/canport_move_zero.cpp
    example/canport/canport_run.cpp
    example/canport/canport_set_zero.cpp
    example/motors/motors_feedback.cpp
    example/motors/motors_move_zero.cpp
    example/motors/motors_run.cpp
    example/motors/motors_set_zero.cpp
)

if(HIGHTORQUE_BUILD_DEMO)
    foreach(demo_file ${DEMO_FILES})
        get_filename_component(demo_name ${demo_file} NAME_WE)

        add_executable(${demo_name}
            ${demo_file}
        )

        target_link_libraries(${demo_name} PRIVATE
            hightorque_robot
        )
    endforeach()
endif()
