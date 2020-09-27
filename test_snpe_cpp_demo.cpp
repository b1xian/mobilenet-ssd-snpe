#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <unordered_map>
#include <algorithm>
#include <assert.h>

#include "DlSystem/DlError.hpp"
#include "DlSystem/RuntimeList.hpp"
#include "DlSystem/UserBufferMap.hpp"
#include "DlSystem/UDLFunc.hpp"
#include "DlSystem/IUserBuffer.hpp"
#include "DlContainer/IDlContainer.hpp"
#include "SNPE/SNPE.hpp"

#include "SNPE/SNPEFactory.hpp"
#include "DlSystem/DlVersion.hpp"
#include "DlSystem/DlEnums.hpp"
#include "DlSystem/String.hpp"

#include "SNPE/SNPEBuilder.hpp"

#include "DlSystem/IUDL.hpp"
#include "DlSystem/UDLContext.hpp"

#include "util/Util.hpp"

#include <chrono>
#include <stack>
using namespace std;
using namespace std::chrono;

class Timer{
  public:
    std::stack<high_resolution_clock::time_point> tictoc_stack;
  void tic(){
    high_resolution_clock::time_point t1=high_resolution_clock::now();
    tictoc_stack.push(t1);
  }
  double toc(std::string msg="",bool flag=true ){
    double diff = duration_cast<milliseconds> (high_resolution_clock::now() - tictoc_stack.top()).count();
    if(msg.size()>0){
      if(flag)
      printf("%s Time Processing:%f ms\n",msg.c_str(),diff);
    }
    tictoc_stack.pop();
    return diff;
  }
  void reset(){
    tictoc_stack = std::stack<high_resolution_clock::time_point>();
  }
};



// get runtime
zdl::DlSystem::Runtime_t checkRuntime()
{
    static zdl::DlSystem::Version_t Version = zdl::SNPE::SNPEFactory::getLibraryVersion();
    static zdl::DlSystem::Runtime_t Runtime;
    std::cout << "SNPE Version: " << Version.asString().c_str() << std::endl; //Print Version number

    Runtime = zdl::DlSystem::Runtime_t::DSP;
    return Runtime;
}


// get container
std::unique_ptr<zdl::DlContainer::IDlContainer> loadContainerFromFile(std::string containerPath)
{
    std::unique_ptr<zdl::DlContainer::IDlContainer> container;
    container = zdl::DlContainer::IDlContainer::open(containerPath);
    return container;
}


// setNetwork Builder Options
std::unique_ptr<zdl::SNPE::SNPE> setBuilderOptions(std::unique_ptr<zdl::DlContainer::IDlContainer>& container,
                                                   zdl::DlSystem::RuntimeList runtimeList)
{
    zdl::DlSystem::StringList outputTensorNames(1);
    outputTensorNames.append("");

    std::unique_ptr<zdl::SNPE::SNPE> snpe;
    zdl::SNPE::SNPEBuilder snpeBuilder(container.get());
    snpe = snpeBuilder.setOutputLayers({})
       //.setOutputTensors(outputTensorNames)
       .setRuntimeProcessorOrder(runtimeList)
       .build();
    return snpe;
}


// getITensors
std::unique_ptr<zdl::DlSystem::ITensor> loadInputTensor(std::unique_ptr<zdl::SNPE::SNPE>& snpe , std::string& fileLine)
{
    std::unique_ptr<zdl::DlSystem::ITensor> input;
    const auto &strList_opt = snpe->getInputTensorNames();
    if (!strList_opt) throw std::runtime_error("Error obtaining Input tensor names");
    const auto &strList = *strList_opt;
    assert (strList.size() == 1);
    std::string filePath(fileLine);
    std::vector<float> inputVec = loadFloatDataFile(filePath);
    const auto &inputDims_opt = snpe->getInputDimensions(strList.at(0));
    const auto &inputShape = *inputDims_opt;
    input = zdl::SNPE::SNPEFactory::getTensorFactory().createTensor(inputShape);
    std::copy(inputVec.begin(), inputVec.end(), input->begin());
    return input;
}


int main(int argc, char** argv)
{

    std::cout << "into main." << std::endl;
    if (argc < 3) {
        std::cout << "parameters required: <model_name> <raw_img_path>" << std::endl;
        return -1;
    }

    string model_name = argv[1];
    cout << "model_name:" << model_name << endl;
    string raw_img_path = argv[2];
    cout << "raw_img_path path:" << raw_img_path << endl;

    static zdl::DlSystem::RuntimeList runtimeList;
    static zdl::DlSystem::Runtime_t runtime = checkRuntime();
    runtimeList.add(runtime);

    bool useUserSuppliedBuffers = false;
    std::unique_ptr<zdl::SNPE::SNPE> snpe;
    zdl::DlSystem::PlatformConfig platformConfig;
    bool usingInitCaching = false;

    std::unique_ptr<zdl::DlContainer::IDlContainer> container = loadContainerFromFile(model_name);
    if (container == nullptr)
    {
       std::cerr << "Error while opening the container file." << std::endl;
       return EXIT_FAILURE;
    }

    snpe = setBuilderOptions(container, runtimeList);
    if (container == nullptr)
    {
       return EXIT_FAILURE;
    }

    const zdl::DlSystem::Optional<zdl::DlSystem::StringList> &outputTensorNames = snpe->getOutputTensorNames();
    cout << "output tensor name:" <<  (*outputTensorNames).at(0) << endl;



    //读取图片
    std::unique_ptr<zdl::DlSystem::ITensor> input;
    const auto &strList_opt = snpe->getInputTensorNames();
    const auto &strList = *strList_opt;
    assert (strList.size() == 1);
    std::string filePath(raw_img_path);
    std::vector<float> inputVec = loadFloatDataFile(filePath);
    cout<<inputVec.size()<<endl;
    const auto &inputDims_opt = snpe->getInputDimensions(strList.at(0));
    const auto &inputShape = *inputDims_opt;
    input = zdl::SNPE::SNPEFactory::getTensorFactory().createTensor(inputShape);
    std::copy(inputVec.begin(), inputVec.end(), input->begin());

    cout << "load file to tensor finish"<<endl;

    //执行inference
    static zdl::DlSystem::TensorMap outputTensorMap;
    Timer timer;
    timer.tic();
    for(int j=0;j<100;j++){
    	bool res = snpe->execute(input.get(), outputTensorMap);
    	cout << "execute finish" <<endl;
    	if (!res) {
        	cout << "execute failed:" << zdl::DlSystem::getLastErrorString() << endl;
    	}
    }
    double diff = timer.toc("Total");
    printf("diff======>:%f\n",diff/100);
    cout<< "write output:" << endl;
	auto tensorPtr = outputTensorMap.getTensor((*outputTensorNames).at(0));
    std::ofstream fout("./detection_out.txt");
	int i = 0;
	for(auto it = tensorPtr->cbegin(); it!=tensorPtr->cend();it++)
	{
        fout << *it << endl;
	    cout << *it << endl;
	}
    fout.close();
    cout << "end ~~~~!!!!!" << endl;
}
