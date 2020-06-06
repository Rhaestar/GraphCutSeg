#include "histogram.hh"
#include <iostream>

namespace GPU
{
    void Histogram::AddElement(uint8_t r, uint8_t g, uint8_t b)
    {
        int index = (int)(r) / BUCKET_SIZE * ARRAY_SIZE * ARRAY_SIZE +
            (int)(g) / BUCKET_SIZE * ARRAY_SIZE + (int)(b) / BUCKET_SIZE;
        hist_[index] += 1;
        size_++;
    }

    float Histogram::GetProba(uint8_t r, uint8_t g, uint8_t b)
    {
        int index = (int)(r) / BUCKET_SIZE * ARRAY_SIZE * ARRAY_SIZE +
            (int)(g) / BUCKET_SIZE * ARRAY_SIZE + (int)(b) / BUCKET_SIZE;
        //std::cout << index << "\n";
        float ret = hist_[index];
        return ret / (float)size_;
    }
}
