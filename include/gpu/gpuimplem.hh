#pragma once

#include "histogram.hh"
#include "SDL.h"

namespace GPU
{
    void FillHists(Histogram& foreHist, Histogram& backHist, SDL_Surface* image,
        SDL_Surface* mask, uint8_t* bitmask);

    void Implem(SDL_Surface* image, SDL_Surface* mask);

    int ComputeDiff(uint8_t r1, uint8_t g1, uint8_t b1,
        uint8_t r2, uint8_t g2, uint8_t b2, float sigma, int param);

    int InitializeCapacities(int* weightsUp, int* weightsDown,
        int* weightsLeft, int* weightsRight, SDL_Surface* image,
        float sigma, int param);

    void InitializeExcess(int* excessFlows, SDL_Surface* image,
        Histogram& foreHist, Histogram& backHist,
        uint8_t* bitmask, int k, float lambda);
}
