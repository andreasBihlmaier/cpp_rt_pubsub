cmake_minimum_required(VERSION 3.16)
project(cpp_rt_pubsub)

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif (NOT CMAKE_BUILD_TYPE)


set(CMAKE_CXX_CLANG_TIDY
  clang-tidy;
  -header-filter=include;
  -checks=*,-modernize-use-trailing-return-type,-fuchsia-default-arguments-calls,-fuchsia-default-arguments-declarations;
  -warnings-as-errors=*;)

set(CMAKE_CXX_CPPCHECK
  cppcheck;
  --std=c++${CMAKE_CXX_STANDARD})

include_directories(
  include
)

#link_directories(
#)

add_library(crps_common SHARED
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


add_executable(broker
  src/broker_main.cpp
)
target_link_libraries(broker
  crps_broker
)

add_executable(demo_pub
  src/demo_pub_main.cpp
)
target_link_libraries(demo_pub
  crps_node
)

add_executable(demo_sub
  src/demo_sub_main.cpp
)
target_link_libraries(demo_sub
  crps_node
)


#TODO add_subdirectory(tests)