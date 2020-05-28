#include "implem.hh"

#include <CLI/CLI.hpp>
#include <SDL2/SDL.h>
#include <cstdint>

int main(int argc, char* argv[])
{
    std::string filename = "";
    std::string maskname = "";
    std::string mode = "CPU";

    CLI::App app{"graphcutseg"};
    app.add_option("input_file", filename, "Input Image")->required();
    app.add_option("Mask_file", maskname, "Mask Image")->required();
    app.add_set("-m", mode, {"GPU", "CPU"}, "Either 'GPU' or 'CPU'");

    CLI11_PARSE(app, argc, argv);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface* image = nullptr;
    SDL_Surface* mask = nullptr;

    if (filename.find(".bmp") != std::string::npos ||
        maskname.find(".bmp") != std::string::npos)
    {
        image = SDL_LoadBMP(filename.c_str());
        mask = SDL_LoadBMP(maskname.c_str());
    }
    else
    {
        std::cerr << "Invalid BMP file :" << filename << "\n";
        return 0;
    }

    CPU::Implem(image, mask);

    SDL_FreeSurface(image);
    SDL_FreeSurface(mask);
    SDL_Quit();
}
