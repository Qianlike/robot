cmake_minimum_required(VERSION 3.5...3.30)
project(hightorque_fdcan)

# ==================== 基础设置 ====================
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# 生成 compile_commands.json（方便 IDE / clangd）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==================== 禁用 yaml-cpp 多余功能 ====================
# 加快编译，避免生成测试和工具
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable yaml-cpp tests" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable yaml-cpp tools" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Disable yaml-cpp contrib" FORCE)
# 推荐使用静态库（嵌入方便，避免运行时找不到 .so）
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

# ==================== 可执行文件 ====================
set(DEMO_FILES
    example/demo1.cpp
    example/demo2.cpp
)

find_package(Threads REQUIRED)

foreach(demo_file ${DEMO_FILES})
    get_filename_component(demo_name ${demo_file} NAME_WE)

    add_executable(${demo_name}
        ${SOURCE_FILES}
        ${demo_file}
    )

    target_include_directories(${demo_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/serial/include
    )

    target_link_libraries(${demo_name} PRIVATE
        serial
        yaml-cpp
        Threads::Threads
    )
endforeach()

