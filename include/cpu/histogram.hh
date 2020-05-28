#pragma once

namespace CPU
{
    class Histogram
    {
    public:

        Histogram(unsigned bucket_size)
            : bucket_size_(bucket_size)
            , size_(0)
        {
            hist_ = new unsigned[bucket_size_ * bucket_size_ * bucket_size_];
        }

        ~Histogram()
        {
            delete[] hist_;
        }

        void AddElement(unsigned r, unsigned g, unsigned b);
        float GetProba(unsigned r, unsigned g, unsigned b);

    private:
        unsigned bucket_size_;
        unsigned size_;
        unsigned* hist_;
    };
}
