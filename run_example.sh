#!/usr/bin/env bash

SNPE_LIB=build/armv7a-release/test_demo

adb push $SNPE_LIB /data/local/tmp/
bin_path="/data/local/tmp/test_demo/"
adb shell "chmod +x ${bin_path}/test-mobilenet-ssd"
adb shell "cd ${bin_path} \
       && export LD_LIBRARY_PATH=${bin_path}:${LD_LIBRARY_PATH} \
       && ./test-mobilenet-ssd mobilenet_iter_73000_int8.dlc 1.raw"


