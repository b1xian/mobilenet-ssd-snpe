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
    std::cout << "into main." << std::endl;
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

    // 构建引擎
    SnpeEngine* engine = new SnpeEngine();
    engine->init(infer_type, model_name);

    // 读取图片
    cv::Mat mat = cv::imread(img_path);
    int width = mat.cols;
    int height = mat.rows;
    int c = mat.channels();

    cv::Mat input_mat = normalize(mat);

    std::pair<int, float*> pair;
    int ret = engine->inference(input_mat, pair);
    for (int i = 0; i< pair.first; i++) {
        cout << pair.second[i] << endl;
    }
    int output_len = pair.first;
    float* output_data = pair.second;


    std::vector<std::string> class_names =
            {"background", "aeroplane", "bicycle", "bird", "boat",
            "bottle", "bus", "car", "cat", "chair",
            "cow", "diningtable", "dog", "horse",
            "motorbike", "person", "pottedplant",
            "sheep", "sofa", "train", "tvmonitor"};

    for (int i = 0; i < output_len; i += 7) {
        std::string label = class_names[int(output_data[i+1])];
        float prob = output_data[i + 2];
        int x = int(output_data[i + 3] * width);
        int y = int(output_data[i + 4] * height);
        int w = int(output_data[i + 5] * width - x);
        int h = int(output_data[i + 6] * height - y);
        cout << label << endl;
        cout << prob << endl;
        cout << x << endl;
        cout << y << endl;
        cout << w << endl;
        cout << h << endl;

        cv::putText(mat, label, cv::Point(x, y), 0, 0.8, cv::Scalar(255,255,255), 2);
        cv::rectangle(mat, cv::Point(x, y), cv::Point(x+w, y+h), cv::Scalar(0,255,0), 2);
    }
    cv::imwrite("./result.jpg", mat);

}