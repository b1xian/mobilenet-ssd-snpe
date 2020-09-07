#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>

#include <jni.h>

#include <string>
#include <vector>

//snpe
#include "DlContainer/IDlContainer.hpp"
#include "DlSystem/DlEnums.hpp"
#include "DlSystem/DlError.hpp"
#include "DlSystem/ITensorFactory.hpp"
#include "DlSystem/RuntimeList.hpp"
#include "SNPE/SNPEBuilder.hpp"
#include "SNPE/SNPEFactory.hpp"
#include "SNPE/SNPE.hpp"

#define TAG "snpe_engine_jni"

const char *g_jni_class = "com/baidu/snpe/SnpeEngine";

extern "C" {

static std::unique_ptr<zdl::SNPE::SNPE> _engine;


jboolean init(JNIEnv *env, jobject thiz, jint modelSize, jbyteArray modelMem, jint
device, jobjectArray outputTensorNameArray) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "snpe model init");

    zdl::DlSystem::RuntimeList runtimeList;
    zdl::DlSystem::Runtime_t runtime;
    if (device == 1) {
        runtime = zdl::DlSystem::Runtime_t::CPU;
    } else if (device == 2) {
        runtime = zdl::DlSystem::Runtime_t::GPU_FLOAT16;
    } else if (device == 3) {
        runtime = zdl::DlSystem::Runtime_t::GPU_FLOAT32_16_HYBRID;
    } else {
        runtime = zdl::DlSystem::Runtime_t::DSP;
    }
    runtimeList.add(runtime);
    const char *runtime_string = zdl::DlSystem::RuntimeList::runtimeToString(runtime);
    __android_log_print(ANDROID_LOG_INFO, TAG, "use runtime:%s", runtime_string);

    uint8_t *model_mem = (uint8_t *) (env->GetByteArrayElements(modelMem, JNI_FALSE));
    auto container = zdl::DlContainer::IDlContainer::open(model_mem, modelSize);
    if (container == nullptr) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "Error while opening the container file:%s",
                            zdl::DlSystem::getLastErrorString());
        return JNI_FALSE;
    }
    __android_log_print(ANDROID_LOG_INFO, TAG, "open model from memory successd");

    jstring jstr;
    const char *outputTenserName;
    jsize len = env->GetArrayLength(outputTensorNameArray);
    zdl::DlSystem::StringList outputTensors(len);
    __android_log_print(ANDROID_LOG_INFO, TAG, "num of output tensor:%d", len);
    for (int i = 0; i < len; i++) {
        jobject jo = env->GetObjectArrayElement(outputTensorNameArray, i);
        jstr = (jstring) jo;
        outputTenserName = env->GetStringUTFChars(jstr, JNI_FALSE);
        __android_log_print(ANDROID_LOG_INFO, TAG, "add output tensor:%s", outputTenserName);
        outputTensors.append(outputTenserName);
    }

    zdl::SNPE::SNPEBuilder snpeBuilder(container.get());
    _engine = snpeBuilder
            .setOutputTensors(outputTensors)
            .setRuntimeProcessorOrder(runtimeList)
            .build();
    if (_engine == nullptr) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "snpeBuilder error:%s",
                            zdl::DlSystem::getLastErrorString());
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

jobjectArray detect(JNIEnv *env, jobject thiz, jbyteArray imgBytes, jobjectArray
outputTensorNameArray) {
    auto* img_data = (float*) (env->GetByteArrayElements(imgBytes, JNI_FALSE));

    const auto &strList_opt = _engine->getInputTensorNames();
    const auto &strList = *strList_opt;
    const auto &inputDims_opt = _engine->getInputDimensions(strList.at(0));
    const auto &inputShape = *inputDims_opt;
    auto input = zdl::SNPE::SNPEFactory::getTensorFactory().createTensor(inputShape);
    int tensor_size = input.get()->getSize();
    std::copy(img_data, img_data + tensor_size, input->begin());

    static zdl::DlSystem::TensorMap outputTensorMap;
    bool res = _engine->execute(input.get(), outputTensorMap);
    if (!res) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "snpe execute error: %s",
                zdl::DlSystem::getLastErrorString());
    }

    jstring jstr;
    const char *outputTenserName;
    jsize len = env->GetArrayLength(outputTensorNameArray);
    jclass floatArr = env->FindClass("[F");
    jobjectArray jObjectArr = env->NewObjectArray(len, floatArr , NULL);
    jfloatArray jfarr;
    for (int i = 0; i < len; i++) {
        jobject jo = env->GetObjectArrayElement(outputTensorNameArray, i);
        jstr = (jstring) jo;
        outputTenserName = env->GetStringUTFChars(jstr, JNI_FALSE);
        __android_log_print(ANDROID_LOG_INFO, TAG, "collect output tensor:%s", outputTenserName);
        auto tensorPtr = outputTensorMap.getTensor(outputTenserName);
        int tensorSize = tensorPtr->getSize();
        __android_log_print(ANDROID_LOG_INFO, TAG, "tensor size:%d", tensorSize);
        jfarr = env->NewFloatArray(tensorSize);
        int j = 0;
        jfloat* buf = new jfloat[tensorSize];
        for(auto it = tensorPtr->cbegin(); it!=tensorPtr->cend();it++)
        {
            buf[j++] = *it;
        }
        env->SetFloatArrayRegion(jfarr, 0, tensorSize, buf);
        env->SetObjectArrayElement(jObjectArr, i, jfarr);
    }

    return jObjectArr;
}

jboolean set_env(JNIEnv *env, jclass clazz, jstring envName, jstring envPath) {
    const char *env_name = env->GetStringUTFChars(envName, 0);
    const char *env_path = env->GetStringUTFChars(envPath, 0);
    __android_log_print(ANDROID_LOG_INFO, TAG, "set env [%s]: %s", env_name, env_path);
    return setenv(env_name, env_path, 1 /*override*/) == 0;
}

static JNINativeMethod jniNativeMethods[] = {
        {"init", "(I[BI[Ljava/lang/String;)Z", (void *) init},
        {"setEnv", "(Ljava/lang/String;Ljava/lang/String;)Z",  (void *) set_env},
        {"detect", "([B[Ljava/lang/String;)[[F", (void *) detect},
};

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    __android_log_print(ANDROID_LOG_DEBUG, "SnpeEngine", "JNI_OnLoad");

    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return JNI_ERR;
    }
    jclass clazz = env->FindClass(g_jni_class);
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    jint ret = (env)->RegisterNatives(clazz, jniNativeMethods,
                                      sizeof(jniNativeMethods) / sizeof(JNINativeMethod));
    if (ret != JNI_OK) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    __android_log_print(ANDROID_LOG_DEBUG, "SnpeEngine", "JNI_OnUnload");

}

}
