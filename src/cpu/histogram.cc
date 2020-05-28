#include "histogram.hh"

namespace CPU
{
    void Histogram::AddElement(unsigned r, unsigned g, unsigned b)
    {
        hist_[r * bucket_size_ * bucket_size_ + g * bucket_size_ + b] += 1;
        size_++;
    }

    float Histogram::GetProba(unsigned r, unsigned g, unsigned b)
    {
        return hist_[r * bucket_size_ * bucket_size_ + g * bucket_size_ + b] /
            (float)(size_);
    }
}
