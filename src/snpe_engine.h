#ifndef VISION_SNPE_PREDICTOR_H
#define VISION_SNPE_PREDICTOR_H

#include "DlContainer/IDlContainer.hpp"
#include "DlSystem/DlEnums.hpp"
#include "DlSystem/DlError.hpp"
#include "DlSystem/ITensorFactory.hpp"
#include "DlSystem/RuntimeList.hpp"
#include "SNPE/SNPEBuilder.hpp"
#include "SNPE/SNPEFactory.hpp"
#include "SNPE/SNPE.hpp"

#include "opencv2/opencv.hpp"

class SnpeEngine {
public:
    SnpeEngine();
    ~SnpeEngine();

    int init(zdl::DlSystem::Runtime_t runtime_t, const std::string &model_path);
    int inference(const cv::Mat& input_mat, cv::Mat& output_mat);

private:
    int setRuntime(zdl::DlSystem::Runtime_t runtime_t);
    void build_tensor(const cv::Mat& mat);

    // snpe model
    std::unique_ptr<zdl::SNPE::SNPE> _engine;
    std::unique_ptr<zdl::DlContainer::IDlContainer> _container;

    // snpe input & output
    zdl::DlSystem::StringList _output_tensor_names;
    // if use ITensor
    std::unique_ptr<zdl::DlSystem::ITensor> _input_tensor;
    zdl::DlSystem::TensorMap _output_tensor_map;

    // snpe builder config
    // _runtime_list : runtime order list
    zdl::DlSystem::RuntimeList _runtime_list;
};

#endif //VISION_SNPE_PREDICTOR_H
