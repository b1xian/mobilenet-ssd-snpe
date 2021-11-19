//
// Created by v_guojinlong on 2020-10-20.
//

#include "opencv2/opencv.hpp"

#include "snpe_engine.h"

using namespace std;

cv::Mat normalize(cv::Mat& mat_src) {
    cv::Mat mat_src_float;
    mat_src.convertTo(mat_src_float, CV_32FC3);

    cv::Mat mat_mean;
    cv::Mat mat_stddev;
    cv::meanStdDev(mat_src_float, mat_mean, mat_stddev);
    cv::Mat mat_dst;

    if (mat_src.channels() == 1) {
        auto m = *((double*)mat_mean.data);
        auto s = *((double*)mat_stddev.data);
        mat_dst = (mat_src_float - m) / (s + 1e-6);
    } else {
        std::vector<cv::Mat> mats;
        cv::split(mat_src_float, mats);
        int c = 0;
        for (auto& mat : mats) {
            auto m = ((double *)mat_mean.data)[c];
            auto s = ((double *)mat_stddev.data)[c];
            mat = (mat - m) / (s + 1e-6);
            c++;
        }
        cv::merge(mats, mat_dst);
    }
    return mat_dst;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "parameters required: <infer_type> <model_name> <img_path>" << std::endl;
        return -1;
    }

    string infer_type = argv[1];
    cout << "infer_type:" << infer_type << endl;
    string model_name = argv[2];
    cout << "model_name:" << model_name << endl;
    string img_path = argv[3];
    cout << "img_path path:" << img_path << endl;

    int runtime = stoi(infer_type);
    zdl::DlSystem::Runtime_t runtime_t;
    switch (runtime) {
        case 0:
            runtime_t = zdl::DlSystem::Runtime_t::CPU_FLOAT32;
        case 1:
            runtime_t = zdl::DlSystem::Runtime_t::GPU_FLOAT32_16_HYBRID;
        case 2:
            runtime_t = zdl::DlSystem::Runtime_t::DSP_FIXED8_TF;
        case 3:
            runtime_t = zdl::DlSystem::Runtime_t::GPU_FLOAT16;
        case 5:
            runtime_t = zdl::DlSystem::Runtime_t::AIP_FIXED8_TF;
        default:
            runtime_t = zdl::DlSystem::Runtime_t::CPU_FLOAT32;
    }

    // 构建引擎
    SnpeEngine* engine = new SnpeEngine();
    engine->init(runtime_t, model_name);

    // 读取图片
    cv::Mat mat = cv::imread(img_path);
    int width = mat.cols;
    int height = mat.rows;

    cv::Mat input_mat = normalize(mat);
    cv::Mat output_mat;
    std::pair<int, float*> pair;
    int ret = engine->inference(input_mat, output_mat);
    if (ret) {
        delete engine;
        cerr << "inference failed!!!" << endl;
        return ret;
    }

    int output_len = output_mat.channels() * output_mat.rows * output_mat.cols;
    float* output_data = (float *)output_mat.data;

    std::vector<std::string> class_names =
            {"background", "aeroplane", "bicycle", "bird", "boat",
            "bottle", "bus", "car", "cat", "chair",
            "cow", "diningtable", "dog", "horse",
            "motorbike", "person", "pottedplant",
            "sheep", "sofa", "train", "tvmonitor"};

    for (int i = 0; i < output_len; i += 7) {
        float prob = output_data[i + 2];
        int x = int(output_data[i + 3] * width);
        int y = int(output_data[i + 4] * height);
        int w = int(output_data[i + 5] * width - x);
        int h = int(output_data[i + 6] * height - y);
        std::string label(class_names[int(output_data[i+1])] + std::to_string(prob));
        cv::putText(mat, label, cv::Point(x, y - 5), 0, 0.5, cv::Scalar(255,0,255), 2);
        cv::rectangle(mat, cv::Point(x, y), cv::Point(x + w, y + h), cv::Scalar(0,255,0), 2);
    }
    cv::imwrite("./result.jpg", mat);

    delete engine;
}