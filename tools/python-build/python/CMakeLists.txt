cmake_minimum_required(VERSION 3.16...3.30)
project(hightorque_robot_python LANGUAGES CXX)

# 不构建根工程的 demo 可执行文件
set(HIGHTORQUE_BUILD_DEMO OFF CACHE BOOL "" FORCE)

# ==================== 核心库 ====================
# 双路回退：
#   - cibuildwheel 容器只包含本项目目录，用构建脚本 rsync 的 core/ 副本
#   - 本地 pip install ./tools/python-build/python 直接引用仓库根
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/CMakeLists.txt")
    add_subdirectory(core hightorque_fdcan_core EXCLUDE_FROM_ALL)
else()
    add_subdirectory(../../.. hightorque_fdcan_core EXCLUDE_FROM_ALL)
endif()

# ==================== Python 绑定 ====================
# scikit-build-core 会自动向 find_package 注入构建环境里的 pybind11_DIR
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_core src/python_bindings.cpp)

# 说明：不用 abi3/Py_LIMITED_API —— pybind11 从未真正支持 limited API 编译
# （pybind/pybind11#1755），因此按 Python 版本分别构建 wheel
# （cp38~cp314 各一个，pip 自动选择匹配版本，对用户透明）

target_link_libraries(_core PRIVATE hightorque_robot)
target_compile_features(_core PRIVATE cxx_std_11)

# Python 3.8+ 在 Windows 上不再默认从 PATH 搜索扩展模块的依赖 DLL。
# MinGW 本地构建时静态链接 GCC/C++/winpthread 运行库，使 wheel 不依赖
# 编译器安装目录；MSVC/cibuildwheel 构建不受此分支影响。
if(MINGW)
    target_link_options(_core PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# 把扩展装进 hightorque_robot 包内（默认会装到 site-packages 根，
# 导致 from . import _core 找不到）
install(TARGETS _core
        LIBRARY DESTINATION hightorque_robot
        RUNTIME DESTINATION hightorque_robot)
