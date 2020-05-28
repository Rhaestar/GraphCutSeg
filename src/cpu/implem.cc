#include "implem.hh"

#include <cstdint>
#include <iostream>
#include "SDL.h"

namespace CPU
{
    void fillHists(Histogram& foreHist, Histogram backHist, SDL_Surface* image,
        SDL_Surface* mask)
    {
        SDL_LockSurface(image);
        SDL_LockSurface(mask);

        uint8_t* pixels = (uint8_t*)image->pixels;
        uint8_t* masks  = (uint8_t*)mask->pixels;

        uint8_t r;
        uint8_t g;
        uint8_t b;

        SDL_PixelFormat* fmti = image->format;
        SDL_PixelFormat* fmtm = mask->format;

        for (int i = 0; i < image->h; ++i)
        {
            for (int j = 0; j < image->w; ++j)
            {
                uint32_t pixelm = *(uint32_t*)(masks + i * mask->pitch +
                    j * 4);
                SDL_GetRGB(pixelm, fmtm, &r, &g, &b);

                if (r == 255 && g == 0 && b == 0)
                {
                    uint32_t pixeli = *(uint32_t*)(pixels + i * image->pitch +
                        j * 4);
                    SDL_GetRGB(pixeli, fmti, &r, &g, &b);
                    foreHist.AddElement(r, g, b);
                }

                else if (r == 0 && g == 0 && b == 255)
                {
                    uint32_t pixeli = *(uint32_t*)(pixels + i * image->pitch +
                        j * 4);
                    SDL_GetRGB(pixeli, fmti, &r, &g, &b);
                    backHist.AddElement(r, g, b);
                }
            }
        }

        SDL_UnlockSurface(image);
        SDL_UnlockSurface(mask);
    }

    void Implem(SDL_Surface* image, SDL_Surface* mask)
    {
        Histogram backHist;
        Histogram foreHist;

        fillHists(foreHist, backHist, image, mask);
    }
}
