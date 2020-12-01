example of snpe with [mobilenet-ssd](https://github.com/chuanqi305/MobileNet-SSD)
trained by [Caffe](https://github.com/weiliu89/caffe/tree/ssd)  
caffe model [MobileNet-Caffe](https://github.com/shicai/MobileNet-Caffe)

├── CMakeLists.txt  
├── android  
├── include  
├── lib  
├── data  
│   ├── VOC2007  
│   ├── VOC_raw  
│   ├── VOC_raw_list.txt  
│   └── VOC_resize  
├── model  
│   ├── deploy.prototxt  
│   ├── mobilenet_iter_73000.caffemodel  
│   ├── mobilenet_iter_73000.dlc  
│   ├── mobilenet_iter_73000_int8.dlc  
└── util  
├── test_snpe_cpp_demo.cpp  
├── test-mobilenet-ssd-cpu  
├── show_ssd_result.py  
├── image_to_raw.py  
├── readme.md  

**build**
```bash
./build.sh
```
**test on device**
```bash
./run_example.sh
```

**mobilenet-ssd model caffe to snpe**  
prepare snpe environment
```bash
nohup snpe-caffe-to-dlc --input_network ./model/deploy.prototxt \
--caffe_bin ./model/mobilenet_iter_73000.caffemodel \
--debug --o ./model/mobilenet_iter_73000.dlc \
> ./model/mobilenet_iter_73000.log 2>&1 &
```

**prepare VOC raw data for snpe quantize**  
read from : data/VOC2007  
generate resize image : data/VOC_resize  
generate resize raw data : data/VOC_raw  
generate raw list : data/VOC_raw_list.txt  
```
python image_to_raw.py
```  

**quantize model to int8**
```bash
nohup snpe-dlc-quantize --debug3 \
--input_dlc ./model/mobilenet_iter_73000.dlc \
--input_list ./data/VOC_raw_list.txt \
--output_dlc ./model/mobilenet_iter_73000_int8.dlc \
> ./model/mobilenet_iter_73000_int8.log 2>&1 &
```



***********************************************
** How to estimate runtime performance on dsp**  
***********************************************

=> adb push snpe-net-run to path /data/local/tmp/test_demo/

=> cd /data/local/tmp/test_demo

=> chmod 777 snpe-net-run 

=> settimg library path:
   export LD_LIBRARY_PATH=./${LD_LIBRARY_PATH}
   export ADSP_LIBRARY_PTAH=./:${ADSP_LIBRATY_PATH}

=> Then, need to do next step, Let's to show follow cmd:
   snpe-net-run --container *.dlc --input_list list.txt --use_dsp 

=> if running is successful, we can have one output folder.
   
=> adb pull output folder to your local snpe develop path. follow cmd:
   snpe-diagview --input_log SNPEDiag.log --output LOGCSVFile > Log 2>&1   
  (Note: find file SNPEDiag.log from output folder) 




*****************************************************
*********** NOTE 1: runing run_example.sh************
*****************************************************

On soc SA8155P, with problem about segmentation fault. 
Solution: 
copy library “libc++_shared” to "/system/lib" 

*****************************************************
********* NOTE 2: cross complie *********************
*****************************************************

android-ndk-r21d with problem about lirary "libc++.so" and some undefined reference.
Solution:
delete library "libc++.so" from "./lib/snpe/armv7a-android"  


