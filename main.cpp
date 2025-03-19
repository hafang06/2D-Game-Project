#include <SDL.h>
#include<SDL_image.h>
#include <bits/stdc++.h>

#define FOR(i, a, b) for(int i = (a); i <= (b); i++)
#define FORD(i, a, b) for(int i = (a); i >= (b); i--)
#define pb push_back
using namespace std;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT =720;
const int PLAYER_SIZE = 128;
const int FRAME_WIDTH = 128;
const int FRAME_HEIGHT = 128;
const int ANIMATION_FRAMES = 4;
const int ANIMATION_SPEED = 100;//milisecond;
const int FRAME_RATE = 60;

//animation
class Animation{
public:
    Animation() = default;
    Animation(SDL_Texture* tex, int frameW, int frameH, int totalFrames, int animSpeed, bool loop = 1): //texture, width, height, is in loop or not?
        texture(tex), frameWidth(frameW), frameHeight(frameH), totalFrames(totalFrames), speed(animSpeed), isLooping(loop){
            FOR(i, 0, totalFrames - 1){
                frames.pb({i * frameWidth, 0, frameWidth, frameHeight});
            }
        }
    void update(){
        if(!isPlaying) return;//not currntly playing
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastUpdate > speed) {
            currentFrame++;
            if (currentFrame >= totalFrames) {
                if (isLooping) currentFrame = 0;
                else {
                    currentFrame = totalFrames - 1;
                    isPlaying = false;
                }
            }
            lastUpdate = currentTime;
        }
    }

    void play(){
        isPlaying = 1;
    }
    void stop(){
        isPlaying = 0;
        reset();
    }
    void reset(){
        currentFrame = 0;
        lastUpdate = SDL_GetTicks();
    }
    SDL_Texture* texture = nullptr;
    vector<SDL_Rect> frames;
    int frameWidth = 0;
    int frameHeight = 0;
    int totalFrames = 0;
    int speed = 100; //ms
    int currentFrame = 0;
    bool isLooping = true;
    bool isPlaying = false;
    Uint32 lastUpdate = 0;
};

//for all entity
//position, velocity.
//manage different types of animation
class Entity{
public:
    Entity(int x, int y): position({x, y}){}

    virtual void update(float deltaTime) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;

    SDL_Point position{0, 0};
    SDL_Point velocity{0, 0};
    bool flipHorizontal = false;//flip model
protected:
    unordered_map<string, Animation> animations;
    string currentAnim = "idle";//default status
};
//
class Player: public Entity{
public:
    float baseSpeed = 10;
    float runSpeed = 13;
    bool isRunning = 0;//always walk till pressing shift

    Player(int x, int y, SDL_Texture* idleTex, SDL_Texture* runTex, SDL_Texture* walkTex): Entity(x, y){
        animations["idle"] = Animation(idleTex, FRAME_WIDTH, FRAME_HEIGHT, ANIMATION_FRAMES, ANIMATION_SPEED);
        animations["run"] = Animation(runTex, FRAME_WIDTH, FRAME_HEIGHT, 7, ANIMATION_SPEED);
        animations["walk"] = Animation(walkTex, FRAME_WIDTH, FRAME_HEIGHT, 8, ANIMATION_SPEED);
        animations["attack1"] = Animation(nullptr, FRAME_WIDTH, FRAME_HEIGHT, 5, 50, 0);
        animations["attack2"] = Animation(nullptr, FRAME_WIDTH, FRAME_HEIGHT, 4, 50, 0);
        animations["attack3"] = Animation(nullptr, FRAME_WIDTH, FRAME_HEIGHT, 4, 50, 0);
        animations["defend"] = Animation(nullptr, FRAME_WIDTH, FRAME_HEIGHT, 5, 50, 0);

    }
    void update(float deltaTime) override {


        //updtae logic
        float movespeed = isRunning ? runSpeed : baseSpeed;
        position.x += velocity.x * deltaTime * movespeed;
        position.y += velocity.y * deltaTime * movespeed;

        //update animation based on current status
        if (velocity.x != 0 || velocity.y != 0) {
            if(isRunning) setAnimation("run");
            else setAnimation("walk");
        }else{
            setAnimation("idle");
        }

        animations[currentAnim].update();
        position.x = max(0, min(position.x, SCREEN_WIDTH - 128));
        position.y = max(0, min(position.y, SCREEN_HEIGHT - 128));

    }
    void render(SDL_Renderer* renderer) override {
        Animation& anim = animations[currentAnim];
        SDL_Rect destRect = {position.x, position.y, anim.frameWidth, anim.frameHeight};
        //flip the texture
        SDL_RendererFlip flip = flipHorizontal ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, anim.texture, &anim.frames[anim.currentFrame], &destRect,0.0,nullptr,flip);
    }

    void setAnimation(const string& animName) {
        if (currentAnim != animName) {
            currentAnim = animName;
            animations[animName].reset();
            animations[animName].play();
        }
    }

    void run() {
        setAnimation("run");
    }

    void attack() {
        setAnimation("attack");
    }

    void defend(){
        setAnimation("defend");
    }

};

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
//SDL_Texture* gPlayerTexture = nullptr;
SDL_Texture* bgTextures[LAYER_COUNT];

Player* player = nullptr;

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
    SDL_Texture* idletex = loadTexture("assets/MAIN_knight/Spritesheet 128/Knight_1/Idle.png");
    SDL_Texture* walktex = loadTexture("assets/MAIN_knight/Spritesheet 128/Knight_1/Walk.png");
    SDL_Texture* attack1tex = loadTexture("assets/MAIN_knight/Spritesheet 128/Knight_1/Attack 1.png");
    SDL_Texture* runtex = loadTexture("assets/MAIN_knight/Spritesheet 128/Knight_1/Run.png");
    if(!idletex || !walktex || !attack1tex || !runtex){
        cerr << "Failed to load player texture\n";
        return 0;
    }
    player = new Player(playerX, playerY, idletex, runtex, walktex);

//    //set frame
//    for (int i = 0; i < ANIMATION_FRAMES; ++i) {
//        gSpriteClips[i] = { i * (FRAME_WIDTH), 0, FRAME_WIDTH, FRAME_HEIGHT };
//    }

    //load texture background
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
//        else if (e.type == SDL_KEYDOWN) {
//            switch (e.key.keysym.sym) {
//                case SDLK_ESCAPE:
//                    isRunning = false;
//                    break;
//                // Thêm các phím điều khiển khác ở đây
//            }
//        }
    }
    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    //handle running sstate
    bool isPressingShift = keystates[SDL_SCANCODE_LSHIFT];//only left shift
    player->isRunning = isPressingShift;
    float currentSpeed;
    if(player->isRunning) currentSpeed = player->runSpeed;
    else currentSpeed = player->baseSpeed;

    player->velocity.x = 0;
    player->velocity.y = 0;
    if (keystates[SDL_SCANCODE_A]) {
        player->velocity.x = -currentSpeed;
        player->flipHorizontal = true;
    }
    if (keystates[SDL_SCANCODE_D]) {
        player->velocity.x = currentSpeed;
        player->flipHorizontal = false;
    }
    if (keystates[SDL_SCANCODE_J]) {
        player->attack();
    }
//        cerr << playerX << " " << playerY << '\n';
}




SDL_Texture* loadTexture(const string &path) {
    SDL_Texture* newTexture = IMG_LoadTexture(gRenderer, path.c_str());
    if (!newTexture) {
        cerr << "Failed to load texture! Error: " << IMG_GetError() << endl;
    }
    return newTexture;
}

void renderBackground() {
    // Xóa màn hình
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    //render background
    SDL_RenderCopy(gRenderer, bgTextures[SKY], NULL, NULL);//sky
    SDL_RenderCopy(gRenderer, bgTextures[MIDDLE_LAYER], NULL, NULL);//middle layer
    SDL_RenderCopy(gRenderer, bgTextures[DOWN_LAYER], NULL, NULL);//down layer
    SDL_RenderCopy(gRenderer, bgTextures[LIGHT], NULL, NULL);//light
    SDL_RenderCopy(gRenderer, bgTextures[TOP_LAYER], NULL, NULL);//top layer

//    SDL_Rect* currentClips = &gSpriteClips[currentFrame];
//    SDL_Rect renderQuad = { playerX, playerY, FRAME_WIDTH, FRAME_HEIGHT };
//    SDL_RenderCopy(gRenderer, gPlayerTexture, currentClips, &renderQuad);

    // Cập nhật màn hình
//    SDL_RenderPresent(gRenderer);
}

void gameloop() {
    Uint32 lastTime = SDL_GetTicks();
    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0;
        lastTime = currentTime;

        handleInput();

        player->update(deltaTime);

        // Render
        SDL_RenderClear(gRenderer);
        renderBackground();

        player->render(gRenderer);

        SDL_RenderPresent(gRenderer);

        SDL_Delay(16);
    }
}

// Dọn dẹp tài nguyên
void close() {
//    SDL_DestroyTexture(gPlayerTexture);
//    gPlayerTexture = nullptr;
    delete player;
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
    gameloop();

    close();
    return 0;
}
