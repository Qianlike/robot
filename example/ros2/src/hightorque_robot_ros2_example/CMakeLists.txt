cmake_minimum_required(VERSION 3.8)
project(hightorque_robot_ros2_example)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosidl_default_generators REQUIRED)

get_filename_component(HIGHTORQUE_ROBOT_PACKAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" REALPATH)
if(NOT HIGHTORQUE_ROBOT_SOURCE_DIR)
  foreach(relative_path "../../.." "../../../..")
    get_filename_component(candidate_dir
      "${HIGHTORQUE_ROBOT_PACKAGE_DIR}/${relative_path}" REALPATH)
    if(EXISTS "${candidate_dir}/CMakeLists.txt" AND
       EXISTS "${candidate_dir}/include/canport.h")
      set(HIGHTORQUE_ROBOT_SOURCE_DIR "${candidate_dir}")
      break()
    endif()
  endforeach()
endif()
set(HIGHTORQUE_ROBOT_SOURCE_DIR "${HIGHTORQUE_ROBOT_SOURCE_DIR}" CACHE PATH
    "Path to the hightorque_robot source tree")

if(NOT EXISTS "${HIGHTORQUE_ROBOT_SOURCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "HIGHTORQUE_ROBOT_SOURCE_DIR does not contain CMakeLists.txt: ${HIGHTORQUE_ROBOT_SOURCE_DIR}")
endif()

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/MotorState.msg"
)

set(HIGHTORQUE_BUILD_DEMO OFF CACHE BOOL "Build hightorque_robot demo executables" FORCE)
add_subdirectory("${HIGHTORQUE_ROBOT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/hightorque_robot")

if(COMMAND rosidl_get_typesupport_target)
  rosidl_get_typesupport_target(HIGHTORQUE_ROS2_TYPESUPPORT_TARGET
    ${PROJECT_NAME} "rosidl_typesupport_cpp")
endif()

function(add_hightorque_ros2_example target source)
  add_executable(${target} ${source})
  target_compile_definitions(${target} PRIVATE
    HIGHTORQUE_ROBOT_SOURCE_DIR=\"${HIGHTORQUE_ROBOT_SOURCE_DIR}\"
  )
  ament_target_dependencies(${target} ament_index_cpp rclcpp)
  if(COMMAND rosidl_get_typesupport_target)
    target_link_libraries(${target} "${HIGHTORQUE_ROS2_TYPESUPPORT_TARGET}")
  else()
    rosidl_target_interfaces(${target} ${PROJECT_NAME} "rosidl_typesupport_cpp")
  endif()
  target_link_libraries(${target} hightorque_robot)
endfunction()

add_hightorque_ros2_example(canport_feedback src/canport_feedback.cpp)
add_hightorque_ros2_example(canport_move_zero src/canport_move_zero.cpp)
add_hightorque_ros2_example(canport_run src/canport_run.cpp)
add_hightorque_ros2_example(canport_set_zero src/canport_set_zero.cpp)
add_hightorque_ros2_example(motors_feedback src/motors_feedback.cpp)
add_hightorque_ros2_example(motors_move_zero src/motors_move_zero.cpp)
add_hightorque_ros2_example(motors_run src/motors_run.cpp)
add_hightorque_ros2_example(motors_set_zero src/motors_set_zero.cpp)

install(TARGETS
  canport_feedback
  canport_move_zero
  canport_run
  canport_set_zero
  motors_feedback
  motors_move_zero
  motors_run
  motors_set_zero
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY "${HIGHTORQUE_ROBOT_SOURCE_DIR}/robot_param/"
  DESTINATION share/${PROJECT_NAME}/robot_param
)

ament_export_dependencies(ament_index_cpp rclcpp rosidl_default_runtime)
ament_package()
