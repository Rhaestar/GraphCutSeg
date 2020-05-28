#include "histogram.hh"

namespace CPU
{
    void Histogram::AddElement(unsigned r, unsigned g, unsigned b)
    {
        hist_[r / BUCKET_SIZE * BUCKET_SIZE * BUCKET_SIZE +
            g / BUCKET_SIZE * BUCKET_SIZE + b / BUCKET_SIZE] += 1;
        size_++;
    }

    float Histogram::GetProba(unsigned r, unsigned g, unsigned b)
    {
        return hist_[r / BUCKET_SIZE * BUCKET_SIZE * BUCKET_SIZE +
            g / BUCKET_SIZE * BUCKET_SIZE + b / BUCKET_SIZE] /
            (float)(size_);
    }
}
