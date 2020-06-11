#!/usr/bin/env python

import sys
from matplotlib.image import imread
import numpy as np

def main():
    file1, file2 = sys.argv[1:1+2]
    img1 = to_grayscale(imread(file1).astype(float))
    img2 = to_grayscale(imread(file2).astype(float))
    dist = compare_images(img1, img2)
    print("Euclidean distance:", dist)

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
