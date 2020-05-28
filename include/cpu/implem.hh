#pragma once

#include "histogram.hh"
#include "SDL.h"

namespace CPU
{
    void FillHists(Histogram& ForeHist, Histogram& BackHist, SDL_Surface* image,
        SDL_Surface* mask);

    void Implem(SDL_Surface* image, SDL_Surface* mask);
}
