        #include <SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include <bits/stdc++.h>
#define FOR(i, a, b) for(int i = (a); i <= (b); i++)
#define FORD(i, a, b) for(int i = (a); i >= (b); i--)
#define pb push_back
using namespace std;

//tourist debugger
void __print(int x) {cerr << x;}
void __print(long x) {cerr << x;}
void __print(long long x) {cerr << x;}
void __print(unsigned x) {cerr << x;}
void __print(unsigned long x) {cerr << x;}
void __print(unsigned long long x) {cerr << x;}
void __print(float x) {cerr << x;}
void __print(double x) {cerr << x;}
void __print(long double x) {cerr << x;}
void __print(char x) {cerr << '\'' << x << '\'';}
void __print(const char *x) {cerr << '\"' << x << '\"';}
void __print(const string &x) {cerr << '\"' << x << '\"';}
void __print(bool x) {cerr << (x ? "true" : "false");}

template<typename T, typename V>
void __print(const pair<T, V> &x) {cerr << '{'; __print(x.first); cerr << ','; __print(x.second); cerr << '}';}
template<typename T>
void __print(const T &x) {int f = 0; cerr << '{'; for (auto &i: x) cerr << (f++ ? "," : ""), __print(i); cerr << "}";}
void _print() {cerr << "]\n";}
template <typename T, typename... V>
void _print(T t, V... v) {__print(t); if (sizeof...(v)) cerr << ", "; _print(v...);}
#ifndef ONLINE_JUDGE
#define debug(x...) cerr << "[" << #x << "] = ["; _print(x)
#else
#define debug(x...)
#endif

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT =720;
const int PLAYER_SIZE = 128;
const int FRAME_WIDTH = 128;
const int FRAME_HEIGHT = 128;
const int ANIMATION_FRAMES = 4;
const int ANIMATION_SPEED = 100;//milisecond;
const int FRAME_RATE = 60;
const int BUTTON_WIDTH = 200;
const int BUTTON_HEIGHT = 50;

SDL_Texture* loadTexture(const string &path);

enum GameState {
    PLAYING,
    GAME_OVER
};
GameState gameState = PLAYING;

//animation
class Animation{
public:
    Animation() = default;
    Animation(SDL_Texture* tex, int frameW, int frameH, int totalFrames, int animSpeed, bool loop = 1): //texture, width, height, is in loop or not?
        texture(tex), frameWidth(frameW), frameHeight(frameH), totalFrames(totalFrames), speed(animSpeed), isLooping(loop){
            FOR(i, 0, totalFrames - 1){
                frames.pb({i * 128, 0, frameWidth, frameHeight});
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
    SDL_Rect hitbox;

    virtual void update(float deltaTime) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;
    void updateHitbox(){
        hitbox.x = position.x + 11;
        hitbox.y = position.y + 45;
        hitbox.w = 80;
        hitbox.h = 85;
    }

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

    float baseSpeed = 150;
    float runSpeed = 500;
    bool isRunning = 0;//always walk till pressing shift
    bool inOtherState = 0;
    string state = "idle";

    bool isJumping = 0;
    const float Gravity = 9800.0;
    const float Ground_Level = 445;
    const float Jumping_F = -8000;//lực nhảy

    bool jumpRequested = 0;
    bool canJump = 1;

//    const float JUMP_HEIGHT = 200.0;//FIXED JUMPING HEIGHT
//    const float JUMP_DURATION = 0.5; //t
    float jumpVelocity; //velocity.y

    //we have walk and run state so we should also have stamina:)
    float stamina = 100.0;
    float staminaDrainRate = 20.0;
    float staminaRegenRate = 10.0;

    bool isDead = 0;

    float attackTimer = 0.0f;
    float attackR = 100.0f;
    float attackCooldown = 1.0f;
    float attackDamage = 30.0f;
    float health = 100.0f;

    Player(int x, int y, SDL_Texture* idleTex, SDL_Texture* runTex, SDL_Texture* walkTex, SDL_Texture* attack1Tex, SDL_Texture* jumpTex): Entity(x, y){
        animations["idle"] = Animation(idleTex, 66, FRAME_HEIGHT, ANIMATION_FRAMES, ANIMATION_SPEED);
        animations["run"] = Animation(runTex, 98, FRAME_HEIGHT, 7, ANIMATION_SPEED);
        animations["walk"] = Animation(walkTex, 65, FRAME_HEIGHT, 8, ANIMATION_SPEED);
        animations["attack1"] = Animation(attack1Tex, 96, FRAME_HEIGHT, 5, 70);
        animations["jump"] = Animation(jumpTex, 88, FRAME_HEIGHT, 6, 60);
        animations["dead"] = Animation(loadTexture("assets/MAIN_knight/Spritesheet 128/Knight_1/Dead.png"), FRAME_WIDTH, FRAME_HEIGHT, 6, 100);
        animations["attack2"] = Animation(nullptr, FRAME_WIDTH, FRAME_HEIGHT, 4, 50, 0);
        animations["attack3"] = Animation(nullptr, FRAME_WIDTH, FRAME_HEIGHT, 4, 50, 0);
        animations["defend"] = Animation(nullptr, FRAME_WIDTH, FRAME_HEIGHT, 5, 50, 0);

    }
    void update(float deltaTime) override {
        //updtae logic
//        jumpVelocity = (2.0 * JUMP_HEIGHT)/JUMP_DURATION - (Gravity * JUMP_DURATION)/2;
//        float movespeed = isRunning ? runSpeed : baseSpeed;
//        cerr << Gravity * deltaTime << '\n';
//        position.y += velocity.y * deltaTime;
        if(isDead){
            animations[currentAnim].update();
            position.x = max(0, min(position.x, SCREEN_WIDTH - animations[currentAnim].frameWidth));
            position.y = max(0, min(position.y, 445));
            animations[currentAnim].isLooping = 0;
            return;
        }

        if(jumpRequested && canJump){
//            velocity.y = jumpVelocity;
            velocity.y = Jumping_F;
            canJump = 0;
        }
        velocity.y += Gravity * deltaTime;
        position.y = ceil(velocity.y * deltaTime + position.y);

        if (position.y >= Ground_Level) {
            position.y = Ground_Level;
            velocity.y = 0.0;
            canJump = 1;
            jumpRequested = 0;
        }

        //attack
        if(state == "attacking")


        //update animation based on current status
//        if(!isGrounded && ) setAnimation("jump");
        if (isRunning) {
            stamina = max(0.0f, stamina - staminaDrainRate * deltaTime);
            if (stamina <= 0){//not enough stamina to run
                isRunning = 0;
                velocity.x = baseSpeed;
                if(flipHorizontal) velocity.x *= -1;
            }
        } else stamina = min(100.0f, stamina + staminaRegenRate * deltaTime);

        position.x += velocity.x * deltaTime;
        if (velocity.x != 0) {
            if(isRunning) setAnimation("run");
            else setAnimation("walk");
        }else if(!inOtherState){
            setAnimation("idle");
        }

        if(currentAnim == "attack1"){
            attackTimer += deltaTime;
        }

        animations[currentAnim].update();

        position.x = max(0, min(position.x, SCREEN_WIDTH - animations[currentAnim].frameWidth));
        position.y = max(0, min(position.y, 445));
        updateHitbox();

    }
    void render(SDL_Renderer* renderer) override {
        Animation& anim = animations[currentAnim];
        //flip the texture
        SDL_RendererFlip flip = flipHorizontal ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_Rect destRect = {position.x, position.y, anim.frameWidth, anim.frameHeight};
        SDL_Point pivot = {0, 0};
//        if (flip) {
//            destRect.x -= anim.frameWidth;
//        }
        SDL_RenderCopyEx(renderer, anim.texture, &anim.frames[anim.currentFrame], &destRect,0.0,&pivot,flip);
    }

    void renderHealthPointBar(SDL_Renderer* renderer){
        SDL_Rect bgRect = {10, 10, 325, 20};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &bgRect);

        SDL_Rect healthRect = {12, 12, (int)(321 * (health/100.0f)), 16};
        SDL_SetRenderDrawColor(renderer, 199, 0, 60, 255);
        SDL_RenderFillRect(renderer, &healthRect);
    }

    void renderStaminaBar(SDL_Renderer* renderer) {
        SDL_Rect bgRect = {10, 40, 200, 20};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &bgRect);

        SDL_Rect staminaRect = {12, 42, (int)(196 * (stamina/100.0f)), 16};
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
        SDL_RenderFillRect(renderer, &staminaRect);
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

    void jump(){
        inOtherState = 1;
        setAnimation("jump");
    }

    void attack() {
        inOtherState = 1;
        setAnimation("attack1");
    }

    void defend(){
        inOtherState = 1;
        setAnimation("defend");
    }

    void dead(){
        isDead = 1;
        setAnimation("dead");
    }

    void takeDamage(float damage) {
        health -= damage;
        if(health <= 0){
            health = 0;
            gameState = GAME_OVER;
            dead();
        }
    }

    void reset(){
        isDead = 0;
        health = 100.0f;
    }



};

enum class EnemyState {
    WANDERING,
    CHASING,
    ATTACKING,
    DEAD
};

class Enemy : public Entity {
public:
    enum class Type { MINOTAUR, SKELETON, WEREWOLF, BLUESLIME, GREENSLIME};

    bool isDead = 0;

    Enemy(int x, int y, Type type, Player* target) : Entity(x, y), enemyType(type), player(target) {
        // Khởi tạo animation theo loại quái
        detectionR = 250.0;
        attackR = 80.0;
        wanderSpeed = 100.0;
        chaseSpeed = 200.0;
        state = EnemyState::WANDERING;
        stateTimer = 0.0;
        switch(type) {
            case Type::MINOTAUR:
                animations["idle"] = Animation(loadTexture("assets/CREP_Minotaur/Minotaur_1/Idle.png"), 115, 128, 10, 100);
                animations["attack"] = Animation(loadTexture("assets/CREP_Minotaur/Minotaur_1/Attack.png"), 128, 128, 5, 200);
                animations["walk"] = Animation(loadTexture("assets/CREP_Minotaur/Minotaur_1/Walk.png"), 128, 128, 12, 100);
                animations["dead"] = Animation(loadTexture("assets/CREP_Minotaur/Minotaur_1/Dead.png"), 128, 128, 5, 100);
                animations["hurt"] = Animation(loadTexture("assets/CREP_Minotaur/Minotaur_1/Hurt.png"), 128, 128, 3, 100);
                break;
            case Type::SKELETON:
                animations["idle"] = Animation(loadTexture("assets/CREP_Skeleton/Skeleton_Spearman/Idle.png"), 128, 128, 7, 150);
                animations["attack"] = Animation(loadTexture("assets/CREP_Skeleton/Skeleton_Spearman/Attack.png"), 128, 128, 4, 200);
                animations["walk"] = Animation(loadTexture("assets/CREP_Skeleton/Skeleton_Spearman/Walk.png"), 128, 128, 7, 100);
                animations["dead"] = Animation(loadTexture("assets/CREP_Skeleton/Skeleton_Spearman/Dead.png"), 128, 128, 5, 100);
                animations["hurt"] = Animation(loadTexture("assets/CREP_Skeleton/Skeleton_Spearman/Hurt.png"), 128, 128, 3, 100);
                break;
            case Type::WEREWOLF:
                animations["idle"] = Animation(loadTexture("assets/CREP_Werewolf/White_Werewolf/Idle.png"), 128, 128, 8, 100);
                animations["attack"] = Animation(loadTexture("assets/CREP_Werewolf/White_Werewolf/Attack.png"), 128, 128, 6, 200);
                animations["walk"] = Animation(loadTexture("assets/CREP_Werewolf/White_Werewolf/Walk.png"), 128, 128, 11, 60);
//                animations["run"] = Animation(loadTexture("assets/CREP_Werewolf/White_Werewolf/Run.png"), 128, 128, 9, 100);
//                animations["runAttack"] = Animation(loadTexture("assets/CREP_Werewolf/White_Werewolf/Run+Attack.png"), 128, 128, 7, 100);
                animations["dead"] = Animation(loadTexture("assets/CREP_Werewolf/White_Werewolf/Dead.png"), 128, 128, 2, 100);
                animations["hurt"] = Animation(loadTexture("assets/CREP_Werewolf/White_Werewolf/Hurt.png"), 128, 128, 2, 100);
                break;
            case Type::BLUESLIME:
                animations["idle"] = Animation(loadTexture("assets/CREP_Slime/Blue_Slime/Idle.png"), 128, 128, 8, 100);
                animations["attack"] = Animation(loadTexture("assets/CREP_Slime/Blue_Slime/Attack.png"), 128, 128, 4, 200);
                animations["walk"] = Animation(loadTexture("assets/CREP_Slime/Blue_Slime/Walk.png"), 128, 128, 8, 100);
//                animations["run"] = Animation(loadTexture("assets/CREP_Slime/Blue_Slime/Run.png"), 128, 128, 7, 100);
                animations["dead"] = Animation(loadTexture("assets/CREP_Slime/Blue_Slime/Dead.png"), 128, 128, 3, 100);
                animations["hurt"] = Animation(loadTexture("assets/CREP_Slime/Blue_Slime/Hurt.png"), 128, 128, 6, 100);
                break;
            case Type::GREENSLIME:
                animations["idle"] = Animation(loadTexture("assets/CREP_Slime/Green_Slime/Idle.png"), 128, 128, 8, 100);
                animations["attack"] = Animation(loadTexture("assets/CREP_Slime/Green_Slime/Attack.png"), 128, 128, 4, 200);
                animations["walk"] = Animation(loadTexture("assets/CREP_Slime/Green_Slime/Walk.png"), 128, 128, 8, 100);
//                animations["run"] = Animation(loadTexture("assets/CREP_Slime/Green_Slime/Run.png"), 128, 128, 7, 100);
                animations["dead"] = Animation(loadTexture("assets/CREP_Slime/Green_Slime/Dead.png"), 128, 128, 3, 100);
                animations["hurt"] = Animation(loadTexture("assets/CREP_Slime/Green_Slime/Hurt.png"), 128, 128, 6, 100);
                break;
        }
    }
    void setAnimation(const string& animName) {
        if (currentAnim != animName) {
            currentAnim = animName;
            animations[animName].reset();
            animations[animName].play();
        }
    }
    void update(float deltaTime) override {
        //logic
        //calc distance
        float difX = player->position.x - position.x;
        float difY = player->position.y - position.y;
        distanceToPlayer = sqrt(difX * difX + difY * difY);
        updateTakingDamage(player->attackDamage);

        switch(state){
            case EnemyState::WANDERING:
                setAnimation("idle");
                updateWandering(deltaTime);
                break;

            case EnemyState::CHASING:
                setAnimation("walk");
                updateChasing(deltaTime);
                break;

            case EnemyState::ATTACKING:
                setAnimation("attack");;
                updateAttacking(deltaTime);
                break;
            case EnemyState::DEAD:
                setAnimation("dead");
                updateDead(deltaTime);
        }
        animations[currentAnim].update();
        position.x += velocity.x * deltaTime;
        position.y += velocity.y * deltaTime;
        updateHitbox();
    }

    void render(SDL_Renderer* renderer) override {
        if(isDead) return;
        Animation& anim = animations[currentAnim];
        //flip the texture
        SDL_RendererFlip flip = flipHorizontal ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_Rect destRect = {position.x, position.y, anim.frameWidth, anim.frameHeight};
        SDL_Point pivot = {0, 0};
//        if (flip) {
//            destRect.x -= anim.frameWidth;
//        }
        SDL_RenderCopyEx(renderer, anim.texture, &anim.frames[anim.currentFrame], &destRect,0.0,&pivot,flip);
    }
    void takeDamage(float damage) {
        health -= damage;
        if(health <= 0){
            health = 0;
            state = EnemyState::DEAD;
            isDead = 1;
        }
    }
private:
    void updateWandering(float deltaTime) {
        if(distanceToPlayer < detectionR) {
            state = EnemyState::CHASING;
            return;
        }

        //change direction after cert time
        stateTimer -= deltaTime;
        if(stateTimer <= 0.0f) {
//            float angle = static_cast<float>(rand() % 360) * (3.14159 / 180.0);
//            velocity.x = 1.0 * cos(angle) * wanderSpeed;
//            velocity.y = 1.0 * sin(angle) * wanderSpeed;
            flipHorizontal = 1 - flipHorizontal;
            //new wander time(random)
            stateTimer = 2.0 + static_cast<float>(rand() % 4);
        }
    }

    void updateChasing(float deltaTime) {
        if(distanceToPlayer > detectionR * 1.2) {
            state = EnemyState::WANDERING;
            velocity = {0, 0};
            return;
        }

        float difX = player->position.x - position.x;
        float difY = player->position.y - position.y;
        float len = sqrt(difX * difX + difY * difY);

        if(len > 0) {
            velocity.x = (difX / len) * chaseSpeed;
            velocity.y = (difY / len) * chaseSpeed;
        }
        velocity.y = max(velocity.y, 0);//fixing flying object=))
        flipHorizontal = (velocity.x < 0);

        if(distanceToPlayer < attackR) {
            state = EnemyState::ATTACKING;
            velocity = {0, 0};
        }
    }

    void updateAttacking(float deltaTime) {
        attackTimer += deltaTime;
        if(attackTimer >= attackCooldown) {
            if(distanceToPlayer < attackR) {
                player->takeDamage(attackDamage);
            }
            attackTimer = 0.0;
        }

        if(distanceToPlayer > attackR) {
            state = EnemyState::CHASING;
        }
    }

    void updateDead(float deltaTime){
        animations[currentAnim].isLooping = 0;
        animations[currentAnim].isPlaying = 0;
    }

    void updateTakingDamage(float damage){
        if(distanceToPlayer < (player->attackR)
                        &&
        (player->attackTimer) >= (player->attackCooldown)) takeDamage(damage);
    }

    Type enemyType;

    EnemyState state;
    Player* player;
    float detectionR;
    float attackR;
    float wanderSpeed;
    float chaseSpeed;
    float stateTimer;
    float attackTimer = 0.0f;
    float attackCooldown = 1.5f;
    float attackDamage = 10.0f;
    float distanceToPlayer = 0.0f;
    float health = 100.0;
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
TTF_Font* gFont = nullptr;

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
vector<Enemy> enemies;

class Button {
public:
    SDL_Rect rect;
    string text;

    Button(int x, int y, string s) {
        rect = {x, y, BUTTON_WIDTH, BUTTON_HEIGHT};
        text = s;
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_Color textColor = {255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, text.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect = {
            rect.x + (BUTTON_WIDTH - textSurface->w)/2,
            rect.y + (BUTTON_HEIGHT - textSurface->h)/2,
            textSurface->w,
            textSurface->h
        };

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    bool isClicked(int mouseX, int mouseY) {
        return (mouseX >= rect.x &&
                mouseX <= rect.x + BUTTON_WIDTH &&
                mouseY >= rect.y &&
                mouseY <= rect.y + BUTTON_HEIGHT);
    }
};

Button playAgainButton(300, 300, "Play Again");
Button quitButton(300, 400, "Quit");


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
    SDL_Texture* jumptex = loadTexture("assets/MAIN_knight/Spritesheet 128/Knight_1/Jump.png");
    if(!idletex || !walktex || !attack1tex || !runtex){
        cerr << "Failed to load player texture\n";
        return 0;
    }
    player = new Player(playerX, playerY, idletex, runtex, walktex, attack1tex, jumptex);

    if(TTF_Init() == -1) {
        cerr << "SDL_ttf could not initialize! Error: " << TTF_GetError() << endl;
        return false;
    }

    gFont = TTF_OpenFont("Arial.ttf", 28);
//    gFont = TTF_OpenFont();

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

SDL_Texture* loadTexture(const string &path) {
    SDL_Texture* newTexture = IMG_LoadTexture(gRenderer, path.c_str());
    if (!newTexture) {
        cerr << "Failed to load texture! Error: " << IMG_GetError() << endl;
    }
    return newTexture;
}

void renderBackground() {
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

//render hitbox  for debug purpose
void renderDebug(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &(player->hitbox));

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    for (auto& enemy : enemies) {
        SDL_RenderFillRect(renderer, &(enemy.hitbox));
    }
}

void renderGameOverMenu() {

    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(gRenderer, &overlay);




    SDL_Color textColor = {255, 0, 0};
    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, "Game Over!", textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
    SDL_Rect textRect = {250, 200, textSurface->w, textSurface->h};
    SDL_RenderCopy(gRenderer, textTexture, NULL, &textRect);



    playAgainButton.render(gRenderer);
    quitButton.render(gRenderer);



    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void handleGameOverInput(SDL_Event& e) {
    if(e.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        if(playAgainButton.isClicked(mouseX, mouseY)) {
            gameState = PLAYING;
            player->reset();
        }
        else if(quitButton.isClicked(mouseX, mouseY)) {
            exit(0);
        }
    }
}


// Xử lý input
void handleInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            isRunning = false;
        }
        if(gameState == GAME_OVER) {
            handleGameOverInput(e);
        }
//        else if (e.type == SDL_KEYDOWN) {
//            switch (e.key.keysym.scancode == SDL_SCANCODE_SPACE && (player ->canJump)) {

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

    player->inOtherState = 0;
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
//    if (keystates[SDL_SCANCODE_SPACE] && (player->canJump)) {
//        player->jump();
//        (player->jumpRequested) = 1;
////        player->velocity.y = -5;
//    }
    //debug gameover menu
    if (keystates[SDL_SCANCODE_SPACE]) {
        player->takeDamage(10.0f);
//        (player->jumpRequested) = 1;
//        player->velocity.y = -5;
    }

//        cerr << playerX << " " << playerY << '\n';
}




void gameloop() {
    Uint32 lastTime = SDL_GetTicks();
    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0;
        lastTime = currentTime;

        handleInput();

        //update all entity
        player->update(deltaTime);
        for (auto &enemy : enemies) {
            enemy.update(deltaTime);
        }

        if(player->attackTimer >= player->attackCooldown){
            player->attackTimer = 0.0f;
        }

//        checkAllCollision();


        // Render
        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
        SDL_RenderClear(gRenderer);
        renderBackground();
//        renderDebug(gRenderer);
        player->renderStaminaBar(gRenderer);
        player->renderHealthPointBar(gRenderer);
        player->render(gRenderer);
        for (auto& enemy : enemies) {
            enemy.render(gRenderer);
        }

        if(gameState == GAME_OVER) renderGameOverMenu();
        SDL_RenderPresent(gRenderer);

        cerr << (player->position.x) << " " << (player->position.y) << " " << (player->velocity.x) << " " << (player->stamina) <<  '\n';
//        cerr << (player->position.x) << " " << (player->position.y) << '\n';
//        debug(player->velocity);
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
    TTF_CloseFont(gFont);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    TTF_Quit();
    SDL_Quit();
    IMG_Quit();
}

int main(int argc, char* args[]) {
    if (!init()) {
        cerr << "Failed to initialize!" << endl;
        return -1;
    }
    enemies.pb({473, 445, Enemy::Type::WEREWOLF, player});
    // Vòng lặp game chính
    gameloop();

    close();
    return 0;
}
