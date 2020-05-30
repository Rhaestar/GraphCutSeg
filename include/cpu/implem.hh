#pragma once

#include "histogram.hh"
#include "SDL.h"

namespace CPU
{
    void FillHists(Histogram& foreHist, Histogram& backHist, SDL_Surface* image,
        SDL_Surface* mask, uint8_t* bitmask);

    void Implem(SDL_Surface* image, SDL_Surface* mask);

    float ComputeDiff(uint8_t r1, uint8_t g1, uint8_t b1,
        uint8_t r2, uint8_t g2, uint8_t b2, float sigma);

    float InitializeCapacities(float* weightsUp, float* weightsDown,
        float* weightsLeft, float* weightsRight, SDL_Surface* image,
        float sigma);

    void InitializeExcess(float* excessFlows, SDL_Surface* image,
        Histogram& foreHist, Histogram& backHist,
        uint8_t* bitmask, float k, float lambda);
}
