#include <SDL.h>
#include<SDL_image.h>
#include <bits/stdc++.h>

#define FOR(i, a, b) for(int i = (a); i <= (b); i++)
#define FORD(i, a, b) for(int i = (a); i >= (b); i--)

using namespace std;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT =720;
const int PLAYER_SIZE = 128;
const int FRAME_WIDTH = 128;
const int FRAME_HEIGHT = 128;
const int ANIMATION_FRAMES = 4;
const int ANIMATION_SPEED = 100;//milisecond;

//background
enum BackgroundLayers {
    SKY,
    LIGHT,
    MIDDLE_LAYER,
    DOWN_LAYER,
    TOP_LAYER,
    LAYER_COUNT
};

// Global Variables
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
SDL_Texture* gPlayerTexture = nullptr;
SDL_Texture* bgTextures[LAYER_COUNT];
bool isRunning = true;

// Vị trí người chơi
//int playerX = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
//int playerY = SCREEN_HEIGHT/2 - PLAYER_SIZE/2;
int playerX = 0;
int playerY = 445;
SDL_Rect gSpriteClips[ANIMATION_FRAMES];
int currentFrame = 0;
Uint32 lastFrameTime = 0;


SDL_Texture* loadTexture(const string &path);

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! Error: " << SDL_GetError() << endl;
        return false;
    }

    //init sdl_image support png
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        cerr << "SDL_image could not initialize! Error: " << IMG_GetError() << endl;
        return false;
    }

    gWindow = SDL_CreateWindow("Monster Slayer",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!gWindow) {
        cerr << "Window could not be created! Error: " << SDL_GetError() << endl;
        return 0;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!gRenderer) {
        cerr << "Renderer could not be created! Error: " << SDL_GetError() << endl;
        return 0;
    }
    gPlayerTexture = loadTexture("assets/MAIN_knight/Spritesheet 128/Knight_1/Idle.png");
    if (!gPlayerTexture) {
        return 0;
    }

    //set frame
    for (int i = 0; i < ANIMATION_FRAMES; ++i) {
        gSpriteClips[i] = { i * (FRAME_WIDTH), 0, FRAME_WIDTH, FRAME_HEIGHT };
    }

    //load texture
    bgTextures[SKY] = loadTexture("assets/Background/Sky.png");
    bgTextures[LIGHT] = loadTexture("assets/Background/Light.png");
    bgTextures[MIDDLE_LAYER] = loadTexture("assets/Background/MiddleLayer.png");
    bgTextures[DOWN_LAYER] = loadTexture("assets/Background/DownLayer.png");
    bgTextures[TOP_LAYER] = loadTexture("assets/Background/TopLayer.png");
    FOR(i, 0, LAYER_COUNT - 1){
        if(!bgTextures[i]){
            return 0;
        }
    }
    return 1;
}

// Xử lý input
void handleInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            isRunning = false;
        }
        else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    isRunning = false;
                    break;
                // Thêm các phím điều khiển khác ở đây
            }
        }
    }
    // Xử lý di chuyển liên tục
    const Uint8* keystates = SDL_GetKeyboardState(NULL);
//    if (keystates[SDL_SCANCODE_W]) playerY -= 5;
//    if (keystates[SDL_SCANCODE_S]) playerY += 5;
    if (keystates[SDL_SCANCODE_A]) playerX -= 5;
    if (keystates[SDL_SCANCODE_D]){
            playerX += 5;
    }
//        cerr << playerX << " " << playerY << '\n';
}


// Cập nhật game state
void update() {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime > lastFrameTime + ANIMATION_SPEED) {
        currentFrame = (currentFrame + 1) % ANIMATION_FRAMES;
        lastFrameTime = currentTime;
    }

    playerX = max(0, min(playerX, SCREEN_WIDTH - FRAME_WIDTH));
    playerY = max(0, min(playerY, SCREEN_HEIGHT - FRAME_HEIGHT));
}

SDL_Texture* loadTexture(const string &path) {
    SDL_Texture* newTexture = IMG_LoadTexture(gRenderer, path.c_str());
    if (!newTexture) {
        cerr << "Failed to load texture! Error: " << IMG_GetError() << endl;
    }
    return newTexture;
}

// Vẽ các đối tượng
void render() {
    // Xóa màn hình
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    //render background
    SDL_RenderCopy(gRenderer, bgTextures[SKY], NULL, NULL);//sky
    SDL_RenderCopy(gRenderer, bgTextures[LIGHT], NULL, NULL);//light
    SDL_RenderCopy(gRenderer, bgTextures[MIDDLE_LAYER], NULL, NULL);//middle layer
    SDL_RenderCopy(gRenderer, bgTextures[DOWN_LAYER], NULL, NULL);//down layer
    SDL_RenderCopy(gRenderer, bgTextures[TOP_LAYER], NULL, NULL);//top layer

    SDL_Rect* currentClips = &gSpriteClips[currentFrame];
    SDL_Rect renderQuad = { playerX, playerY, FRAME_WIDTH, FRAME_HEIGHT };
    SDL_RenderCopy(gRenderer, gPlayerTexture, currentClips, &renderQuad);

    // Cập nhật màn hình
    SDL_RenderPresent(gRenderer);
}

// Dọn dẹp tài nguyên
void close() {
    SDL_DestroyTexture(gPlayerTexture);
    gPlayerTexture = nullptr;

    FOR(i, 0, LAYER_COUNT - 1) {
        SDL_DestroyTexture(bgTextures[i]);
        bgTextures[i] = nullptr;
    }
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    IMG_Quit();
}

int main(int argc, char* args[]) {
    if (!init()) {
        cerr << "Failed to initialize!" << endl;
        return -1;
    }

    // Vòng lặp game chính
    while (isRunning) {
        handleInput();
        update();
        render();
        SDL_Delay(16); // ~60 FPS
    }

    close();
    return 0;
}
