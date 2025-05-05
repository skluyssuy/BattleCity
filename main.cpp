#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include <algorithm>
#include <SDL_ttf.h>
#include <fstream>

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
const int TILE_SIZEm = 20;
enum GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    VICTORY,
    QUIT
};
TTF_Font* font1 = nullptr;
TTF_Font* font2 = nullptr;

void renderMenu(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    if (font2) {
        SDL_Color brick = {255, 80, 80};

        // "Battle City" text
        SDL_Surface* battleCitySurface = TTF_RenderText_Blended(font2, "Battle City", brick);
        SDL_Texture* battleCityText = SDL_CreateTextureFromSurface(renderer, battleCitySurface);
        SDL_Rect battleCityTextRect = {
            SCREEN_WIDTH / 2 - battleCitySurface->w / 2,  // Center the text horizontally and place it at the top
            battleCitySurface->h,
            battleCitySurface->w,
            battleCitySurface->h
        };
        SDL_RenderCopy(renderer, battleCityText, NULL, &battleCityTextRect);
        SDL_FreeSurface(battleCitySurface);
        SDL_DestroyTexture(battleCityText);
    }


    // Draw "Start" button
    SDL_Rect startButton = {SCREEN_WIDTH / 2 - 100, 200, 200, 60};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &startButton);

    // Draw "Quit" button
    SDL_Rect quitButton = {SCREEN_WIDTH / 2 - 100, 300, 200, 60};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &quitButton);

    // Draw text using Blended (anti-aliased, high quality)
    if (font1) {
        SDL_Color white = {255, 255, 255};

        // Start button text
        SDL_Surface* startSurface = TTF_RenderText_Blended(font1, "1 Player", white);
        SDL_Texture* startText = SDL_CreateTextureFromSurface(renderer, startSurface);
        SDL_Rect startTextRect = {
            startButton.x + (startButton.w - startSurface->w) / 2,
            startButton.y + (startButton.h - startSurface->h) / 2,
            startSurface->w,
            startSurface->h
        };
        SDL_RenderCopy(renderer, startText, NULL, &startTextRect);
        SDL_FreeSurface(startSurface);
        SDL_DestroyTexture(startText);

        // Quit button text
        SDL_Surface* quitSurface = TTF_RenderText_Blended(font1, "Quit", white);
        SDL_Texture* quitText = SDL_CreateTextureFromSurface(renderer, quitSurface);
        SDL_Rect quitTextRect = {
            quitButton.x + (quitButton.w - quitSurface->w) / 2,
            quitButton.y + (quitButton.h - quitSurface->h) / 2,
            quitSurface->w,
            quitSurface->h
        };
        SDL_RenderCopy(renderer, quitText, NULL, &quitTextRect);
        SDL_FreeSurface(quitSurface);
        SDL_DestroyTexture(quitText);
    }

    SDL_RenderPresent(renderer);
}

void handleMenuEvents(GameState &state, bool &running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
            state = QUIT;
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x;
            int my = event.button.y;
            // Check if click is in "Start" area
            if (mx >= SCREEN_WIDTH / 2 - 100 && mx <= SCREEN_WIDTH / 2 + 100) {
                if (my >= 200 && my <= 260) {
                    state = PLAYING;
                }
                else if (my >= 300 && my <= 360) {
                    state = QUIT;
                }
            }
        }
    }
}

SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (!newTexture) {
        cerr << "Failed to load image " << path << "! SDL_image Error: " << IMG_GetError() << endl;
    }
    return newTexture;
}

class Wall
{
public:
    int x, y;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;

    Wall(int startX, int startY , SDL_Texture* tex)
    {
        x = startX;
        y = startY;
        active = true;
        rect = {x,y, TILE_SIZEm, TILE_SIZEm};
        texture = tex;
    }
    void render(SDL_Renderer* renderer)
    {
        if (active && texture)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};

class Stone {
public:
    int x, y;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;

    Stone(int startX, int startY , SDL_Texture* tex)
    {
        x = startX;
        y = startY;
        active = true;
        rect = {x,y, TILE_SIZEm, TILE_SIZEm};
        texture = tex;
    }
    void render(SDL_Renderer* renderer)
    {
        if (active && texture)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};
class Bush{
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;

    Bush(int startX, int startY , SDL_Texture* tex)
    {
        x = startX;
        y = startY;
        rect = {x,y, TILE_SIZEm, TILE_SIZEm};
        texture = tex;
    }
    void render(SDL_Renderer* renderer)
    {
        if (texture)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};

class Ice{
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;

    Ice(int startX, int startY , SDL_Texture* tex)
    {
        x = startX;
        y = startY;
        rect = {x,y, TILE_SIZEm, TILE_SIZEm};
        texture = tex;
    }
    void render(SDL_Renderer* renderer)
    {
        if (texture)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};
class Water{
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;

    Water(int startX, int startY , SDL_Texture* tex)
    {
        x = startX;
        y = startY;
        rect = {x,y, TILE_SIZEm, TILE_SIZEm};
        texture = tex;
    }
    void render(SDL_Renderer* renderer)
    {
        if (texture)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};
class Base{
public:
    int x,y;
    SDL_Rect rect;
    SDL_Texture *texture;
    int hp;
    bool active;
    Base(){};
    Base(int startX, int startY, SDL_Texture* tex)
    {
        x = startX;
        y = startY;
        active = true;
        texture = tex;
        rect = {x,y,TILE_SIZE,TILE_SIZE};
        hp = 2;
    }
    void render(SDL_Renderer *renderer)
    {
        if (texture)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};

class Bullet
{
public:
    int x , y ;
    int dx , dy;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;

    Bullet(int startX, int startY, int dirX, int dirY, SDL_Texture* tex)
    {
        x = startX;
        y = startY;
        dx = dirX;
        dy = dirY;
        active = true;
        texture = tex;
        rect = {x, y, 10, 10};
    }

    void move()
    {
        x += dx;
        y += dy;
        rect.x = x;
        rect.y = y;
        if( x < TILE_SIZE || x > SCREEN_WIDTH - TILE_SIZE||
            y < TILE_SIZE || y > SCREEN_HEIGHT - TILE_SIZE)
        {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
    if (active && texture)
        {
        double angle = 0.0;
        if (dx == 0 && dy < 0) angle = 0.0;         // UP
        else if (dx == 0 && dy > 0) angle = 180.0;  // DOWN
        else if (dx < 0 && dy == 0) angle = 270.0;  // LEFT
        else if (dx > 0 && dy == 0) angle = 90.0;   // RIGHT

        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
        }
    }
};

class EnemyTank
{
public:
    int x, y, dirX, dirY, moveDelay, shootDelay;
    SDL_Rect rect;
    bool active;
    vector<Bullet> bullets;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture;
    Mix_Chunk* shootSound;

    EnemyTank(int startX, int startY, SDL_Texture* tex, SDL_Texture* bulletTex, const vector<Wall>& walls , Mix_Chunk* Sound)
    {
        x = startX;
        y = startY;
        dirX = 0;
        dirY = 1;
        moveDelay = 15;
        shootDelay = 5;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        active = true;
        texture = tex;
        bulletTexture = bulletTex;
        shootSound = Sound;
        for (const auto& wall : walls)
        {
            if (wall.active && SDL_HasIntersection(&rect, &wall.rect))
            {
                active = false;
                break;
            }
        }
    }

    void move (const vector<Wall> &walls , const vector<Stone> &stones , const vector<Water> &waters)
    {
        if(--moveDelay > 0) return;
        moveDelay = 15 ;
        int r =rand() % 4;
        if ( r == 0 )//Up
        {
            this -> dirX = 0;
            this -> dirY = -10;
        }
        if ( r == 1 )//Down
        {
            this -> dirX = 0;
            this -> dirY = 10;
        }
        if ( r == 2 )//left
        {
            this -> dirX = -10;
            this -> dirY = 0;
        }
        if ( r == 3 )//Right
        {
            this -> dirX = 10;
            this -> dirY = 0;
        }
        int newX = x + this->dirX;
        int newY = y + this->dirY;

        SDL_Rect newRect = { newX , newY , TILE_SIZE , TILE_SIZE};
        for (const auto&wall : walls)
        {
            if(wall.active && SDL_HasIntersection(&newRect , &wall.rect))
            {
                return;
            }
        }

        for (const auto&stone : stones)
        {
            if(stone.active && SDL_HasIntersection(&newRect , &stone.rect))
            {
                return;
            }
        }
        for (const auto&water: waters)
        {
            if(SDL_HasIntersection(&newRect , &water.rect))
            {
                return;
            }
        }
        if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE *2 &&
            newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE *2)
        {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }

    void shoot() {
        if (--shootDelay > 0) return;
        shootDelay = 20;
        if (shootSound) Mix_PlayChannel(-1, shootSound, 0);
        bullets.emplace_back(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture);
    }

    void updateBullets()
    {
        for ( auto &bullet : bullets)
        {
            bullet.move();
        }
        bullets.erase (remove_if ( bullets.begin() , bullets.end() ,
                                  [] (Bullet &b) { return !b.active;}), bullets.end());
    }

    void render(SDL_Renderer* renderer)
    {
        if (active && texture)
        {
            double angle = 0.0;
            if (dirX == 0 && dirY < 0) angle = 0.0;         // UP
            else if (dirX == 0 && dirY > 0) angle = 180.0;   // DOWN
            else if (dirX < 0 && dirY == 0) angle = 270.0;   // LEFT
            else if (dirX > 0 && dirY == 0) angle = 90.0;    // RIGHT

            SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
        }

        for (auto& bullet : bullets) bullet.render(renderer);
    }
};

class PlayerTank
{
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    SDL_Texture* texture;
    SDL_Texture* bulletTexture;
    vector<Bullet> bullets;
    Mix_Chunk* shootSound;

    PlayerTank(int startX, int startY, SDL_Renderer* renderer, SDL_Texture* tex, SDL_Texture* bulletTex , Mix_Chunk* Sound )
    {
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = -1;
        texture = tex;
        bulletTexture = bulletTex;
        shootSound = Sound;
    }

    void shoot()
    {
        if (shootSound) Mix_PlayChannel(-1, shootSound, 0);
        bullets.emplace_back(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, bulletTexture);

    }

    void updateBullets()
    {
        for ( auto &bullet : bullets)
        {
            bullet.move();
        }
        bullets.erase (remove_if ( bullets.begin() , bullets.end() ,
                                  [] (Bullet &b) { return !b.active;}), bullets.end());
    }

    void move( int dx , int dy , const vector<Wall>& walls , const vector<Stone> &stones , const vector<Water> &waters)
    {
        int newX = x + dx;
        int newY = y + dy;
        this -> dirX = dx;
        this -> dirY = dy;

        SDL_Rect newRect = { newX , newY , TILE_SIZE , TILE_SIZE };

        for ( auto i = 0 ; i < walls.size() ; i++)
        {
            if ( walls[i].active && SDL_HasIntersection ( &newRect , &walls[i].rect) )
            {
                return;
            }
        }
        for (const auto&stone : stones)
        {
            if(stone.active && SDL_HasIntersection(&newRect , &stone.rect))
            {
                return;
            }
        }
        for (const auto&water: waters)
        {
            if(SDL_HasIntersection(&newRect , &water.rect))
            {
                return;
            }
        }
        if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE *2 &&
            newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE *2)
        {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }

    void render(SDL_Renderer* renderer)
    {
    if (texture) {
        double angle = 0.0;
        if (dirX == 0 && dirY < 0) angle = 0.0;         // UP
        else if (dirX == 0 && dirY > 0) angle = 180.0;   // DOWN
        else if (dirX < 0 && dirY == 0) angle = 270.0;   // LEFT
        else if (dirX > 0 && dirY == 0) angle = 90.0;    // RIGHT

        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    }

    for (auto& bullet : bullets) bullet.render(renderer);
    }
};

class Game
{
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    SDL_Texture* backgroundTexture;
    SDL_Texture* wallTexture;
    SDL_Texture* stoneTexture;
    SDL_Texture* bushTexture;
    SDL_Texture* iceTexture;
    SDL_Texture* waterTexture;
    SDL_Texture* baseTexture;
    SDL_Texture* bulletTexture;
    SDL_Texture* playerTexture;
    SDL_Texture* enemyTexture;
    Mix_Chunk* shootSound;
    Mix_Chunk* explosionSound;
    Mix_Chunk* gameOverSound;
    Mix_Chunk* victorySound;
    GameState state = MENU;

    vector<Wall> walls;
    vector<Stone> stones;
    vector<Bush> bushs;
    vector<Ice> ices;
    vector<Water> waters;
    Base base;

    PlayerTank* player;

    int enemyNumber = 5;
    int remainingEnemies = enemyNumber;// Số enemy chưa xuất hiện
    const int maxActiveEnemies = 3;
    vector<EnemyTank> enemies;

    Game()
    {
        running = true;
        if (SDL_Init (SDL_INIT_VIDEO) < 0) {
            cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
            running = false;
        }

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
            running = false;
        }

        shootSound = Mix_LoadWAV("sound/shoot.wav");
        explosionSound = Mix_LoadWAV("sound/explosion.wav");
        gameOverSound = Mix_LoadWAV("sound/gameover.wav");
        victorySound = Mix_LoadWAV("sound/victory.wav");

        if (!shootSound || !explosionSound || !gameOverSound) {
            cerr << "Failed to load sound effects! SDL_mixer Error: " << Mix_GetError() << endl;
        }

        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

        if (!window) {
            cerr << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
            running = false;
        }

        renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer)
        {
            cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
            running = false;
        }

        backgroundTexture = loadTexture("image/background.jpg", renderer);
        wallTexture = loadTexture("image/wall.png", renderer);
        stoneTexture = loadTexture("image/stone.png", renderer);
        bushTexture = loadTexture("image/bush.png", renderer);
        iceTexture = loadTexture("image/ice.png", renderer);
        waterTexture = loadTexture("image/water.png", renderer);
        bulletTexture = loadTexture("image/bullet.png", renderer);
        playerTexture = loadTexture("image/player.png", renderer);
        enemyTexture = loadTexture("image/enemy.png", renderer);

        generateWalls();
        player = new PlayerTank( (19 * TILE_SIZE)/2, 13 * TILE_SIZE, renderer, playerTexture, bulletTexture , shootSound);
        spawnEnemies();
    }

    void render() {
        if (backgroundTexture) SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        for (auto& wall : walls) wall.render(renderer);
        for (auto& stone : stones) stone.render(renderer);
        for (auto& bush : bushs) bush.render(renderer);
        for (auto& ice : ices) ice.render(renderer);
        for (auto& water : waters) water.render(renderer);
        player->render(renderer);
        for (auto& enemy : enemies) enemy.render(renderer);

        SDL_RenderPresent(renderer);
    }

    void run() {
        while (running) {
            handleEvents();
            update();
            render();
            SDL_Delay(16);
        }
    }

    ~Game() {
        delete player;
        SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(wallTexture);
        SDL_DestroyTexture(bulletTexture);
        SDL_DestroyTexture(playerTexture);
        SDL_DestroyTexture(enemyTexture);
        Mix_FreeChunk(shootSound);
        Mix_FreeChunk(explosionSound);
        Mix_FreeChunk(gameOverSound);
        Mix_FreeChunk(victorySound);
        Mix_CloseAudio();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
    }

    generateWalls() {
        walls.clear();
        bushs.clear();
        waters.clear();
        ices.clear();
        stones.clear();
        vector <vector <char>> Map(28,vector<char>(27,'.'));
        string timkiem = "map.txt";
        ifstream file(timkiem);
        for(int i= 1; i<=27; i++)
        {
            for (int j = 1; j<=36; j++){
                file >> Map[i][j];
            }
        }
        file.close();
        for (int i = 1; i<=27; i++){
            for(int j = 0; j<=36 ;j++) {
                if(Map[i][j] == '#'){
                    Wall w = Wall((j+1)*TILE_SIZEm,(i+1)*TILE_SIZEm, wallTexture);
                    walls.push_back(w);
                }
                if(Map[i][j] == '@'){
                    Stone k = Stone((j+1)*TILE_SIZEm,(i+1)*TILE_SIZEm,stoneTexture);
                    stones.push_back(k);
                }
                if(Map[i][j] == '%'){
                    Bush k = Bush((j+1)*TILE_SIZEm,(i+1)*TILE_SIZEm,bushTexture);
                    bushs.push_back(k);
                }
                if(Map[i][j] == '-'){
                    Ice k = Ice((j+1)*TILE_SIZEm,(i+1)*TILE_SIZEm,iceTexture);
                    ices.push_back(k);
                }
                if(Map[i][j] == '~'){
                    Water k = Water((j+1)*TILE_SIZEm,(i+1)*TILE_SIZEm,waterTexture);
                    waters.push_back(k);
                }
            }
        }

    }
    void handleEvents()
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if(event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_UP: player->move(0 , -10 , walls , stones , waters); break;
                    case SDLK_DOWN: player->move(0 , 10 , walls , stones , waters); break;
                    case SDLK_LEFT: player->move(-10 , 0 , walls , stones , waters); break;
                    case SDLK_RIGHT: player->move(10 , 0 , walls , stones , waters); break;
                    case SDLK_SPACE: player->shoot(); break;
                }
            }
        }
    }

    void renderVictory(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); // Dark green
        SDL_RenderClear(renderer);

        if (font2) {
            SDL_Color gold = {255, 215, 0};

            SDL_Surface* surface = TTF_RenderText_Blended(font2, "Victory!", gold);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect rect = {
                SCREEN_WIDTH / 2 - surface->w / 2,
                SCREEN_HEIGHT / 2 - surface->h / 2,
                surface->w,
                surface->h
            };
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }

        SDL_RenderPresent(renderer);
    }

    void renderGameOver(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 80, 0, 0, 255); // Dark red
        SDL_RenderClear(renderer);

        if (font2) {
            SDL_Color red = {255, 0, 0};

            SDL_Surface* surface = TTF_RenderText_Blended(font2, "Game Over", red);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect rect = {
                SCREEN_WIDTH / 2 - surface->w / 2,
                SCREEN_HEIGHT / 2 - surface->h / 2,
                surface->w,
                surface->h
            };
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }

        SDL_RenderPresent(renderer);
    }

    void update() {
        player->updateBullets();
        for (auto& enemy : enemies) {
            enemy.move(walls , stones , waters);
            enemy.updateBullets();
            if (rand() % 20 < 3) enemy.shoot();
            for (auto& bullet : enemy.bullets) {
                for (auto& wall : walls)
                    if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect))
                    {
                        if (explosionSound) Mix_PlayChannel(-1, explosionSound, 0);
                        wall.active = false;
                        bullet.active = false;
                        break;
                    }
            }
            for (auto& bullet : enemy.bullets) {
                for (auto& stone : stones)
                    if (stone.active && SDL_HasIntersection(&bullet.rect, &stone.rect))
                    {
                        if (explosionSound) Mix_PlayChannel(-1, explosionSound, 0);
                        bullet.active = false;
                        break;
                    }
            }
        }

        for (auto& bullet : player->bullets) {
            for (auto& wall : walls)
                if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect))
                {
                    if (explosionSound) Mix_PlayChannel(-1, explosionSound, 0);
                    wall.active = false;
                    bullet.active = false;
                    break;
                }
            for (auto& enemy : enemies)
                if (enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect))
                {
                    if (explosionSound) Mix_PlayChannel(-1, explosionSound, 0);
                    enemy.active = false;
                    bullet.active = false;
                    break;
                }
            for (auto& stone : stones)
                if (stone.active && SDL_HasIntersection(&bullet.rect, &stone.rect))
                {
                    if (explosionSound) Mix_PlayChannel(-1, explosionSound, 0);
                    bullet.active = false;
                    break;
                }
        }
        if (enemies.size() < maxActiveEnemies && remainingEnemies > 0)
        {
            spawnEnemies();
        }

        enemies.erase(remove_if(enemies.begin(), enemies.end(), [](EnemyTank& e) { return !e.active; }), enemies.end());
        if (enemies.empty())
        {
            state = VICTORY;
            if (victorySound) Mix_PlayChannel(-1, victorySound, 0);
            renderVictory(renderer);
            SDL_Delay(3000);
            running = false;
        }

        for (auto& enemy : enemies)
            for (auto& bullet : enemy.bullets)
                if (SDL_HasIntersection(&bullet.rect, &player->rect))
                {
                    if (gameOverSound) Mix_PlayChannel(-1, gameOverSound, 0);
                    renderGameOver(renderer);
                    SDL_Delay(3000);
                    running = false;
                    return;
                }
    }

    void spawnEnemies()
    {
        int enemiesToSpawn = std::min(maxActiveEnemies - (int)enemies.size(), remainingEnemies);

        const int maxAttempts = 100;

        for (int i = 0; i < enemiesToSpawn; ++i)
        {
            int ex, ey;
            bool valid = false;
            int attempts = 0;

            while (!valid && attempts < maxAttempts)
            {
                ++attempts;
                ex = (rand() % (MAP_WIDTH - 2) + 1) * TILE_SIZE;
                ey = (rand() % (MAP_HEIGHT - 2) + 1) * TILE_SIZE;
                SDL_Rect tempRect = {ex, ey, TILE_SIZE, TILE_SIZE};

                valid = true;

                for (const auto& wall : walls)
                {
                    if (wall.active && SDL_HasIntersection(&tempRect, &wall.rect))
                    {
                        valid = false;
                        break;
                    }
                }
                for (const auto& stone : stones)
                {
                    if (stone.active && SDL_HasIntersection(&tempRect, &stone.rect))
                    {
                        valid = false;
                        break;
                    }
                }
                for (const auto& water : waters)
                {
                    if (SDL_HasIntersection(&tempRect, &water.rect))
                    {
                        valid = false;
                        break;
                    }
                }

                if (valid && SDL_HasIntersection(&tempRect, &player->rect))
                    valid = false;

                if (valid)
                {
                    for (const auto& enemy : enemies)
                    {
                        if (SDL_HasIntersection(&tempRect, &enemy.rect))
                        {
                            valid = false;
                            break;
                        }
                    }
                }
            }

            if (valid)
            {
                enemies.emplace_back(ex, ey, enemyTexture, bulletTexture, walls, shootSound);
                --remainingEnemies;
            }
            else
            {
                std::cerr << "Warning: Could not spawn enemy " << i + 1 << " after " << maxAttempts << " attempts.\n";
            }
        }
    }

};

int main(int argc, char* argv[]) {
    if (TTF_Init() == -1) {
        cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << endl;
        return 1;
    }

    font1 = TTF_OpenFont("arial.ttf", 28);  // Đảm bảo file arial.ttf có trong thư mục chạy
    font2 = TTF_OpenFont("arial2.ttf", 64);
    if (!font1 || !font2) {
        cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << endl;
        return 1;
    }
    Game game;
    if (!game.running) return 1;
    while (game.state != QUIT ) {
        if ( game.state == MENU) {
            handleMenuEvents( game.state, game.running);
            renderMenu(game.renderer);
            SDL_Delay(16);
        }
        else if ( game.state == PLAYING) {
            game.run();  // run() sẽ tự thoát khi thắng hoặc thua
            game.state = QUIT;  // Quay lại menu nếu chơi xong
        }
    }
    TTF_CloseFont(font1);
    TTF_CloseFont(font2);
    TTF_Quit();
    return 0;
}
