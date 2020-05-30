#include "implem.hh"

#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <cfloat>
#include "SDL.h"

namespace CPU
{
    void fillHists(Histogram& foreHist, Histogram& backHist, SDL_Surface* image,
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

    void InitializeExcess(float *excessFlows, SDL_Surface* image,
        Histogram& foreHist, Histogram& backHist,
        uint8_t* bitmask, float k, float lambda)
    {
        SDL_LockSurface(image);

        uint8_t* pixels = (uint8_t*)image->pixels;
        SDL_PixelFormat* fmt = image->format;

        uint8_t r;
        uint8_t g;
        uint8_t b;

        uint32_t pixel;

        float pobj;
        float pbkg;

        for (int i = 0; i < image->h; ++i)
        {
            for (int j = 0; j < image->w; ++j)
            {
                switch (bitmask[i * image->w + j])
                {
                    case 0:
                        pixel = *(uint32_t*)(pixels + i *
                            image->pitch + j * 4);
                        SDL_GetRGB(pixel, fmt, &r, &g, &b);
                        pobj = foreHist.GetProba(r, g, b);
                        pbkg = backHist.GetProba(r, g, b);
                        if (pobj == 0.f)
                            pobj = FLT_MIN;
                        if (pbkg == 0.f)
                            pbkg = FLT_MIN;
                        pobj = -logf(pobj);
                        pbkg = -logf(pbkg);
                        excessFlows[i * image->w + j] = lambda * pbkg -
                            lambda * pobj;
                        break;
                    case 1:
                        excessFlows[i * image->w + j] = k * 10000;
                        break;
                    case 2:
                        excessFlows[i * image->w + j] = -k * 10000;
                        break;
                    default:
                        break;
                }
            }
        }

        SDL_UnlockSurface(image);

    }

    int IsAnyActive(float* excessFlows, uint32_t* heights, uint32_t width,
        uint32_t height, uint32_t heightMax)
    {
        int ret = 0;
        for (unsigned i = 0; i < height; ++i)
        {
            for (unsigned j = 0; j < width; ++j)
            {
                if (excessFlows[i * width + j] > 0 &&
                    heights[i * width + j] < heightMax)
                    ret++;
            }
        }
        return ret;
    }

    void Push(float* excessFlows, float* weightsUp, float* weightsDown,
        float* weightsLeft, float* weightsRight, uint32_t* heights,
        uint32_t heightMax, uint32_t width, uint32_t height)
    {
        for (unsigned i = 0; i < height; ++i)
        {
            for (unsigned j = 0; j < width; ++j)
            {
                float currFlow = excessFlows[i * width + j];
                uint32_t currHeight = heights[i * width + j];

                if (currFlow > 0 && currHeight < heightMax)
                {
                    if (i > 0 && currHeight - 1 == heights[(i-1) * width + j])
                    {
                        float flow = std::min(currFlow,
                            weightsUp[i * width + j]);
                        excessFlows[i * width + j] -= flow;
                        excessFlows[(i - 1) * width + j] += flow;
                        weightsUp[i * width + j] -= flow;
                        weightsDown[(i - 1) * width + j] += flow;
                    }
                    if (j > 0 && currHeight - 1 == heights[i * width + j - 1])
                    {
                        float flow = std::min(currFlow,
                            weightsLeft[i * width + j]);
                        excessFlows[i * width + j] -= flow;
                        excessFlows[i * width + j - 1] += flow;
                        weightsLeft[i * width + j] -= flow;
                        weightsRight[i * width + j - 1] += flow;
                    }
                    if (i < height - 1 && currHeight - 1 == heights[(i+1) * width + j])
                    {
                        float flow = std::min(currFlow,
                            weightsDown[i * width + j]);
                        excessFlows[i * width + j] -= flow;
                        excessFlows[(i+1) * width + j] += flow;
                        weightsDown[i * width + j] -= flow;
                        weightsUp[(i+1) * width + j] += flow;
                    }
                    if (j < width - 1 && currHeight - 1 == heights[i * width + j
                        + 1])
                    {
                        float flow = std::min(currFlow,
                            weightsRight[i * width + j]);
                        excessFlows[i * width + j] -= flow;
                        excessFlows[i * width + j + 1] += flow;
                        weightsRight[i * width + j] -= flow;
                        weightsLeft[i * width + j + 1] += flow;
                    }
                }
            }
        }
    }

    void Relabel(float* excessFlows,
        float* weightsUp, float* weightsDown, float* weightsLeft,
        float* weightsRight, uint32_t* heights, uint32_t* heightsTemp,
        uint32_t heightMax, unsigned width, unsigned height)
    {
        for (unsigned i = 0; i < height; ++i)
        {
            for (unsigned j = 0; j < width; ++j)
            {
                float currFlow = excessFlows[i * width + j];
                uint32_t currHeight = heights[i * width + j];

                if (currFlow > 0 && currHeight < heightMax)
                {
                    uint32_t newHeight = heightMax;
                    if (i > 0 && weightsUp[i * width + j] > 0)
                    {
                        newHeight = std::min(newHeight,
                            heights[(i - 1) * width + j] + 1);
                    }
                    if (j > 0 && weightsLeft[i * width + j] > 0)
                    {
                        newHeight = std::min(newHeight,
                            heights[i * width + j - 1] + 1);
                    }
                    if (i < height - 1 && weightsDown[i * width + j] > 0)
                    {
                        newHeight = std::min(newHeight,
                            heights[(i + 1) * width + j] + 1);
                    }
                    if (j < width - 1 && weightsRight[i * width + j] > 0)
                    {
                        newHeight = std::min(newHeight,
                            heights[i * width + j + 1] + 1);
                    }
                    heightsTemp[i * width + j] = newHeight;
                }
            }
        }
    }

    void SavePicture(uint32_t* heights, uint32_t width, uint32_t height,
        uint32_t maxHeight)
    {
        SDL_Surface *image;

        image = SDL_CreateRGBSurface(0, width, height, 32,0,0,0,0);

        SDL_LockSurface(image);

        uint8_t* pixels = (uint8_t*)image->pixels;
        SDL_PixelFormat* fmt = image->format;

        for(unsigned i = 0; i < height; ++i)
        {
            for (unsigned j = 0; j < width; ++j)
            {
                uint32_t* pixel = (uint32_t*)(pixels + i * image->pitch +
                    j * 4);
                if (heights[i * width + j] > maxHeight - maxHeight)
                    *pixel = SDL_MapRGBA(fmt, 255, 255, 255, 255);
                else
                    *pixel = SDL_MapRGBA(fmt, 0, 0, 0, 255);

            }
        }

        SDL_UnlockSurface(image);
        SDL_SaveBMP(image, "output.bmp");
    }

    void Implem(SDL_Surface* image, SDL_Surface* mask)
    {
        Histogram backHist;
        Histogram foreHist;

        uint32_t width = image->w;
        uint32_t height = image->h;
        uint32_t heightMax = width * height;
        float sigma = 5.f;
        float lambda = 0.1f;

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

        InitializeExcess(excessFlows, image, foreHist, backHist, bitmask,
            maxCap, lambda);

        unsigned ip = 0;

        while (IsAnyActive(excessFlows, heights,width, height, heightMax)
                && ip < 1000)
        {
            std::cout << IsAnyActive(excessFlows, heights,width, height, heightMax) << "\n";
            Relabel(excessFlows, weightsUp, weightsDown, weightsLeft,
                weightsRight, heights, heights_temp, heightMax,
                width, height);

            for (unsigned i = 0; i < height; ++i)
            {
                for (unsigned j = 0; j < width; ++j)
                {
                    heights[i * width + j] = heights_temp[i * width + j];
                }
            }

            Push(excessFlows, weightsUp, weightsDown, weightsLeft,
                weightsRight, heights, heightMax, width, height);
            ip++;
            std::cout << "new " << ip << "\n";
        }

        /*for (unsigned i = 0; i < height; ++i)
        {
            for (unsigned j = 0; j< width; ++j)
                std::cout << excessFlows[i * width + j] << " ";
            std::cout << "\n";
        }*/

        SavePicture(heights, width, height, heightMax);

        free(bitmask);
        free(weightsUp);
        free(weightsDown);
        free(weightsLeft);
        free(weightsRight);
        free(heights);
        free(excessFlows);

    }
}
