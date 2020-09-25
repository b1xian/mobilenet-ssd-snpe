#!/usr/bin/env bash

# android build

# create build dir if not exists
if [ ! -d build ]; then
    mkdir -p build
fi
cd build

BUILD_DIR="armv7a-release"
if [ ! -d $BUILD_DIR ]; then
    mkdir -p $BUILD_DIR
fi
cd $BUILD_DIR

if [ "$ANDROID_NDK_HOME" = "" ]; then
    echo "ERROR: Please set ANDROID_NDK_HOME environment"
    exit
fi

echo "===== ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
ANDROID_TOOLCHAIN=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake

echo "===== cmake target: android-armeabi-v7a"
cmake  -DTARGET_OS=android \
       -DANDROID_ABI=armeabi-v7a \
       -DANDROID_PLATFORM=android-23 \
       -DCMAKE_TOOLCHAIN_FILE=$ANDROID_TOOLCHAIN \
       ../..

make -j 4

cd ../../
DEMO_DIR=build/$BUILD_DIR/test_demo
if [ ! -d $DEMO_DIR ]; then
    mkdir -p $DEMO_DIR
fi
cp build/$BUILD_DIR/test-mobilenet-ssd $DEMO_DIR
cp model/mobilenet_iter_73000_int8.dlc $DEMO_DIR
cp model/mobilenet_iter_73000.dlc $DEMO_DIR
cp data/VOC_raw/1.raw $DEMO_DIR
cp lib/snpe/armv7a-android/* $DEMO_DIR
