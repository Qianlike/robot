cmake_minimum_required(VERSION 3.5...3.30)
project(hightorque_robot_ros1_example)

find_package(catkin REQUIRED COMPONENTS
  message_generation
  roscpp
  roslib
)

add_message_files(
  FILES
    MotorState.msg
)

generate_messages()

catkin_package(
  CATKIN_DEPENDS
    message_runtime
    roscpp
    roslib
)

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

set(HIGHTORQUE_BUILD_DEMO OFF CACHE BOOL "Build hightorque_robot demo executables" FORCE)
add_subdirectory("${HIGHTORQUE_ROBOT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/hightorque_robot")

include_directories(${catkin_INCLUDE_DIRS})

function(add_hightorque_ros1_example target source)
  add_executable(${target} ${source})
  target_compile_definitions(${target} PRIVATE
    HIGHTORQUE_ROBOT_SOURCE_DIR=\"${HIGHTORQUE_ROBOT_SOURCE_DIR}\"
  )
  add_dependencies(${target}
    ${${PROJECT_NAME}_EXPORTED_TARGETS}
    ${catkin_EXPORTED_TARGETS}
  )
  target_link_libraries(${target}
    hightorque_robot
    ${catkin_LIBRARIES}
  )
endfunction()

add_hightorque_ros1_example(canport_feedback src/canport_feedback.cpp)
add_hightorque_ros1_example(canport_move_zero src/canport_move_zero.cpp)
add_hightorque_ros1_example(canport_run src/canport_run.cpp)
add_hightorque_ros1_example(canport_set_zero src/canport_set_zero.cpp)
add_hightorque_ros1_example(motors_feedback src/motors_feedback.cpp)
add_hightorque_ros1_example(motors_move_zero src/motors_move_zero.cpp)
add_hightorque_ros1_example(motors_run src/motors_run.cpp)
add_hightorque_ros1_example(motors_set_zero src/motors_set_zero.cpp)

install(TARGETS
  canport_feedback
  canport_move_zero
  canport_run
  canport_set_zero
  motors_feedback
  motors_move_zero
  motors_run
  motors_set_zero
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY "${HIGHTORQUE_ROBOT_SOURCE_DIR}/robot_param/"
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/robot_param
)
