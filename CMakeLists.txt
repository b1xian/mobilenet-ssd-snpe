cmake_minimum_required( VERSION 2.8 )
project(test_snpe_demo)
set(demo_name test-mobilenet-ssd)

add_definitions("-Wall -g")
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread")


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
if (${TARGET_OS} STREQUAL "android")
    SET(snpe_libs c++ SNPE snpe_adsp snpe_dsp_domains_system snpe_dsp_domains snpe_dsp_domains_v2_system
            snpe_dsp_domains_v2  calculator calculator_domains PlatformValidatorShared)
elseif(${TARGET_OS} STREQUAL "linux")
    SET(snpe_libs SNPE)
endif()
# ---------------snpe------------#

add_executable(${demo_name} test_snpe_cpp_demo.cpp util/Util.cpp)
#add_executable(${demo_name} test_snpe_cpp_demo.cpp)
target_compile_options(${demo_name} PRIVATE -std=c++11 -O3 -frtti -fexceptions)
target_link_libraries(${demo_name} PUBLIC ${snpe_libs})
if (${TARGET_OS} MATCHES "android")
    target_link_libraries(${demo_name} PUBLIC log z)
endif()
