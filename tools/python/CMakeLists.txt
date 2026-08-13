cmake_minimum_required(VERSION 3.16...3.30)
project(hightorque_robot_python LANGUAGES CXX)

# 不构建根工程的 demo 可执行文件
set(HIGHTORQUE_BUILD_DEMO OFF CACHE BOOL "" FORCE)

# ==================== 核心库 ====================
# 双路回退：
#   - cibuildwheel 容器只包含本项目目录，用构建脚本 rsync 的 core/ 副本
#   - 本地 pip install ./tools/python 直接引用仓库根
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/CMakeLists.txt")
    add_subdirectory(core hightorque_fdcan_core EXCLUDE_FROM_ALL)
else()
    add_subdirectory(../.. hightorque_fdcan_core EXCLUDE_FROM_ALL)
endif()

# ==================== Python 绑定 ====================
# scikit-build-core 会自动向 find_package 注入构建环境里的 pybind11_DIR
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_core src/python_bindings.cpp)

# 说明：不用 abi3/Py_LIMITED_API —— pybind11 从未真正支持 limited API 编译
# （pybind/pybind11#1755），因此按 Python 版本分别构建 wheel
# （cp38~cp314 各一个，pip 自动选择匹配版本，对用户透明）

target_link_libraries(_core PRIVATE hightorque_robot)
target_compile_features(_core PRIVATE cxx_std_17)

# 把扩展装进 hightorque_robot 包内（默认会装到 site-packages 根，
# 导致 from . import _core 找不到）
install(TARGETS _core LIBRARY DESTINATION hightorque_robot)

# 内置参数数据用 CMake install 打进 wheel（不依赖 wheel.packages 的文件过滤，
# 该过滤在 scikit-build-core 1.x 下会漏掉 .yaml）。
# 单一数据源：仓库根 robot_param/（core 分支为 staging 副本），不在此维护拷贝。
# 注意：原始 robot_config.yaml 的 param_file 为相对路径，C++ 端按 config 所在
# 目录解析，在 wheel 内归一化后同样指向 robot_param/ 目录，无需修改版。
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/robot_param")
    set(HIGHTORQUE_ROBOT_PARAM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/core/robot_param")
else()
    set(HIGHTORQUE_ROBOT_PARAM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../robot_param")
endif()
install(DIRECTORY ${HIGHTORQUE_ROBOT_PARAM_DIR}
        DESTINATION hightorque_robot
        FILES_MATCHING PATTERN "*.yaml")
