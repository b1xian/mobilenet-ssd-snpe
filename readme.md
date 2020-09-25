example of snpe with [mobilenet-ssd](https://github.com/chuanqi305/MobileNet-SSD)
trained by [MobileNet-Caffe](https://github.com/shicai/MobileNet-Caffe)

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

**mobilenet-ssd model caffe to snpe**  
prepare snpe environment
```bash
nohup snpe-caffe-to-dlc --input_network ./model/deploy.prototxt \
--caffe_bin ./model/mobilenet_iter_73000.caffemodel \
--debug --o ./model/mobilenet_iter_73000.dlc \
> ./model/mobilenet_iter_73000.log 2>&1 &
```

**build on ubuntu**
```bash
mkdir build && cd build
cmake ..
```

**run mobilenet-ssd with snpe cpu**  
write detection output to ./detection_out.txt  
`./test-mobilenet-ssd-cpu ./model/mobilenet_iter_73000.dlc ./data/VOC_raw/1.raw`

**show mobilenet-ssd result**
```bash
python show_ssd_result.py --draw_img=./data/VOC_resize/1.jpg --detection_out=./detection_out.txt
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

**run mobilenet-ssd with snpe dsp**  
project: ./android 