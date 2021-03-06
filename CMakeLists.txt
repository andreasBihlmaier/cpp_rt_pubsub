cmake_minimum_required(VERSION 3.16)
project(cpp_rt_pubsub)

option(CRPS_LOGGER_NO_DEBUG "Remove crps::Logger::LogLevel::Debug code (must be wrapped in CRPS_LOGGER_DEBUG macro)" OFF)

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif (NOT CMAKE_BUILD_TYPE)

if (CRPS_LOGGER_NO_DEBUG)
 add_definitions(-DCRPS_LOGGER_NO_DEBUG)
endif()


set(CMAKE_CXX_CLANG_TIDY
  clang-tidy;
  -header-filter=include/crps;
  -checks=*,-modernize-use-trailing-return-type,-fuchsia-default-arguments-calls,-fuchsia-default-arguments-declarations,-fuchsia-overloaded-operator;
  -warnings-as-errors=*;)

set(CMAKE_CXX_CPPCHECK
  cppcheck;
  --std=c++${CMAKE_CXX_STANDARD}
  --suppress=*:*/third_party/*)

include_directories(
  include
  third_party/nlohmann_json/include
)

#link_directories(
#)

add_library(crps_common SHARED
  src/broker_protocol.cpp
  src/logger.cpp
  src/message_queue.cpp
  src/os.cpp
)
#target_link_libraries(crps_common
#)

add_library(crps_linux SHARED
  src/linux_network.cpp
  src/linux_os.cpp
)
target_link_libraries(crps_linux
  crps_common
)

add_library(crps_broker SHARED
  src/broker.cpp
)
target_link_libraries(crps_broker
  crps_common
  crps_linux
)

add_library(crps_node SHARED
  src/node.cpp
  src/publisher.cpp
  src/subscriber.cpp
)
target_link_libraries(crps_node
  crps_common
  crps_linux
)

add_library(crps_demo SHARED
  src/demo/parse_options.cpp
)

add_executable(broker
  src/broker_main.cpp
)
target_link_libraries(broker
  crps_broker
  crps_demo
)

add_executable(demo_pub
  src/demo_pub_main.cpp
)
target_link_libraries(demo_pub
  crps_node
  crps_demo
)

add_executable(demo_sub
  src/demo_sub_main.cpp
)
target_link_libraries(demo_sub
  crps_node
  crps_demo
)

add_executable(benchmark
  src/benchmark_main.cpp
)
target_link_libraries(benchmark
  crps_node
  crps_demo
)


#TODO add_subdirectory(tests)