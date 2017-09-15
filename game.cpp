#include "game.hpp"
#include <sstream>
#include <stdexcept>
#ifdef DEBUG
#include <iostream>
#endif

//esources/sounds/Collect_Point_00.wav

diamonds::Game::Game():
upWindow_{SDL_CreateWindow(WINDOW_TITLE.c_str(),
                               (initializer_.screenWidth - WINDOW_WIDTH) / 2,
                               (initializer_.screenHeight - WINDOW_HEIGHT) / 2,
                               WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN),
              [](SDL_Window *pWindow){ SDL_DestroyWindow(pWindow); }},
upRenderer_{SDL_CreateRenderer(upWindow_.get(), -1, SDL_RENDERER_ACCELERATED |
                               SDL_RENDERER_PRESENTVSYNC),
            [](SDL_Renderer *pRenderer){ SDL_DestroyRenderer(pRenderer); }},
upSprite_{loadTextureFromFile(PATH_TO_SPRITE),
            [](SDL_Texture *pTexture){ SDL_DestroyTexture(pTexture); }},
upSound_{Mix_LoadWAV(PATH_TO_SOUND.c_str()),
         [](Mix_Chunk *pSound){ Mix_FreeChunk(pSound); }}
{
    field_.reserve(FIELD_SIZE_Y);
    for(auto y = 0u; y < FIELD_SIZE_Y; ++y)
    {
        std::vector<int> tmp;
        tmp.reserve(FIELD_SIZE_X);
        for(auto x = 0u; x < FIELD_SIZE_X; ++x)
            tmp.push_back(random_.nextInt(0, diamondColorList_.size() - 1));
        field_.push_back(tmp);
    }
    SDL_SetRenderDrawBlendMode(upRenderer_.get(), SDL_BLENDMODE_BLEND);
}

void diamonds::Game::run()
{
    while(isRunning_)
    {
        userInputPhase();
        updatePhase();
        drawPhase();
    }
}

SDL_Texture *diamonds::Game::loadTextureFromFile(const std::string& filename)
{
    auto pSurface = IMG_Load(filename.c_str());
    if(!pSurface)
        throw std::runtime_error(std::string("Error while loading image: ") +
                                 filename + std::string(" - ") +
                                 std::string(SDL_GetError()));
    auto pTexture = SDL_CreateTextureFromSurface(upRenderer_.get(), pSurface);
    SDL_FreeSurface(pSurface);
    pSurface = nullptr;
    return pTexture;
}

void diamonds::Game::userInputPhase()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                isRunning_ = false;
                break;
            case SDL_MOUSEBUTTONUP:
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    auto x = event.button.x / DIAMOND_WIDTH;;
                    auto y = event.button.y / DIAMOND_HEIGHT;

                    if(selectedX_ >= 0 && selectedY_ >= 0)
                    {
                        bool isClickedCellAdjacent = (abs(x - selectedX_) == 1 &&
                                                      y == selectedY_) ||
                                                      (abs(y - selectedY_) == 1 &&
                                                       x == selectedX_);
                        if(isClickedCellAdjacent)
                            std::swap(field_[y][x], field_[selectedY_][selectedX_]);
                        selectedX_ = -1;
                        selectedY_ = -1;
                    }
                    else
                    {
                        selectedX_ = x;
                        selectedY_ = y;
                    }
                }
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        isRunning_ = false;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

void diamonds::Game::updatePhase()
{
    findAndRemoveLines();
    checkHoles();
}

void diamonds::Game::drawPhase()
{
    SDL_SetRenderDrawColor(upRenderer_.get(), 0, 0, 0, 255);
    SDL_RenderClear(upRenderer_.get());
    //Drawing stuff
    for(auto y = 0; y < int(field_.size()); ++y)
    {
        for(auto x = 0; x < int(field_[y].size()); ++x)
        {
            if(field_[y][x] >= 0)
            {
                SDL_Rect src{0, 0, DIAMOND_WIDTH, DIAMOND_HEIGHT},
                        dest{x * DIAMOND_WIDTH, y * DIAMOND_HEIGHT,
                            DIAMOND_WIDTH, DIAMOND_HEIGHT};
                if(x != selectedX_ || y != selectedY_ || SDL_GetTicks() % 250 > 125)
                    SDL_RenderCopy(upRenderer_.get(), upSprite_.get(),
                               &src, &dest);

                SDL_Color color = diamondColorList_[field_[y][x]];
                SDL_SetRenderDrawColor(upRenderer_.get(), color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(upRenderer_.get(), &dest);
            }
        }
    }
    SDL_RenderPresent(upRenderer_.get());
    draw_ = false;
}

void diamonds::Game::findAndRemoveLines()
{
    int maxLineLenth{1}, maxLineHeight{1}, maxX{0}, maxY{0};

    //Horizontal lines

    for(auto y = 0u; y < FIELD_SIZE_Y; ++y)
    {
        int currentColor{field_[y][0]};
        int currentLenth{1};
        for(auto x = 1u; x < FIELD_SIZE_X; ++x)
        {
            if(field_[y][x] == currentColor)
            {
                ++currentLenth;
            }
            else
            {
                if(currentLenth > maxLineLenth && currentColor != -1)
                {
                    maxLineLenth = currentLenth;
                    maxX = x - currentLenth;
                    maxY = y;
                }
                currentLenth = 1;
                currentColor = field_[y][x];
            }
        }

        if(currentLenth > maxLineLenth && currentColor != -1)
        {
            maxLineLenth = currentLenth;
            maxX = FIELD_SIZE_X - currentLenth;
        }

        if(maxLineLenth  >= MIN_LINE_SIZE)
        {
            for(int x{maxX}; x < maxX + maxLineLenth; ++x)
                field_[y][x] = -1;
            maxLineLenth= 1;
            maxX = 0;
            std::ostringstream oss;
            oss << WINDOW_TITLE << ":" << ++score_;
            SDL_SetWindowTitle(upWindow_.get(), oss.str().c_str());
            Mix_PlayChannel(0, upSound_.get(), 0);
        }
     }

    //Vertical lines

    for(auto x = 0u; x < FIELD_SIZE_X; ++x)
    {
        int currentColor{field_[0][x]};
        int currentHeight{1};
        for(auto y = 1u; y < FIELD_SIZE_Y; ++y)
        {
            if(field_[y][x] == currentColor)
            {
                ++currentHeight;
            }
            else
            {
                if(currentHeight > maxLineHeight && currentColor != -1)
                {
                    maxLineHeight = currentHeight;
                    maxY = y - currentHeight;
                    maxX = x;
                }
                currentHeight = 1;
                currentColor = field_[y][x];
            }
        }

        if(currentHeight > maxLineHeight && currentColor != -1)
        {
            maxLineHeight = currentHeight;
            maxY = FIELD_SIZE_Y - currentHeight;
        }

        if(maxLineHeight >= MIN_LINE_SIZE)
        {
            for(int y{maxY}; y < maxY + maxLineHeight; ++y)
                field_[y][x] = -1;
            maxLineHeight= 1;
            maxY = 0;
            std::ostringstream oss;
            oss << WINDOW_TITLE << ":" << ++score_;
            SDL_SetWindowTitle(upWindow_.get(), oss.str().c_str());
            Mix_PlayChannel(0, upSound_.get(), 0);
        }
    }
}

void diamonds::Game::checkHoles()
{
    for(auto y = FIELD_SIZE_Y - 2; y >= 0; --y)
    {
        for(auto x = 0; x < FIELD_SIZE_X; ++x)
        {
            if(field_[y][x] != -1 && field_[y + 1][x] == -1)
            {
                for(auto yy = y; yy >= 0; --yy)
                    field_[yy + 1][x] = field_[yy][x];
                field_[0][x] = -1;
            }
        }
    }

    for(auto x = 0u; x < FIELD_SIZE_X; ++x)
        if(field_[0][x] == -1)
            field_[0][x] = random_.nextInt(0, diamondColorList_.size() - 1);
}

diamonds::SDL2Initializer::SDL2Initializer()
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error(std::string(SDL_GetError()));
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        throw std::runtime_error(std::string("Sound loading error: ") +
                                 std::string(Mix_GetError()));
    SDL_DisplayMode dm;
    if(SDL_GetDesktopDisplayMode(0, &dm))
        throw std::runtime_error(std::string(SDL_GetError()));
    screenWidth = dm.w;
    screenHeight = dm.h;
#ifdef DEBUG
    std::cout << screenWidth << " X " << screenHeight << std::endl;
#endif
}

diamonds::SDL2Initializer::~SDL2Initializer()
{
    Mix_CloseAudio();
    SDL_Quit();
}
