#include "implem.hh"

#include <cstdint>
#include <iostream>
#include <cstdlib>
#include "SDL.h"

namespace CPU
{
    void fillHists(Histogram& foreHist, Histogram backHist, SDL_Surface* image,
        SDL_Surface* mask, uint8_t* bitmask)
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
                    bitmask[i * image->w + j] = 1;
                }

                else if (r == 0 && g == 0 && b == 255)
                {
                    uint32_t pixeli = *(uint32_t*)(pixels + i * image->pitch +
                        j * 4);
                    SDL_GetRGB(pixeli, fmti, &r, &g, &b);
                    backHist.AddElement(r, g, b);
                    bitmask[i * image->w + j] = 2;
                }
            }
        }

        SDL_UnlockSurface(image);
        SDL_UnlockSurface(mask);
    }

    float ComputeDiff(uint8_t r1, uint8_t g1, uint8_t b1,
        uint8_t r2, uint8_t g2, uint8_t b2, float sigma)
    {
        float rdiff = (float)(r1) - (float)(r2);
        float gdiff = (float)(g1) - (float)(g2);
        float bdiff = (float)(b1) - (float)(b2);

        float sqdiff = rdiff * rdiff + gdiff * gdiff + bdiff * bdiff;
        return expf(-sqdiff / (2 * sigma * sigma));
    }

    float InitializeCapacities(float* weightsUp, float* weightsDown,
        float* weightsLeft, float* weightsRight, SDL_Surface* image,
        float sigma)
    {
        SDL_LockSurface(image);

        float maxCap = 0;

        uint8_t* pixels = (uint8_t*)image->pixels;
        SDL_PixelFormat* fmt = image->format;

        uint8_t r1;
        uint8_t g1;
        uint8_t b1;
        uint8_t r2;
        uint8_t g2;
        uint8_t b2;


        for (int i = 0; i < image->h; ++i)
        {
            for (int j = 0; j < image->w; ++j)
            {
                uint32_t pixel = *(uint32_t*)(pixels + i * image->pitch +
                    j * 4);
                SDL_GetRGB(pixel, fmt, &r1, &g1, &b1);

                if (i > 0)
                {
                    uint32_t pixel2 = *(uint32_t*)(pixels + (i - 1) *
                        image->pitch + j * 4);
                    SDL_GetRGB(pixel2, fmt, &r2, &g2, &b2);
                    weightsUp[i * image->w + j] =
                        ComputeDiff(r1, g1, b1, r2, g2, b2, sigma);
                    maxCap = std::max(maxCap, weightsUp[i * image->w + j]);
                }
                if (i < image->h - 1)
                {
                    uint32_t pixel2 = *(uint32_t*)(pixels + (i + 1) *
                        image->pitch + j * 4);
                    SDL_GetRGB(pixel2, fmt, &r2, &g2, &b2);
                    weightsDown[i * image->w + j] =
                        ComputeDiff(r1, g1, b1, r2, g2, b2, sigma);
                    maxCap = std::max(maxCap, weightsDown[i * image->w + j]);
                }
                if (j > 0)
                {
                    uint32_t pixel2 = *(uint32_t*)(pixels + i *
                        image->pitch + (j - 1) * 4);
                    SDL_GetRGB(pixel2, fmt, &r2, &g2, &b2);
                    weightsLeft[i * image->w + j] =
                        ComputeDiff(r1, g1, b1, r2, g2, b2, sigma);
                    maxCap = std::max(maxCap, weightsLeft[i * image->w + j]);
                }
                if (j < image->w - 1)
                {
                    uint32_t pixel2 = *(uint32_t*)(pixels + i *
                        image->pitch + (j + 1) * 4);
                    SDL_GetRGB(pixel2, fmt, &r2, &g2, &b2);
                    weightsRight[i * image->w + j] =
                        ComputeDiff(r1, g1, b1, r2, g2, b2, sigma);
                    maxCap = std::max(maxCap, weightsRight[i * image->w + j]);
                }
            }
        }

        SDL_UnlockSurface(image);

        return maxCap;
    }

    void Implem(SDL_Surface* image, SDL_Surface* mask)
    {
        Histogram backHist;
        Histogram foreHist;

        int width = image->w;
        int height = image->h;
        float sigma = 10.f;

        uint8_t* bitmask = (uint8_t*)calloc(height * width, sizeof(uint8_t));

        float* weightsUp    = (float*)calloc(height * width, sizeof(float));
        float* weightsDown  = (float*)calloc(height * width, sizeof(float));
        float* weightsLeft  = (float*)calloc(height * width, sizeof(float));
        float* weightsRight = (float*)calloc(height * width, sizeof(float));

        uint32_t* heights = (uint32_t*)calloc(height * width,
            sizeof(uint32_t));
        uint32_t* heights_temp = (uint32_t*)calloc(height * width,
            sizeof(uint32_t));

        float* excessFlows = (float*)calloc(height * width, sizeof(float));

        fillHists(foreHist, backHist, image, mask, bitmask);

        float maxCap = InitializeCapacities(weightsUp, weightsDown,
            weightsLeft, weightsRight, image, sigma);

        maxCap += 1.f;

        for (int i = 0; i < height; ++i)
        {
            for(int j = 0; j < width; ++j)
            {
                if (weightsUp[i * width + j] > 1.f)
                std::cout << weightsUp[i * width + j] << " ";
            }
            std::cout << "\n";
        }

        free(bitmask);
        free(weightsUp);
        free(weightsDown);
        free(weightsLeft);
        free(weightsRight);
        free(heights);
        free(heights_temp);
        free(excessFlows);

    }
}
