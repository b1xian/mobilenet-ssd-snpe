cmake_minimum_required( VERSION 2.8 )
project(test_snpe_demo)

set(demo_name test-mobilenet-ssd)

add_definitions("-Wall -g")
set(CMAKE_BUILD_TYPE Release)

set(THIRD_PARTY_DIR third_party)

# ---------------opencv------------#
set(OPENCV_VER 2.4.13.4)
set(OPENCV_DIR ${THIRD_PARTY_DIR}/opencv_${OPENCV_VER})
set(OPENCV_INCLUDE_DIR ${OPENCV_DIR}/${TARGET_OS}-${TARGET_ARCH}/include/)
set(OPENCV_LINK_DIR ${OPENCV_DIR}/${TARGET_OS}-${TARGET_ARCH}/lib/)
set(OPENCV_3RDPARTY_LINK_DIR ${OPENCV_DIR}/${TARGET_OS}-${TARGET_ARCH}/3rdparty/)
message(STATUS "[Dependency] opencv lib_dir=" ${OPENCV_LINK_DIR})
set(OPENCV_LIB opencv_calib3d opencv_highgui opencv_imgproc opencv_core libtiff libpng libjpeg libjasper IlmImf)
# ---------------opencv------------#


# ---------------snpe------------#
set(SNPE_VER 1.43)
set(SNPE_DIR ${THIRD_PARTY_DIR}/snpe/${SNPE_VER})
set(SNPE_INCLUDE_DIR ${SNPE_DIR}/include/zdl)
message(${SNPE_INCLUDE_DIR})
set(SNPE_LINK_DIR ${SNPE_DIR}/lib/${TARGET_OS}-${TARGET_ARCH})
message(STATUS "[Dependency] snpe lib_dir=" ${SNPE_LINK_DIR})
if (${TARGET_OS} STREQUAL "android")
    SET(SNPE_LIB c++_shared SNPE PlatformValidatorShared)
elseif(${TARGET_OS} STREQUAL "linux")
    SET(SNPE_LIB SNPE)
endif()
# ---------------snpe------------#



# ---------------build target------------#
#add_executable(${demo_name} test_snpe_cpp_demo.cpp util/Util.cpp)
aux_source_directory(src snpe_srcs)
add_executable(${demo_name} ${snpe_srcs})

target_compile_options(${demo_name} PRIVATE -std=c++11 -O3 -frtti -fexceptions)
target_link_directories(${demo_name} PUBLIC ${SNPE_LINK_DIR} ${OPENCV_LINK_DIR} ${OPENCV_3RDPARTY_LINK_DIR})
target_include_directories(${demo_name} PUBLIC ${SNPE_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR})
target_link_libraries(${demo_name} PUBLIC ${SNPE_LIB} ${OPENCV_LIB})
if (${TARGET_OS} MATCHES "android")
    target_link_libraries(${demo_name} PUBLIC log z)
endif()
