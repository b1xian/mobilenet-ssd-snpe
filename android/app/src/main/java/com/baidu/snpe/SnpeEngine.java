package com.baidu.snpe;

public class SnpeEngine
{
    public native boolean init(int modelSize, byte[] modelMem, int device, String[] outputTensorNameArray);

    public native boolean setEnv(String envName, String nativeDir);

    public native float[][] detect(byte[] imgBytes, String[] outputTensorNameArray);

    static {
        System.loadLibrary("snpedemo");
    }
}
