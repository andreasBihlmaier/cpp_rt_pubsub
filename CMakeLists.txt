cmake_minimum_required(VERSION 3.16)
project(cpp_rt_pubsub)


#find_package(Boost COMPONENTS program_options REQUIRED)

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
  -checks=*,-modernize-use-trailing-return-type;
  -warnings-as-errors=*;)

set(CMAKE_CXX_CPPCHECK
  cppcheck;
  --std=c++${CMAKE_CXX_STANDARD})

include_directories(
  include
)

#link_directories(
#)

add_library(cpp_rt_pubsub_broker SHARED
  src/broker.cpp
)
#target_link_libraries(cpp_rt_pubsub_broker
#)

add_library(cpp_rt_pubsub_publisher SHARED
  src/publisher.cpp
)
#target_link_libraries(cpp_rt_pubsub_publisher
#)

add_library(cpp_rt_pubsub_subscriber SHARED
  src/subscriber.cpp
)
#target_link_libraries(cpp_rt_pubsub_subscriber
#)


add_executable(broker
  src/broker_main.cpp
)
target_link_libraries(broker
  cpp_rt_pubsub_broker
#  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

add_executable(demo_pub
  src/demo_pub_main.cpp
)
target_link_libraries(demo_pub
  cpp_rt_pubsub_publisher
#  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

add_executable(demo_sub
  src/demo_sub_main.cpp
)
target_link_libraries(demo_sub
  cpp_rt_pubsub_subscriber
#  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)


#TODO add_subdirectory(tests)