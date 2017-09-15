#pragma once

#include <SDL2/SDL.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "random.h"
#include <vector>
#include <memory>
#include <functional>

namespace diamonds {
    template<typename T>
    using my_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;
    using Matrix = std::vector<std::vector<int>>;
    class SDL2Initializer
    {
    public:
        int screenWidth, screenHeight;
        explicit SDL2Initializer();
        ~SDL2Initializer();
    };

    class Game
    {
    public:
        explicit Game();
        void run();
    private:
        const std::string WINDOW_TITLE{"TeDiamonds"};
        const std::string PATH_TO_SPRITE{"resources/gfx/diamonds.png"};
        const std::string PATH_TO_SOUND{"resources/sounds/Jump_00.wav"};
        enum
        {
            WINDOW_WIDTH = 1152,
            WINDOW_HEIGHT = 640,
            DIAMOND_WIDTH = 64,
            DIAMOND_HEIGHT = 64,
            FIELD_SIZE_X = WINDOW_WIDTH / DIAMOND_WIDTH,
            FIELD_SIZE_Y = WINDOW_HEIGHT / DIAMOND_HEIGHT,
            MIN_LINE_SIZE = 3
        };
        SDL2Initializer initializer_;
        my_unique_ptr<SDL_Window> upWindow_;
        my_unique_ptr<SDL_Renderer> upRenderer_;
        my_unique_ptr<SDL_Texture> upSprite_;
        my_unique_ptr<Mix_Chunk> upSound_;
        bool isRunning_{true};
        bool draw_{true};
        const std::vector<SDL_Color> diamondColorList_
        {
            { 0xff, 0x00, 0x00, 0xc0 },
            { 0xff, 0x80, 0x00, 0xc0 },
            { 0xff, 0xff, 0x00, 0xc0 },
            { 0x80, 0xff, 0x00, 0xc0 },
            { 0x00, 0xff, 0xff, 0xc0 },
            { 0x00, 0x80, 0xff, 0xc0 },
            { 0x00, 0x00, 0xff, 0xc0 },
            { 0x80, 0x00, 0xff, 0xc0 },
            { 0xff, 0x00, 0xff, 0xc0 },
            { 0xff, 0x00, 0x80, 0xc0 }
        };
        Matrix field_;
        SDL_Texture *loadTextureFromFile(const std::string &filename);
        Random random_;
        int selectedX_{-1}, selectedY_{-1};
        long int score_{0};
        void userInputPhase();
        void updatePhase();
        void drawPhase();
        void findAndRemoveLines();
        void checkHoles();
    };
}

