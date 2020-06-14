#!/usr/bin/env python

import os
import sys
from matplotlib.image import imread
import numpy as np

data = ["12003", "124084", "153077", "21077", "3096", "372047", "42049", "42078", "banana1", "banana2", "banana3", "ceramic", "flower", "grave", "sheep"]

PATH_TO_OUTPUTS = "./outputs"
PATH_TO_EXPECTED = "./expected_outputs"

def main():
    for d in data:
        if d.startswith(("1","2","3","4")):
            expected = to_grayscale(imread(os.path.join(
                PATH_TO_EXPECTED, d + ".png")).astype(float))
        else:
            expected = to_grayscale(imread(os.path.join(
                PATH_TO_EXPECTED, d + ".bmp")).astype(float))

        img_cpu = to_grayscale(imread(os.path.join(
            PATH_TO_OUTPUTS, "cpu_" + d + ".bmp")).astype(float))
        img_gpu = to_grayscale(imread(os.path.join(
            PATH_TO_OUTPUTS, "gpu_" + d + ".bmp")).astype(float))
        print(d)
        dist_cpu = compare_images(img_cpu, expected)
        print("CPU Euclidean distance:", dist_cpu)
        dist_gpu = compare_images(img_gpu, expected)
        print("GPU Euclidean distance:", dist_gpu)
        print("MEAN Euclidean distance:", (dist_cpu + dist_gpu) / 2)
        print("-------------------------------------------")

def compare_images(img1, img2):
    img1 = img1/np.linalg.norm(img1)
    img2 = img2/np.linalg.norm(img2)
    return np.linalg.norm(img1 - img2)

def to_grayscale(arr):
    if len(arr.shape) == 3:
        return np.average(arr, -1)  # average over the last axis (color channels)
    else:
        return arr

if __name__ == "__main__":
    main()
