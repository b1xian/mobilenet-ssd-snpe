package com.baidu.snpe;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;
import android.content.res.AssetManager;

import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

public class MainActivity extends Activity
{
    private static final String TAG = "MainActivity";

    private static final Map<String, Integer> deviceMap = new HashMap() {
        {
            put("CPU", 1);
            put("GPU_FP16", 2);
            put("GPU_MIXED", 3);
            put("DSP", 4);
        }
    };

    private SnpeEngine snpeEngine = new SnpeEngine();

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        String envName = "ADSP_LIBRARY_PATH";
        String nativeDir = this.getApplicationInfo().nativeLibraryDir;
        nativeDir += ";/system/lib/rfsa/adsp;/system/vendor/lib/rfsa/adsp;/dsp";
        boolean ret = snpeEngine.setEnv(envName, nativeDir);
        if (!ret) {
            Log.e(TAG, "set dsp env failed!");
        }
        Log.e(TAG, "set dsp env successed!");


        String modelName = "mobilenet_iter_73000_int8.dlc";
        int device = deviceMap.get("DSP");
        Log.i(TAG , "modelName:" + modelName);
        byte[] modelMem = getAssertFileMem(getAssets(), modelName);
        if (modelMem == null) {
            throw new RuntimeException("model is empty! please check model file is exists!");
        }
        int modelLength = modelMem.length;
        Log.i(TAG , "modelLength:" + modelLength);
        String[] outputTensorNameArray = new String[]{
//                "conv0/scale.conv0",
//                "conv0/relu.conv0",
//                "conv1/dw/scale.conv1/dw",
//                "conv1/dw/relu.conv1/dw",
//                "conv1/scale.conv1",
//                "conv1/relu.conv1",
//                "conv2/dw/scale.conv2/dw",
//                "conv2/dw/relu.conv2/dw",
//                "conv2/scale.conv2",
//                "conv2/relu.conv2",
//                "conv3/dw/scale.conv3/dw",
//                "conv3/dw/relu.conv3/dw",
//                "conv3/scale.conv3",
//                "conv3/relu.conv3",
//                "conv4/dw/scale.conv4/dw",
//                "conv4/dw/relu.conv4/dw",
                "detection_out"
        };
        boolean ret_init = snpeEngine.init(modelLength, modelMem, device, outputTensorNameArray);
        if (!ret_init)
        {
            Log.e(TAG, "snpeEngine Init failed");
            return;
        }
        Log.i(TAG, "snpeEngine Init successed");

        String imgName = "1.raw";
        byte[] imgBytes = getAssertFileMem(getAssets(), imgName);
        float[][] detect_res = snpeEngine.detect(imgBytes, outputTensorNameArray);
//        String write_file = "/storage/emulated/0/vast/detection_out.txt";
//        writeDetectResult(detect_res, write_file);
    }

    private void writeDetectResult (float[][] detect_res, String write_file) {
        String writeResultFileName = "";

        BufferedWriter bw = null;
        try {
            File file = new File(writeResultFileName);
            if (!file.exists()) {
                System.out.println(writeResultFileName);
                file.createNewFile();
            }
            file.setWritable(true);
            bw = new BufferedWriter(new FileWriter(file));
            for (int i = 0; i < detect_res.length; i++) {
                float[] res = detect_res[i];
                for (int j = 0; j < res.length; j++) {
                    System.out.println(res[j]);
                    bw.write(String.valueOf(res[j]));
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (bw != null) {
                    bw.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

    }


    private byte[] getAssertFileMem(AssetManager asm, String modelName) {
        InputStream inputStream = null;
        ByteArrayOutputStream outStream = new ByteArrayOutputStream();
        byte buffer[] = new byte[1024];
        try{
            inputStream = asm.open(modelName);
            int length;
            while ((length = inputStream.read(buffer)) != -1) {
                outStream.write(buffer, 0, length);
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (outStream != null) {
                    outStream.close();
                }
            } catch (IOException e){
                e.printStackTrace();
            }
            try {
                if (inputStream != null) {
                    inputStream.close();
                }
            } catch (IOException e){
                e.printStackTrace();
            }
        }
        return outStream.toByteArray();
    }

    byte[] getNV12(int inputWidth, int inputHeight, Bitmap scaled) {

        int[] argb = new int[inputWidth * inputHeight];

        scaled.getPixels(argb, 0, inputWidth, 0, 0, inputWidth, inputHeight);

        byte[] yuv = new byte[inputWidth * inputHeight * 3 / 2];
        encodeYUV420SP(yuv, argb, inputWidth, inputHeight);

        scaled.recycle();

        return yuv;
    }

    void encodeYUV420SP(byte[] yuv420sp, int[] argb, int width, int height) {
        final int frameSize = width * height;

        int yIndex = 0;
        int uvIndex = frameSize;

        int a = 0;
        int R = 0;
        int G = 0;
        int B = 0;
        int Y = 0;
        int U = 0;
        int V = 0;

        int index = 0;
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {

                a = (argb[index] & 0xff000000) >> 24; // a is not used obviously
                R = (argb[index] & 0xff0000) >> 16;
                G = (argb[index] & 0xff00) >> 8;
                B = (argb[index] & 0xff) >> 0;

                // well known RGB to YUV algorithm
                Y = ((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
                U = ((-38 * R - 74 * G + 112 * B + 128) >> 8) + 128;
                V = ((112 * R - 94 * G - 18 * B + 128) >> 8) + 128;

                // NV21 has a plane of Y and interleaved planes of VU each sampled by a factor of 2
                //    meaning for every 4 Y pixels there are 1 V and 1 U.  Note the sampling is every other
                //    pixel AND every other scanline.
                yuv420sp[yIndex++] = (byte) ((Y < 0) ? 0 : ((Y > 255) ? 255 : Y));
                if (j % 2 == 0 && index % 2 == 0) {
                    yuv420sp[uvIndex++] = (byte) ((U < 0) ? 0 : ((U > 255) ? 255 : U));
                    yuv420sp[uvIndex++] = (byte) ((V < 0) ? 0 : ((V > 255) ? 255 : V));
                }

                index++;
            }
        }
    }
}
