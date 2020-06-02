#pragma once

#include <cstdint>

#define BUCKET_SIZE 32
#define ARRAY_SIZE (256 / 32)

namespace CPU
{
    class Histogram
    {
    public:

        Histogram()
            : size_(0)
        {
            for (int i = 0; i < ARRAY_SIZE * ARRAY_SIZE * ARRAY_SIZE; ++i)
                hist_[i] = 0;
        }

        ~Histogram()
        {}

        void AddElement(uint8_t r, uint8_t g, uint8_t b);
        float GetProba(uint8_t r, uint8_t g, uint8_t b);

    private:
        unsigned size_;
        unsigned hist_[ARRAY_SIZE * ARRAY_SIZE * ARRAY_SIZE];
    };
}
