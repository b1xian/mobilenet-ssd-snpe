#!/usr/bin/env bash

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

DEMO_DIR=build/${TARGET}/test_demo
rm -r ${DEMO_DIR}
mkdir ${DEMO_DIR}

cp build/${TARGET}/test-mobilenet-ssd ${DEMO_DIR}
cp model/mobilenet_iter_73000_int8.dlc ${DEMO_DIR}
cp model/mobilenet_iter_73000.dlc ${DEMO_DIR}
cp data/VOC_raw/1.raw ${DEMO_DIR}
cp data/VOC_resize/9.jpg ${DEMO_DIR}
cp third_party/snpe/lib/${TARGET}/* ${DEMO_DIR}

bin_path="/data/local/tmp/test_demo/"
adb shell "rm -r ${bin_path}"
adb push ${DEMO_DIR} /data/local/tmp/
adb shell "chmod +x ${bin_path}/test-mobilenet-ssd"
#adb shell "cd ${bin_path} \
#       && export LD_LIBRARY_PATH=${bin_path}:${LD_LIBRARY_PATH} \
#       && ./test-mobilenet-ssd mobilenet_iter_73000_int8.dlc 1.raw"
adb shell "cd ${bin_path} \
       && export LD_LIBRARY_PATH=${bin_path}:${LD_LIBRARY_PATH} \
       && ./test-mobilenet-ssd 0 mobilenet_iter_73000.dlc 9.jpg"

adb pull $bin_path"result.jpg" ./
