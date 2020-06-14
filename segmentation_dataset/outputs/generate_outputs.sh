#!/bin/bash

data=("12003" "124084" "153077" "21077" "3096" "372047" "42049" "42078" "banana1" "banana2" "banana3" "ceramic" "flower" "grave" "sheep")

for d in "${data[@]}"; do
    d+=".bmp"
    ../../build/graphcutseg -m "CPU" ../inputs/$d ../masks/$d
    mv output.bmp cpu_$d
    ../../build/graphcutseg -m "GPU" ../inputs/$d ../masks/$d
    mv output.bmp gpu_$d
done
