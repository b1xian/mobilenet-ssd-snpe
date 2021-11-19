#include "snpe_engine.h"

#include <iostream>

using namespace std;

SnpeEngine::SnpeEngine() {}

int SnpeEngine::init(zdl::DlSystem::Runtime_t runtime_t, const std::string &model_path) {
    // 1.set runtime
    int ret = setRuntime(runtime_t);
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

    cout << "init success..." << endl;
    return 0;
}


SnpeEngine::~SnpeEngine() {
    _engine.reset();
    cout << "deinit success..." << endl;
}

int SnpeEngine::setRuntime(zdl::DlSystem::Runtime_t runtime_t){
    const char* runtime_string = zdl::DlSystem::RuntimeList::runtimeToString(runtime_t);

    if (!zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime_t) ||
        ((runtime_t == zdl::DlSystem::Runtime_t::GPU_FLOAT16 || runtime_t == zdl::DlSystem::Runtime_t::GPU_FLOAT32_16_HYBRID)
        && !zdl::SNPE::SNPEFactory::isGLCLInteropSupported())) {
        cout << "SNPE runtime " <<  runtime_string << " not support" << endl;
        return -1;
    }

    cout << "SNPE model init, using runtime " <<  runtime_string << endl;

    _runtime_list.add(runtime_t);
    return 0;
}

void SnpeEngine::build_tensor(const cv::Mat& mat) {

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

int SnpeEngine::inference(const cv::Mat& input_mat, cv::Mat& output_mat) {
    build_tensor(input_mat);

    bool ret = _engine->execute(_input_tensor.get(), _output_tensor_map);
    if (!ret) {
        cerr << "engine inference error : " << zdl::DlSystem::getLastErrorString() << endl;
        return -1;
    } else {
        cout << "engine inference success..." << endl;
    }

    const zdl::DlSystem::Optional<zdl::DlSystem::StringList> &outputTensorNames = _engine->getOutputTensorNames();
    auto itensor = _output_tensor_map.getTensor((*outputTensorNames).at(0));
    if (itensor == nullptr) {
        cerr << "output tensor is null : " << zdl::DlSystem::getLastErrorString() << endl;
        return -1;
    }
    auto itensor_shape = itensor->getShape();
    auto* dims = itensor_shape.getDimensions();
    size_t dim_count = itensor_shape.rank();
    int output_len = 1;
    for (unsigned int i = 0; i < dim_count; i++) {
        output_len *= dims[i];
    }
    output_mat = cv::Mat({output_len, 1}, CV_32FC1);
    float* output_data = (float *)output_mat.data;
    int i = 0;
    for(auto it = itensor->begin(); it!=itensor->end();it++) {
        output_data[i++] = *it;
    }
    return 0;
}


