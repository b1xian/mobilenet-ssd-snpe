#include "snpe_engine.h"

#include <iostream>
#include <fstream>

using namespace std;

SnpeEngine::SnpeEngine() {}

int SnpeEngine::init(std::string infer_type, std::string model_path) {

    // 1.set runtime
    int ret = setRuntime(infer_type);
    if(ret != 0) {
        cout << "setRuntime error : " << zdl::DlSystem::getLastErrorString() << endl;
        return -1;
    }

    // 2. load model
    _container = zdl::DlContainer::IDlContainer::open(model_path);
    if (_container == nullptr) {
        cout << "load model error : " << zdl::DlSystem::getLastErrorString() << endl;
        return -1;
    }

    // 3. build engine
    zdl::SNPE::SNPEBuilder snpe_builder(_container.get());
    _engine = snpe_builder
            .setOutputLayers({})
            .setRuntimeProcessorOrder(_runtime_list)
            .build();
    if (_engine == nullptr) {
        cout << "build engine error : " << zdl::DlSystem::getLastErrorString() << endl;
        return -1;
    }

    const auto &strList_opt = _engine->getInputTensorNames();
    if (!strList_opt) throw std::runtime_error("Error obtaining Input tensor names");
    const auto &strList = *strList_opt;
    const auto &inputDims_opt = _engine->getInputDimensions(strList.at(0));
    const auto &inputShape = *inputDims_opt;

    cout << "init success..." << endl;
    return 0;
}

int SnpeEngine::setRuntime(std::string infer_type){
    zdl::DlSystem::Runtime_t runtime_t;

    if (infer_type == "1") {
        runtime_t = zdl::DlSystem::Runtime_t::GPU_FLOAT32_16_HYBRID;
    } else if (infer_type == "2") {
        runtime_t = zdl::DlSystem::Runtime_t::DSP;
    } else {
        runtime_t = zdl::DlSystem::Runtime_t::CPU;
    }

    const char* runtime_string = zdl::DlSystem::RuntimeList::runtimeToString(runtime_t);

    if (!zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime_t) ||
        (infer_type == "1" && !zdl::SNPE::SNPEFactory::isGLCLInteropSupported())) {
        cout << "SNPE runtime " <<  runtime_string << " not support" << endl;
        return -1;
    }

    cout << "SNPE model init, using runtime " <<  runtime_string << endl;

    _runtime_list.add(runtime_t);
    return 0;
}

void SnpeEngine::build_tensor(cv::Mat& mat) {

    zdl::DlSystem::Dimension dims[4];
    dims[0] = 1;
    dims[1] = mat.rows;
    dims[2] = mat.cols;
    dims[3] = mat.channels();
    size_t size = 4; // fp32
    zdl::DlSystem::TensorShape tensorShape(dims, size);

    int mem_size = mat.rows * mat.cols * mat.channels();
    float* src = (float*) mat.data;
    _input_tensor = zdl::SNPE::SNPEFactory::getTensorFactory().createTensor(tensorShape);
    std::copy(src, src + mem_size, _input_tensor->begin());
}

int SnpeEngine::inference(cv::Mat& input_mat, std::pair<int, float*>& pair) {
    build_tensor(input_mat);

    bool ret = _engine->execute(_input_tensor.get(), _output_tensor_map);
    if (!ret) {
        cout << "engine inference error : " << zdl::DlSystem::getLastErrorString() << endl;
        return -1;
    } else {
        cout << "engine inference success..." << endl;
    }

    const zdl::DlSystem::Optional<zdl::DlSystem::StringList> &outputTensorNames = _engine->getOutputTensorNames();
    auto itensor = _output_tensor_map.getTensor((*outputTensorNames).at(0));
    if (itensor == nullptr) {
        cout << "output tensot is null : " << zdl::DlSystem::getLastErrorString() << endl;
        return -1;
    }
    auto itensor_shape = itensor->getShape();
    auto* dims = itensor_shape.getDimensions();
    size_t dim_count = itensor_shape.rank();
    int output_len = 1;
    for (int i = 0; i< dim_count; i++) {
        output_len *=dims[i];
    }
    float* output_data = (float*)malloc(sizeof(float) * output_len);
    int i = 0;
    for(auto it = itensor->begin(); it!=itensor->end();it++)
    {
        output_data[i++] = *it;
    }
    pair.first = output_len;
    pair.second = output_data;
    return 0;
}


