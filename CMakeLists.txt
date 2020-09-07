cmake_minimum_required( VERSION 2.8 )
project(snpe_cpp_demo)

add_definitions("-Wall -g")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread")

set(TARGET_OS "linux")

# ---------------snpe------------#
if (TARGET_OS MATCHES linux)
    set(snpe_compile_arch x86_64-linux-clang)
elseif(TARGET_OS MATCHES android)
    set(snpe_compile_arch armv7a-android)
endif()


include_directories(
    ${PROJECT_SOURCE_DIR}/include/snpe/zdl
)
link_directories(
    ${PROJECT_SOURCE_DIR}/lib/snpe/${snpe_compile_arch}
)

if (TARGET_OS MATCHES linux)
    set(snpe_libs SNPE)
endif()
# ---------------snpe------------#


add_executable(test-mobilenet-ssd-cpu test_snpe_cpp_demo.cpp util/Util.cpp)
target_link_libraries(test-mobilenet-ssd-cpu ${snpe_libs})


