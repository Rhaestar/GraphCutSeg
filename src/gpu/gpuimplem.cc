#include "gpuimplem.hh"

#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <cfloat>
#include "SDL.h"

namespace GPU
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

    int ComputeDiff(uint8_t r1, uint8_t g1, uint8_t b1,
        uint8_t r2, uint8_t g2, uint8_t b2, float sigma, int param)
    {
        int rdiff = r1 - r2;
        int gdiff = g1 - g2;
        int bdiff = b1 - b2;

        float sqdiff = rdiff * rdiff + gdiff * gdiff + bdiff * bdiff;
        return (int)((float)(param) * expf(-sqdiff / (2 * sigma * sigma)));
    }

    int InitializeCapacities(int* weightsUp, int* weightsDown,
        int* weightsLeft, int* weightsRight, SDL_Surface* image,
        float sigma, int param)
    {
        SDL_LockSurface(image);

        int maxCap = 0;

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
                        ComputeDiff(r1, g1, b1, r2, g2, b2, sigma, param);
                    maxCap = std::max(maxCap, weightsUp[i * image->w + j]);
                }
                if (i < image->h - 1)
                {
                    uint32_t pixel2 = *(uint32_t*)(pixels + (i + 1) *
                        image->pitch + j * 4);
                    SDL_GetRGB(pixel2, fmt, &r2, &g2, &b2);
                    weightsDown[i * image->w + j] =
                        ComputeDiff(r1, g1, b1, r2, g2, b2, sigma, param);
                    maxCap = std::max(maxCap, weightsDown[i * image->w + j]);
                }
                if (j > 0)
                {
                    uint32_t pixel2 = *(uint32_t*)(pixels + i *
                        image->pitch + (j - 1) * 4);
                    SDL_GetRGB(pixel2, fmt, &r2, &g2, &b2);
                    weightsLeft[i * image->w + j] =
                        ComputeDiff(r1, g1, b1, r2, g2, b2, sigma, param);
                    maxCap = std::max(maxCap, weightsLeft[i * image->w + j]);
                }
                if (j < image->w - 1)
                {
                    uint32_t pixel2 = *(uint32_t*)(pixels + i *
                        image->pitch + (j + 1) * 4);
                    SDL_GetRGB(pixel2, fmt, &r2, &g2, &b2);
                    weightsRight[i * image->w + j] =
                        ComputeDiff(r1, g1, b1, r2, g2, b2, sigma, param);
                    maxCap = std::max(maxCap, weightsRight[i * image->w + j]);
                }
            }
        }

        SDL_UnlockSurface(image);

        return maxCap;
    }

    void InitializeExcess(int *excessFlows, SDL_Surface* image,
        Histogram& foreHist, Histogram& backHist,
        uint8_t* bitmask, int k, float lambda)
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
                        excessFlows[i * image->w + j] = (int)(lambda * pbkg -
                            lambda * pobj);
                        break;
                    case 1:
                        excessFlows[i * image->w + j] = k;
                        break;
                    case 2:
                        excessFlows[i * image->w + j] = -k;
                        break;
                    default:
                        std::cout << "default\n";
                        break;
                }
            }
        }

        SDL_UnlockSurface(image);

    }

    int IsAnyActive(int* excessFlows, uint32_t* heights, uint32_t width,
        uint32_t height, uint32_t heightMax)
    {
        int ret = 0;
        for (uint32_t i = 0; i < height; ++i)
        {
            for (uint32_t j = 0; j < width; ++j)
            {
                bool test1 = excessFlows[i * width + j] > 0;
                int it = excessFlows[i * width + j];
                it += 1;
                bool test2 = heights[i * width + j] < heightMax;
                if (test1 && test2)
                    ret++;
            }
        }
        return ret;
    }

    void Push(int* excessFlows, int* weightsUp, int* weightsDown,
        int* weightsLeft, int* weightsRight, uint32_t* heights,
        uint32_t heightMax, uint32_t width, uint32_t height)
    {
        for (unsigned i = 0; i < height; ++i)
        {
            for (unsigned j = 0; j < width; ++j)
            {
                int currFlow = excessFlows[i * width + j];
                uint32_t currHeight = heights[i * width + j];

                if (currFlow > 0 && currHeight < heightMax)
                {
                    if (i > 0 && currHeight - 1 == heights[(i-1) * width + j])
                    {
                        int flow = std::min(currFlow,
                            weightsUp[i * width + j]);
                        excessFlows[i * width + j] -= flow;
                        excessFlows[(i - 1) * width + j] += flow;
                        weightsUp[i * width + j] -= flow;
                        weightsDown[(i - 1) * width + j] += flow;
                    }
                    if (j > 0 && currHeight - 1 == heights[i * width + j - 1])
                    {
                        int flow = std::min(currFlow,
                            weightsLeft[i * width + j]);
                        excessFlows[i * width + j] -= flow;
                        excessFlows[i * width + j - 1] += flow;
                        weightsLeft[i * width + j] -= flow;
                        weightsRight[i * width + j - 1] += flow;
                    }
                    if (i < height - 1 && currHeight - 1 == heights[(i+1) * width + j])
                    {
                        int flow = std::min(currFlow,
                            weightsDown[i * width + j]);
                        excessFlows[i * width + j] -= flow;
                        excessFlows[(i+1) * width + j] += flow;
                        weightsDown[i * width + j] -= flow;
                        weightsUp[(i+1) * width + j] += flow;
                    }
                    if (j < width - 1 && currHeight - 1 == heights[i * width + j
                        + 1])
                    {
                        int flow = std::min(currFlow,
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

    void Relabel(int* excessFlows,
        int* weightsUp, int* weightsDown, int* weightsLeft,
        int* weightsRight, uint32_t* heights, uint32_t* heightsTemp,
        uint32_t heightMax, unsigned width, unsigned height)
    {
        for (unsigned i = 0; i < height; ++i)
        {
            for (unsigned j = 0; j < width; ++j)
            {
                int currFlow = excessFlows[i * width + j];
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
        uint32_t heightMax = 100;
        float sigma = 5.f;
        float lambda = 10.f;
        int param = 1;

        uint8_t* bitmask = (uint8_t*)calloc(height * width, sizeof(uint8_t));

        int* weightsUp    = (int*)calloc(height * width, sizeof(int));
        int* weightsDown  = (int*)calloc(height * width, sizeof(int));
        int* weightsLeft  = (int*)calloc(height * width, sizeof(int));
        int* weightsRight = (int*)calloc(height * width, sizeof(int));

        uint32_t* heights = (uint32_t*)calloc(height * width,
            sizeof(uint32_t));
        //uint32_t* heights_temp = (uint32_t*)calloc(height * width,
        //    sizeof(uint32_t));

        int* excessFlows = (int*)calloc(height * width, sizeof(int));

        fillHists(foreHist, backHist, image, mask, bitmask);

        int maxCap = InitializeCapacities(weightsUp, weightsDown,
            weightsLeft, weightsRight, image, sigma, param);

        maxCap += 1;

        InitializeExcess(excessFlows, image, foreHist, backHist, bitmask,
            maxCap, lambda);

        unsigned ip = 0;

        while (ip < 1000 &&IsAnyActive(excessFlows, heights,width, height,
            heightMax))
        {
            //std::cout << IsAnyActive(excessFlows, heights,width, height, heightMax) << "\n";
            Relabel(excessFlows, weightsUp, weightsDown, weightsLeft,
                weightsRight, heights, heights, heightMax,
                width, height);

            /*for (unsigned i = 0; i < height; ++i)
            {
                for (unsigned j = 0; j < width; ++j)
                {
                    heights[i * width + j] = heights_temp[i * width + j];
                }
            }*/

            Push(excessFlows, weightsUp, weightsDown, weightsLeft,
                weightsRight, heights, heightMax, width, height);
            ip++;
            //std::cout << "new " << ip << "\n";
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
