#pragma once

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

        void AddElement(unsigned r, unsigned g, unsigned b);
        float GetProba(unsigned r, unsigned g, unsigned b);

    private:
        unsigned size_;
        unsigned hist_[BUCKET_SIZE * BUCKET_SIZE * BUCKET_SIZE];
    };
}
