cmake_minimum_required (VERSION 3.0)
project (scout.cpp)


# adjust these if wabt or yaml-cpp are not subdirectories of this main repo directory
set(WABT_DIR .)
set(YAMLCPP_DIR .)


option(BUILD_TESTS OFF)
add_subdirectory(${WABT_DIR}/wabt)
add_subdirectory(${YAMLCPP_DIR}/yaml-cpp)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -g -Wall")

add_library(intx STATIC
  intx/lib/intx/div.cpp
  )


add_executable(scout.exec scout.cpp bn_api/bn_api.cpp bn_api/bn_api.h)
add_dependencies(scout.exec wabt yaml-cpp)
target_link_libraries(scout.exec wabt yaml-cpp intx)
include_directories(${CMAKE_BINARY_DIR}/wabt ${WABT_DIR}/wabt ${WABT_DIR}/wabt/src ${YAMLCPP_DIR}/yaml-cpp/include ${YAMLCPP_DIR}/yaml-cpp/src ./intx/include ./bn_api)
