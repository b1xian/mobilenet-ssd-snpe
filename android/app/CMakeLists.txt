project(snpedemo)

cmake_minimum_required(VERSION 3.6.0-rc2)

include_directories(${CMAKE_SOURCE_DIR}/src/main/jni/include)

SET(snpe_lib_dir ${CMAKE_SOURCE_DIR}/src/main/jni-libs/${ANDROID_ABI}/libSNPE.so)
ADD_LIBRARY(snpe_lib STATIC IMPORTED)
SET_TARGET_PROPERTIES(snpe_lib PROPERTIES IMPORTED_LOCATION ${snpe_lib_dir})

add_library(snpedemo SHARED src/main/jni/cpp/snpe_engine_jni.cpp)

target_link_libraries(snpedemo android log snpe_lib)
