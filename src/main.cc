#include "histogram.hh"

#include <CLI/CLI.hpp>
#include <SDL2/SDL.h>
#include <cstdint>

int main(int argc, char* argv[])
{
    std::string filename = "";
    std::string mode = "CPU";

    CLI::App app{"graphcutseg"};
    app.add_option("filename", filename, "Input Image")->required();
    app.add_set("-m", mode, {"GPU", "CPU"}, "Either 'GPU' or 'CPU'");

    CLI11_PARSE(app, argc, argv);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface* image = nullptr;

    if (filename.find(".bmp") != std::string::npos)
        image = SDL_LoadBMP(filename.c_str());
    else
    {
        std::cerr << "Invalid BMP file :" << filename << "\n";
        return 0;
    }

    SDL_PixelFormat* fmt = image->format;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint32_t* pixels = (uint32_t*)image->pixels;
    SDL_GetRGB(*(pixels + 100), fmt, &r, &g, &b);

    std::cout << "(" << (int)r << ", " << (int)g << ", " << (int)b << ")" <<
        "\n";

    SDL_Quit();

}
