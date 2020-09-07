import cv2
import numpy as np
import os
import random

VOC_path = './data/VOC2007'
VOC_raw_path = './data/VOC_raw'
VOC_resize_path = './data/VOC_resize'
raw_list_file = './data/VOC_raw_list.txt'
raw_file_names = ''

if not os.path.exists(VOC_raw_path):
    os.makedirs(VOC_raw_path)
if not os.path.exists(VOC_resize_path):
    os.makedirs(VOC_resize_path)

imgs = os.listdir(VOC_path)
for i, file_name in enumerate(imgs):
    img = cv2.imread(os.path.join(VOC_path, file_name))
    resized_img = cv2.resize(img, (300, 300), cv2.INTER_LINEAR)
    resize_file_name = os.path.join(VOC_resize_path, str(i) + '.jpg')
    cv2.imwrite(resize_file_name, resized_img)


    mean,std = cv2.meanStdDev(img)
    mean = mean.reshape(1,3)
    std = std.reshape(1,3)

    resized_img = (resized_img-mean)/(0.000001 + std)
    resized_img_data = np.array(resized_img, np.float32)
    raw_file_name = os.path.join(VOC_raw_path, str(i) + '.raw')
    resized_img_data.tofile(raw_file_name)

    raw_file_names += (raw_file_name + '\n')


with open(raw_list_file, 'w') as f:
    f.write(raw_file_names)
f.close()
