#include <SDL.h>
#include <iostream>

// Các hằng số
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_SIZE = 40;

// Biến toàn cục
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
bool isRunning = true;

// Vị trí người chơi
int playerX = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
int playerY = SCREEN_HEIGHT/2 - PLAYER_SIZE/2;

// Khởi tạo SDL và tạo cửa sổ
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! Error: " << SDL_GetError() << std::endl;
        return false;
    }

    gWindow = SDL_CreateWindow("Monster Slayer",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!gWindow) {
        std::cerr << "Window could not be created! Error: " << SDL_GetError() << std::endl;
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!gRenderer) {
        std::cerr << "Renderer could not be created! Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

// Xử lý input
void handleInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e) {
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
    if (keystates[SDL_SCANCODE_W]) playerY -= 5;
    if (keystates[SDL_SCANCODE_S]) playerY += 5;
    if (keystates[SDL_SCANCODE_A]) playerX -= 5;
    if (keystates[SDL_SCANCODE_D]) playerX += 5;
}

// Cập nhật game state
void update() {
    // Giới hạn di chuyển trong màn hình
    if (playerX < 0) playerX = 0;
    if (playerX > SCREEN_WIDTH - PLAYER_SIZE) playerX = SCREEN_WIDTH - PLAYER_SIZE;
    if (playerY < 0) playerY = 0;
    if (playerY > SCREEN_HEIGHT - PLAYER_SIZE) playerY = SCREEN_HEIGHT - PLAYER_SIZE;
}

// Vẽ các đối tượng
void render() {
    // Xóa màn hình
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    // Vẽ player (tạm thời dùng hình vuông)
    SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255);
    SDL_Rect playerRect = { playerX, playerY, PLAYER_SIZE, PLAYER_SIZE };
    SDL_RenderFillRect(gRenderer, &playerRect);

    // Cập nhật màn hình
    SDL_RenderPresent(gRenderer);
}

// Dọn dẹp tài nguyên
void close() {
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

int main(int argc, char* args[]) {
    if (!init()) {
        std::cerr << "Failed to initialize!" << std::endl;
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
