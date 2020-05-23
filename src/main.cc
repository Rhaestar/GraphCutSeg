#include <CLI/CLI.hpp>
#include <SDL2/SDL.h>

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

    if (filename.find(".bmp"))
        image = SDL_LoadBMP(filename.c_str());
    else
    {
        std::cerr << "Invalid BMP file :" << filename << "\n";
        return 0;
    }

    SDL_Quit();

}
