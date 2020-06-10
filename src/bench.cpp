#include "implem.hh"
#include "gpuimplem.hh"

#include <benchmark/benchmark.h>
#include <SDL2/SDL.h>
#include <iostream>

std::string filename;
std::string maskname;

void BM_implem_cpu(benchmark::State& st)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface* image = nullptr;
    SDL_Surface* mask = nullptr;

    image = SDL_LoadBMP(filename.c_str());
    mask = SDL_LoadBMP(maskname.c_str());

    for (auto _ : st)
        CPU::Implem(image, mask);

    SDL_FreeSurface(image);
    SDL_FreeSurface(mask);
    SDL_Quit();
}

void BM_implem_gpu(benchmark::State& st)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface* image = nullptr;
    SDL_Surface* mask = nullptr;

    image = SDL_LoadBMP(filename.c_str());
    mask = SDL_LoadBMP(maskname.c_str());

    for (auto _ : st)
        GPU::Implem(image, mask);

    SDL_FreeSurface(image);
    SDL_FreeSurface(mask);
    SDL_Quit();
}

int main(int argc, char **argv)
{

    BENCHMARK(BM_implem_cpu)
        ->Unit(benchmark::kMillisecond)
        ->UseRealTime();

    BENCHMARK(BM_implem_gpu)
        ->Unit(benchmark::kMillisecond)
        ->UseRealTime();

    ::benchmark::Initialize(&argc, argv);
    
    std::cout << "argv : " << argv[1] << "\n";
    
    if (argc < 3)
    {
        std::cout << "USAGE: bench input_file mask_file\n";
        return 0;
    }

    filename = argv[1];
    maskname = argv[2];
    ::benchmark::RunSpecifiedBenchmarks();
}
