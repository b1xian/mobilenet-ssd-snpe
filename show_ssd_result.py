import cv2
import numpy as np
import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--draw_img', type=str)
    parser.add_argument('--detection_out', type=str)
    args = parser.parse_args()

    img_path = args.draw_img
    detection_out = args.detection_out

    font = cv2.FONT_HERSHEY_SIMPLEX
    class_names= ["background",
                     "aeroplane", "bicycle", "bird", "boat",
                     "bottle", "bus", "car", "cat", "chair",
                     "cow", "diningtable", "dog", "horse",
                     "motorbike", "person", "pottedplant",
                     "sheep", "sofa", "train", "tvmonitor"]

    draw_img = cv2.imread(img_path)
    width, height, channel = draw_img.shape

    f = open(detection_out, 'r')
    lines = f.readlines()
    f.close()
    for i in range(0, len(lines), 7):
        out = np.asarray(lines[i + 1:i + 7], dtype=float)

        label = class_names[int(out[0])]
        prob = out[1]
        x = int(out[2] * width)
        y = int(out[3] * height)
        w = int(out[4] * width - x)
        h = int(out[5] * height - y)
        draw_img = cv2.putText(draw_img, str(label) + "%.2f" % prob, (x,y+20), font, 0.8, (0,255,0), 2)
        draw_img = cv2.rectangle(draw_img, (x,y), (x+w,y+h), (0,255,0), 2)
        print("label:{0}, prob:{1}, bbox:{2},{3},{4},{5}".format(label, prob, x, y, x+w, y+h))

    cv2.imwrite('./show_result.jpg', draw_img)


if __name__ == '__main__':
    main()