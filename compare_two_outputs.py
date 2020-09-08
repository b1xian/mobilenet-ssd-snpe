import os
import numpy as np
import matplotlib.pyplot as plt
import argparse

plt.figure(figsize=(20,10), dpi=100)
plt.rcParams['font.sans-serif'] = ['Arial Unicode MS']


output_tensors = {
    "conv0/scale.conv0"
    , "conv0/relu.conv0"
    , "conv1/dw/scale.conv1/dw"
    , "conv1/dw/relu.conv1/dw"
    , "conv1/scale.conv1"
    , "conv1/relu.conv1"
    , "conv2/dw/scale.conv2/dw"
    , "conv2/dw/relu.conv2/dw"
    , "conv2/scale.conv2"
    , "conv2/relu.conv2"
    , "conv3/dw/scale.conv3/dw"
    , "conv3/dw/relu.conv3/dw"
    , "conv3/scale.conv3"
    , "conv3/relu.conv3"
    , "conv4/dw/scale.conv4/dw"
    , "conv4/dw/relu.conv4/dw"
}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--compare1', type=str)
    parser.add_argument('--compare2', type=str)
    args = parser.parse_args()

    file1 = args.compare1
    file2 = args.compare2

    content1 = open(file1)
    content2 = open(file2)

    lines1 = content1.readlines()
    lines2 = content2.readlines()

    lines1 = [line.strip("\n") for line in lines1]
    lines2 = [line.strip("\n") for line in lines2]

    num_layes = len(output_tensors)


    len_outputs = 1000
    output_names = []
    mean_diffs = []
    flag_index = 0
    layer_diffs = {}
    for i in range(num_layes):
        output_name = lines1[flag_index]
        output_names.append(output_name)

        outputs1 = lines1[flag_index+1:flag_index+1+len_outputs]
        outputs1 = np.asarray([float(ele) for ele in outputs1], dtype=float)

        outputs2 = lines2[flag_index+1:flag_index+1+len_outputs]
        outputs2 = np.asarray([float(ele) for ele in outputs2], dtype=float)

        flag_index += (len_outputs + 1)

        mean_diff = np.sum(np.abs(outputs2 - outputs1)) / len_outputs
        layer_diffs[output_name] = mean_diff
        mean_diffs.append(mean_diff)
    mean_diffs.append(mean_diffs[0])
    mean_diffs = mean_diffs[1:]
    ml = max(list(map(len, layer_diffs.keys())))
    for (k, v) in layer_diffs.items():
        print(k.ljust(ml), ':', v)
    plt.bar([i for i in range(num_layes)], mean_diffs)
    plt.ylabel('输出差异', fontsize=20)
    # plt.yticks(np.arange(0,1,0.1))
    plt.title('每层输出平均差异图', fontsize=20)
    plt.savefig('compare.jpg')


if __name__ == '__main__':

    main()

