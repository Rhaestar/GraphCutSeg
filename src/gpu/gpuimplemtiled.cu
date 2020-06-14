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

#define K 4
#define BLOCK_SIZE 32
#define TILE_SIZE (BLOCK_SIZE + 2)

    template<typename T>
    __device__ inline T* eltPtr(T *baseAddress, size_t col, size_t row,
        size_t pitch)
    {
        return (T*)((char*)baseAddress + row * pitch + col * sizeof(T));
    }

    __global__ void Push(int* excessFlows, int* weightsUp, int* weightsDown,
        int* weightsLeft, int* weightsRight, uint32_t* heights,
        uint32_t heightMax, uint32_t width, uint32_t height, size_t pitch)
    {

        //Collaborative loading
        __shared__ int s_excess[BLOCK_SIZE][BLOCK_SIZE];

        size_t x = blockDim.x * blockIdx.x + threadIdx.x;
        size_t y = blockDim.x * blockIdx.y + threadIdx.y * K;

        size_t x_tile = threadIdx.x;
        size_t y_tile = threadIdx.y * K;

        if (x >= height || y >= width)
            return;

        for (size_t i = 0; i < K && y + i < width; i++)
        {
            s_excess[x_tile][y_tile + i] =
                *eltPtr(excessFlows, y + i, x, pitch); 
        }

        __syncthreads();

        //Flow propagation

        //RIGHT
        int ef = 0;
        for (size_t i = 0; i < K && y + i < width; i++)
        {
            ef += s_excess[x_tile][y_tile + i];
            int *wRight = eltPtr(weightsRight, y + i, x, pitch);
            int flow = min(*wRight, ef);
            *wRight -= flow;
            s_excess[x_tile][y_tile + i] = ef - flow;
            ef = flow;
        }
        
        //DOWN
        ef = 0;
        for (size_t i = 0; i < K && y + i < height; i++)
        {
            ef += s_excess[y_tile + i ][x_tile];
            int *wDown = eltPtr(weightsDown, x, y + i, pitch);
            int flow = min(*wDown, ef);
            *wDown -= flow;
            s_excess[y_tile + i][x_tile] = ef - flow;
            ef = flow;
        }
        
        //LEFT
        ef = 0;
        for (size_t i = 1; i <= K; i++)
        {
            while (y + K - i >= width)
                i++;
            ef += s_excess[x_tile][y_tile + K - i];
            int *wLeft = eltPtr(weightsLeft, y + K - i, x, pitch);
            int flow = min(*wLeft, ef);
            *wLeft -= flow;
            s_excess[x_tile][y_tile + K - i] = ef - flow;
            ef = flow;
        }
        //UP
        ef = 0;
        for (size_t i = 1; i <= K; i++)
        {
            while (y + K - i >= height)
                i++;
            ef += s_excess[y_tile + K - i][x_tile];
            int *wUp = eltPtr(weightsUp, x, y + K - i, pitch);
            int flow = min(*wUp, ef);
            *wUp -= flow;
            s_excess[y_tile + K - i][x_tile] = ef - flow;
            ef = flow;
        }
        __syncthreads();

        //Collaborative unloading

        for (size_t i = 0; i < K && y + i < width; i++)
        {
            *eltPtr(excessFlows, y + i, x, pitch) = s_excess[x_tile][y_tile + i]; 
        }
    }

    __global__ void Relabel(int* excessFlows,
        int* weightsUp, int* weightsDown, int* weightsLeft,
        int* weightsRight, uint32_t* heights, uint32_t* heightsTemp,
        uint32_t heightMax, unsigned width, unsigned height, size_t pitch)
    {
        size_t x = blockDim.x * blockIdx.x + threadIdx.x;
        size_t y = blockDim.y * blockIdx.y + threadIdx.y;

        if (x >= width || y >= height)
            return;

        int* currFlow = eltPtr(excessFlows, x, y, pitch);
        uint32_t* currHeight = eltPtr(heights, x, y, pitch);
        uint32_t* currHeightTemp = eltPtr(heightsTemp, x, y, pitch);

        if (*currFlow > 0 && *currHeight < heightMax)
        {
            uint32_t newHeight = heightMax;
            if (y > 0 && *eltPtr(weightsUp, x, y, pitch) > 0)
            {
                newHeight = min(newHeight, *eltPtr(heights, x, y - 1,
                    pitch) + 1);
            }
            if (x > 0 && *eltPtr(weightsLeft, x, y, pitch) > 0)
            {
                newHeight = min(newHeight, *eltPtr(heights, x - 1, y,
                    pitch) + 1);
            }
            if (y < height - 1 && *eltPtr(weightsDown, x, y, pitch) > 0)
            {
                newHeight = min(newHeight, *eltPtr(heights, x, y + 1,
                    pitch) + 1);
            }
            if (x < width - 1 && *eltPtr(weightsRight, x, y, pitch) > 0)
            {
                newHeight = min(newHeight, *eltPtr(heights, x + 1, y,
                    pitch) + 1);
            }

            *currHeightTemp = newHeight;
        }
    }

    __global__ void isActive(int* excessFlows, uint32_t* heights,
        uint32_t heightMax, unsigned width, unsigned height, size_t pitch,
        int* isAnyActive)
    {
        size_t x = blockDim.x * blockIdx.x + threadIdx.x;
        size_t y = blockDim.y * blockIdx.y + threadIdx.y;

        if (x >= width || y >= height)
            return;

        int* currFlow = eltPtr(excessFlows, x, y, pitch);
        uint32_t* currHeight = eltPtr(heights, x, y, pitch);

        if (*currFlow > 0 && *currHeight < heightMax)
        {
            atomicAdd(currFlow, 1);
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
        uint32_t heightMax = 10;
        float sigma = 10.f;
        float lambda = 1.f;
        int param = 10;

        uint8_t* bitmask = (uint8_t*)calloc(height * width, sizeof(uint8_t));

        int* weightsUp    = (int*)calloc(height * width, sizeof(int));
        int* weightsDown  = (int*)calloc(height * width, sizeof(int));
        int* weightsLeft  = (int*)calloc(height * width, sizeof(int));
        int* weightsRight = (int*)calloc(height * width, sizeof(int));

        uint32_t* heights = (uint32_t*)calloc(height * width,
            sizeof(uint32_t));

        int* excessFlows = (int*)calloc(height * width, sizeof(int));

        fillHists(foreHist, backHist, image, mask, bitmask);

        int maxCap = InitializeCapacities(weightsUp, weightsDown,
            weightsLeft, weightsRight, image, sigma, param);

        maxCap += 1;

        InitializeExcess(excessFlows, image, foreHist, backHist, bitmask,
            maxCap, lambda);

        int w = std::ceil((float)width / BLOCK_SIZE);
        int h = std::ceil((float)height / BLOCK_SIZE);

        dim3 dimBlockP(BLOCK_SIZE, BLOCK_SIZE / K);
        dim3 dimBlockR(BLOCK_SIZE, BLOCK_SIZE);
        dim3 dimGrid(w, h);

        size_t pitch;
        int *d_weightsUp, *d_weightsDown, *d_weightsLeft, *d_weightsRight,
            *d_excessFlows;

        uint32_t *d_heights, *d_heightsTemp;

        cudaMallocPitch((void **) &d_weightsUp, &pitch, width * sizeof(int),
            height);
        cudaMallocPitch((void **) &d_weightsDown, &pitch, width * sizeof(int),
            height);
        cudaMallocPitch((void **) &d_weightsLeft, &pitch, width * sizeof(int),
            height);
        cudaMallocPitch((void **) &d_weightsRight, &pitch, width * sizeof(int),
            height);
        cudaMallocPitch((void **) &d_excessFlows, &pitch, width * sizeof(int),
            height);

        cudaMallocPitch((void **) &d_heights, &pitch, width * sizeof(int),
            height);
        cudaMallocPitch((void **) &d_heightsTemp, &pitch, width * sizeof(int),
            height);

        cudaMemcpy2D(d_weightsUp, pitch, weightsUp, width * sizeof(int),
            width * sizeof(int), height, cudaMemcpyHostToDevice);
        cudaMemcpy2D(d_weightsDown, pitch, weightsDown, width * sizeof(int),
            width * sizeof(int), height, cudaMemcpyHostToDevice);
        cudaMemcpy2D(d_weightsLeft, pitch, weightsLeft, width * sizeof(int),
            width * sizeof(int), height, cudaMemcpyHostToDevice);
        cudaMemcpy2D(d_weightsRight, pitch, weightsRight, width * sizeof(int),
            width * sizeof(int), height, cudaMemcpyHostToDevice);
        cudaMemcpy2D(d_excessFlows, pitch, excessFlows, width * sizeof(int),
            width * sizeof(int), height, cudaMemcpyHostToDevice);

        cudaMemcpy2D(d_heights, pitch, heights, width * sizeof(uint32_t),
            width * sizeof(uint32_t), height, cudaMemcpyHostToDevice);
        cudaMemcpy2D(d_heightsTemp, pitch, d_heights, pitch,
            width * sizeof(uint32_t), height, cudaMemcpyDeviceToDevice);

        unsigned ip = 0;

        int isAnyActive = 1;
        int falseUtil = 0;
        int *d_isAnyActive;
        cudaMalloc((void**) &d_isAnyActive, sizeof(int));

        while (ip < 100 && isAnyActive > 0)
        {
            cudaMemcpy(d_isAnyActive, &falseUtil, sizeof(int),
                cudaMemcpyHostToDevice);

            Relabel<<<dimGrid, dimBlockR>>>(d_excessFlows,
                d_weightsUp, d_weightsDown, d_weightsLeft, d_weightsRight,
                d_heights, d_heightsTemp, heightMax,
                width, height, pitch);
            
            cudaDeviceSynchronize();

            cudaMemcpy2D(d_heights, pitch, d_heightsTemp, pitch,
                width * sizeof(uint32_t), height, cudaMemcpyDeviceToDevice);

            cudaDeviceSynchronize();

            Push<<<dimGrid, dimBlockP>>>(d_excessFlows,
                d_weightsUp, d_weightsDown, d_weightsLeft, d_weightsRight,
                d_heights, heightMax, width, height, pitch);
            
            cudaDeviceSynchronize();

            isActive<<<dimGrid, dimBlockR>>>(d_excessFlows, d_heights, heightMax,
                width, height, pitch, d_isAnyActive);

            cudaDeviceSynchronize();


            cudaMemcpy(&isAnyActive, d_isAnyActive, sizeof(int),
                cudaMemcpyDeviceToHost);
            std::cout << "new " << isAnyActive << "\n";
            ip++;
        }

        cudaMemcpy2D(heights, width * sizeof(uint32_t), d_heights, pitch,
            width * sizeof(uint32_t), height, cudaMemcpyDeviceToHost);
        SavePicture(heights, width, height, heightMax);

        cudaFree(d_weightsUp);
        cudaFree(d_weightsDown);
        cudaFree(d_weightsLeft);
        cudaFree(d_weightsRight);
        cudaFree(d_excessFlows);
        cudaFree(d_heights);
        cudaFree(d_heightsTemp);

        free(bitmask);
        free(weightsUp);
        free(weightsDown);
        free(weightsLeft);
        free(weightsRight);
        free(heights);
        free(excessFlows);

    }
}