#include "histogram.hh"

namespace CPU
{
    void Histogram::AddElement(uint8_t r, uint8_t g, uint8_t b)
    {
        hist_[r / BUCKET_SIZE * BUCKET_SIZE * BUCKET_SIZE +
            g / BUCKET_SIZE * BUCKET_SIZE + b / BUCKET_SIZE] += 1;
        size_++;
    }

    float Histogram::GetProba(uint8_t r, uint8_t g, uint8_t b)
    {
        return hist_[r / BUCKET_SIZE * BUCKET_SIZE * BUCKET_SIZE +
            g / BUCKET_SIZE * BUCKET_SIZE + b / BUCKET_SIZE] /
            (float)(size_);
    }
}
