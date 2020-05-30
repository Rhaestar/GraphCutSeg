#pragma once

#include <cstdint>

#define BUCKET_SIZE 16

namespace CPU
{
    class Histogram
    {
    public:

        Histogram()
            : size_(0)
        {
            for (int i = 0; i < BUCKET_SIZE * BUCKET_SIZE * BUCKET_SIZE; ++i)
                hist_[i] = 0;
        }

        ~Histogram()
        {}

        void AddElement(uint8_t r, uint8_t g, uint8_t b);
        float GetProba(uint8_t r, uint8_t g, uint8_t b);

    private:
        unsigned size_;
        unsigned hist_[BUCKET_SIZE * BUCKET_SIZE * BUCKET_SIZE];
    };
}
