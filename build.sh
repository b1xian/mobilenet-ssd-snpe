#!/usr/bin/env bash

# android build

# NDK
# 通过环境变量或手动设置ANDROID_NDK_HOME路径
#export ANDROID_NDK_HOME=~/Library/Android/sdk/ndk/21.3.6528147

# default setting
TARGET_INDEX="2"
# 0:linux-x86_64
# 1:android-armv7a
# 2:android-armv8a

show_help() {
    echo "Usage: $0 [option...]" >&2
    echo
    echo "   -t, --target            Set build target, 0-osx, 1-android-armv7a, 2-android-armv8a, 3-android-x86_64, 4-android-x86, 5-android-armv7a and x86, 6-android-armv8a and x86"
    echo "   -h, --help              show help message"
    echo
}

# parse arguments
while [ $# != 0 ]
do
  case "$1" in
    -t)
        TARGET_INDEX=$2
        shift
        ;;
    --target)
        TARGET_INDEX=$2
        shift
        ;;
    -h)
        show_help
        exit 1
        ;;
    --help)
    show_help
        exit 1
        ;;
    *)
        ;;
  esac
  shift
done

case "${TARGET_INDEX}" in
0)
    TARGET="linux-x86_64"
    ;;
1)
    TARGET="android-armeabi-v7a"
    ;;
2)
    TARGET="android-arm64-v8a"
    ;;
*)
    TARGET="android-arm64-v8a"
    ;;
esac

android_target_tag="android"
if [[ ${TARGET} == *$android_target_tag* ]]; then
  if [ "${ANDROID_NDK_HOME}" = "" ]; then
    echo "ERROR: Please set ANDROID_NDK_HOME environment"
    exit
  fi
  ANDROID_TOOLCHAIN=${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake
  ANDROID_TOOLCHAIN=${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake
  echo "===== ANDROID_TOOLCHAIN=${ANDROID_TOOLCHAIN}"
fi

# create build dir if not exists
if [ ! -d build ]; then
    mkdir -p build
fi
cd build


BUILD_DIR=${TARGET}
if [ ! -d ${BUILD_DIR} ]; then
    mkdir -p ${BUILD_DIR}
fi
cd ${BUILD_DIR}


if [ "${TARGET}" = "android-armeabi-v7a" ]; then
echo "===== build target: android-armeabi-v7a"
    cmake  -DTARGET_OS=android \
           -DANDROID_ABI=armeabi-v7a \
           -DANDROID_PLATFORM=android-23 \
           -DCMAKE_TOOLCHAIN_FILE=${ANDROID_TOOLCHAIN} \
           -DTARGET_ARCH=armeabi-v7a \
           ../..

elif [ "${TARGET}" = "android-arm64-v8a" ]; then
echo "===== build target: android-arm64-v8a"
    cmake  -DTARGET_OS=android \
           -DANDROID_ABI=arm64-v8a \
           -DANDROID_PLATFORM=android-23 \
           -DCMAKE_TOOLCHAIN_FILE=${ANDROID_TOOLCHAIN} \
           -DTARGET_ARCH=arm64-v8a \
           ../..

elif [ "${TARGET}" = "linux-x86_64" ]; then
echo "===== build target: linux-x86_64"
   cmake   -DTARGET_OS=linux \
           -DTARGET_ARCH=x86_64 \
           ../..
fi

make -j 4

