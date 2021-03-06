cmake_minimum_required(VERSION 2.8)

enable_testing()

project(bmcpp)

# Compiler configs
set(CMAKE_C_FLAGS
  "-Wall -Wextra -std=gnu99 -fdata-sections -ffunction-sections " CACHE INTERNAL "c compiler flags" FORCE)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11 -fdata-sections -ffunction-sections -fno-rtti -fno-exceptions" CACHE INTERNAL "c++ compiler flags" FORCE)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections " CACHE INTERNAL "c++ linker flags" FORCE)
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_C_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")

add_library(bmcpp INTERFACE)
include_directories(include)

# Testing
function(bmcpp_test name)
  add_executable(${name} test/${name}.cpp)
  add_test(
    NAME    ${name}_valgrind
    COMMAND valgrind --leak-check=full
                     --error-exitcode=1 $<TARGET_FILE:${name}>)
  add_test(${name} ${name})
endfunction(bmcpp_test)

bmcpp_test(list)
bmcpp_test(compile-test)
