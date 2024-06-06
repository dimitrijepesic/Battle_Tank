#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define SIZE 300
#define MAX 20

static char curr_tank[50] = "images/tank1.bmp";
static char diff[50] = "images/lako.bmp";
static char mapsize[50] = "images/10.bmp";
static char music_icon[50] = "images/music.bmp";
static int angle = 0;
int score = 0, lives = 2, tanks_left = 10, N = 10, play_music = 1, exist = 0, bot_difficulty = 10, look = 1;
Uint32 last_shot = 0, shoot_cooldown = 500;
static Uint32 last_move = 0, menu_lasted = 0;
Uint32 enemy_speed = 600;
Uint32 spawn_time = 10000, last_spawn = 0;
SDL_Rect tank;
SDL_Rect meni_rect, zavrsi_rect;
bool skoci = false;

Uint32** enemy_last_turn;
Uint32** enemy_last_shot;
Uint32 turn_cooldown = 500;

int getGameTime() {
    return SDL_GetTicks() - menu_lasted;
}

void draw_menu(SDL_Renderer* renderer, int window_width, int window_height, int*** map, int*** enemies, int*** explosion, int*** directions,
    int*** bonuses, int*** bullets, int* power_up, int* pu_started, int* last_pu, int* pu_placed_time,
    int* pu_x, int* pu_y, int* pu_placed, int* game, int* dir, int* tile_size, SDL_Window* window, int* size, Mix_Music* music, Mix_Music* music2);
void set_game(int*** map, int*** enemies, int*** explosion, int*** directions, int*** bonuses, int*** bullets, int* power_up,
    int* pu_started, int* last_pu, int* pu_placed_time, int* pu_x, int* pu_y, int* pu_placed, int* game, int* dir,
    int* tile_size, SDL_Renderer* renderer, SDL_Window* window, int* size);
void updateHighScore(int sc);
void readHighScores(SDL_Renderer* renderer, int width, int height, Mix_Music* music);

void initialize_enemy_timers() {
    enemy_last_turn = malloc(N * sizeof(Uint32*));
    enemy_last_shot = malloc(N * sizeof(Uint32*));
    for (int i = 0; i < N; i++) {
        enemy_last_turn[i] = malloc(N * sizeof(Uint32));
        enemy_last_shot[i] = malloc(N * sizeof(Uint32));
        for (int j = 0; j < N; j++) {
            enemy_last_turn[i][j] = 0;
            enemy_last_shot[i][j] = 0;
        }
    }
}

void free_enemy_timers() {
    for (int i = 0; i < N; i++) {
        free(enemy_last_turn[i]);
        free(enemy_last_shot[i]);
    }
    free(enemy_last_turn);
    free(enemy_last_shot);
}


static int redosled = 0;

void print_map_and_enemies(int** map, int** enemies, int N) {
    system("clear"); // On Windows, use "cls" instead of "clear"
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (enemies[i][j] > 0) {
                printf("E ");
            }
            else {
                printf("%d ", map[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

typedef struct node {
    int name;
    struct name* parent;
    struct node* up;
    struct node* down;
    struct node* left;
    struct node* right;
} node;

typedef struct {
    int poslednji_potez;
    bool pokusaji[4];
} pomeranje;

void free_all(node* root) {
    if (root == NULL) {
        return;
    }
    free_all(root->up);
    free_all(root->down);
    free_all(root->left);
    free_all(root->right);
    free(root);
}

node* create_node(int name, node* parent) {
    node* nov_node = (node*)malloc(sizeof(node));
    if (nov_node == NULL) {
        exit(1);
    }
    nov_node->name = name;
    nov_node->parent = parent;
    nov_node->up = NULL;
    nov_node->down = NULL;
    nov_node->left = NULL;
    nov_node->right = NULL;
    return nov_node;
}

void generate_map(SDL_Renderer* renderer, int** map, int tile_size) {

    SDL_Surface* surface1 = SDL_LoadBMP("images/t1.bmp");
    SDL_Surface* surface2 = SDL_LoadBMP("images/t2.bmp");
    SDL_Surface* surface3 = SDL_LoadBMP("images/t3.bmp");
    SDL_Surface* surface4 = SDL_LoadBMP("images/t4.bmp");
    SDL_Surface* surface5 = SDL_LoadBMP("images/t5.bmp");

    SDL_Texture* texture1 = SDL_CreateTextureFromSurface(renderer, surface1);
    SDL_Texture* texture2 = SDL_CreateTextureFromSurface(renderer, surface2);
    SDL_Texture* texture3 = SDL_CreateTextureFromSurface(renderer, surface3);
    SDL_Texture* texture4 = SDL_CreateTextureFromSurface(renderer, surface4);
    SDL_Texture* texture5 = SDL_CreateTextureFromSurface(renderer, surface5);

    SDL_FreeSurface(surface1);
    SDL_FreeSurface(surface2);
    SDL_FreeSurface(surface3);
    SDL_FreeSurface(surface4);
    SDL_FreeSurface(surface5);

    SDL_Rect tile[MAX][MAX];
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            tile[i][j].x = i * tile_size;
            tile[i][j].y = j * tile_size;
            tile[i][j].w = tile_size;
            tile[i][j].h = tile_size;
        }
    }

    SDL_Rect select_tile;
    select_tile.w = tile_size;
    select_tile.h = tile_size;
    select_tile.x = 0;
    select_tile.y = 0;

    SDL_SetRenderDrawColor(renderer, 0x66, 0x66, 0xBB, 0xFF);
    SDL_RenderClear(renderer);
    SDL_Delay(25);

    static double water_angle = 0.0;
    water_angle += 90.0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            switch (map[i][j]) {
            case 1:
                SDL_RenderCopyEx(renderer, texture1, &select_tile, &tile[i][j], water_angle, NULL, SDL_FLIP_NONE); // voda
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                SDL_RenderCopy(renderer, texture2, &select_tile, &tile[i][j]); // put
                break;
            case 7:
            case 8:
            case 9:
                SDL_RenderCopy(renderer, texture3, &select_tile, &tile[i][j]); // cigla
                break;
            case 10:
                SDL_RenderCopy(renderer, texture4, &select_tile, &tile[i][j]); // metal
                break;
            case 11:
                SDL_RenderCopy(renderer, texture5, &select_tile, &tile[i][j]); // baza
                break;
            default:
                break;
            }
        }
    }

    SDL_DestroyTexture(texture1);
    SDL_DestroyTexture(texture2);
    SDL_DestroyTexture(texture3);
    SDL_DestroyTexture(texture4);
    SDL_DestroyTexture(texture5);
}


void make_map(int** map) {
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (j == N / 2 || j == 0 || j == N - 1) map[i][j] = 2;
            else map[i][j] = rand() % 10 + 1;
        }
    }
    map[N / 2][N - 1] = 11;
    map[N / 2][N - 2] = 7;
    map[N / 2 - 1][N - 1] = map[N / 2 + 1][N - 1] = 7;
    map[N / 2 - 1][N - 2] = map[N / 2 + 1][N - 2] = 7;
    map[N / 2][N - 3] = 2;
    map[N / 2 - 2][N - 1] = map[N / 2 + 2][N - 1] = 2;
    map[N / 2 - 2][N - 2] = map[N / 2 + 2][N - 2] = 2;
    map[N / 2 - 2][N - 3] = map[N / 2 + 2][N - 3] = 2;
    map[N / 2 - 1][N - 3] = map[N / 2 + 1][N - 3] = 2;
}

void move_tank(SDL_Renderer* renderer, int** map, int dir, int** enemies) {
    SDL_Surface* tank_s = SDL_LoadBMP(curr_tank);
    SDL_Texture* tank_t = SDL_CreateTextureFromSurface(renderer, tank_s);

    int x = tank.x / tank.w;
    int y = tank.y / tank.w;

    SDL_Rect select_tile;
    select_tile.x = 0;
    select_tile.y = 0;
    select_tile.w = tank.w;
    select_tile.h = tank.w;

    switch (dir) {
    case 0: //levo
        look = 0;
        angle = 270;
        if (x > 0 && map[x - 1][y] > 1 && map[x - 1][y] < 7 && !enemies[x - 1][y]) {
            tank.x = tank.x - tank.w;
        }
        break;
    case 1: //gore
        look = 1;
        angle = 0;
        if (y > 0 && map[x][y - 1] > 1 && map[x][y - 1] < 7 && !enemies[x][y - 1]) {
            tank.y = tank.y - tank.w;
        }
        break;
    case 2: //desno
        look = 2;
        angle = 90;
        if (x < N - 1 && map[x + 1][y] > 1 && map[x + 1][y] < 7 && !enemies[x + 1][y]) {
            tank.x = tank.x + tank.w;
        }
        break;
    case 3: //dole
        look = 3;
        angle = 180;
        if (y < N - 1 && map[x][y + 1] > 1 && map[x][y + 1] < 7 && !enemies[x][y + 1]) {
            tank.y = tank.y + tank.w;
        }
        break;
    default:
        break;
    }
    SDL_RenderCopyEx(renderer, tank_t, &select_tile, &tank, angle, NULL, SDL_FLIP_NONE);

    SDL_FreeSurface(tank_s);
    SDL_DestroyTexture(tank_t);
}
void game_over(SDL_Renderer* renderer, int window_width, int window_height, int*** map, int*** enemies, int*** explosion, int*** directions,
    int*** bonuses, int*** bullets, int* power_up, int* pu_started, int* last_pu, int* pu_placed_time,
    int* pu_x, int* pu_y, int* pu_placed, int* game, int* dir, int* tile_size, SDL_Window* window,
    int* size, Mix_Music* music, Mix_Music* music2) {
    exist = 0;
    SDL_Cursor* hand_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_Cursor* arrow_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

    SDL_Surface* mn_s = SDL_LoadBMP("images/meni.bmp");
    SDL_Texture* mn_t = SDL_CreateTextureFromSurface(renderer, mn_s);
    SDL_FreeSurface(mn_s);
    SDL_Surface* ni_s = SDL_LoadBMP("images/novaigra.bmp");
    SDL_Texture* ni_t = SDL_CreateTextureFromSurface(renderer, ni_s);
    SDL_FreeSurface(ni_s);

    TTF_Font* font;
    font = TTF_OpenFont("fonts/BAD GRUNGE.ttf", 200);
    TTF_Font* font2 = TTF_OpenFont("fonts/Mantey-Black.ttf", 30);
    SDL_Surface* text;
    SDL_Surface* text2;
    SDL_Texture* text_texture;
    SDL_Texture* text2_texture;
    SDL_Color color = { 255, 255, 255 };
    char txt[100];

    text = TTF_RenderText_Solid(font, "GAME OVER", color);
    text_texture = SDL_CreateTextureFromSurface(renderer, text);
    sprintf(txt, "Osvojeni rezultat: %d", score);
    text2 = TTF_RenderText_Solid(font2, txt, color);
    text2_texture = SDL_CreateTextureFromSurface(renderer, text2);

    int text_y = 0;
    SDL_Rect dest = { (window_width - text->w) / 2, text_y, text->w, text->h };
    SDL_Rect dest2 = { (window_width - text2->w) / 2 , text2->h * 3, text2->w, text2->h };

    SDL_Rect buttons[2];
    int button_width = 170;
    int button_height = 40;
    int button_spacing = 15;
    int button_x = (window_width - button_width) / 2;
    int first_button_y = (window_height - (3 * button_height + button_spacing));

    for (int i = 0; i < 2; i++) {
        buttons[i] = (SDL_Rect){ button_x, first_button_y + i * (button_height + button_spacing), button_width, button_height };
    }
    bool hover[2] = { false, false };
    bool running = true;
    bool animation_done = false;

    Uint32 start_time = SDL_GetTicks();

    updateHighScore(score);

    while (running) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (!animation_done) {
            Uint32 current_time = SDL_GetTicks();
            Uint32 elapsed_time = current_time - start_time;
            text_y = (elapsed_time / 10) > ((window_height / 2) - (text->h / 2)) ? ((window_height / 2) - (text->h / 2)) : (elapsed_time / 10);

            if (text_y == (window_height / 2 - text->h / 2)) {
                animation_done = true;
            }

            dest.y = text_y;
        }

        SDL_RenderCopy(renderer, text_texture, NULL, &dest);
        SDL_RenderCopy(renderer, text2_texture, NULL, &dest2);

        for (int i = 0; i < 2; i++) {
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 250);
            if (hover[i]) SDL_SetRenderDrawColor(renderer, 190, 190, 190, 100);
            SDL_RenderFillRect(renderer, &buttons[i]);
        }

        SDL_RenderCopy(renderer, ni_t, NULL, &buttons[0]);
        SDL_RenderCopy(renderer, mn_t, NULL, &buttons[1]);

        SDL_RenderPresent(renderer);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            }
            else if (event.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                hover[0] = SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[0]);
                hover[1] = SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[1]);

                if (hover[0] || hover[1]) {
                    SDL_SetCursor(hand_cursor);
                }
                else {
                    SDL_SetCursor(arrow_cursor);
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[0])) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    if (play_music) Mix_PlayMusic(music2, 0);
                    set_game(map, enemies, explosion, directions, bonuses, bullets, power_up,
                        pu_started, last_pu, pu_placed_time, pu_x, pu_y, pu_placed, game,
                        dir, &tile_size, renderer, window, &size);
                    for (int i = 0; i < N; i++) {
                        for (int j = 0; j < N; j++) {
                            (*enemies)[i][j] = 0;
                        }
                    }
                    menu_lasted = SDL_GetTicks();
                    running = false;
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[1])) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    set_game(map, enemies, explosion, directions, bonuses, bullets, power_up,
                        pu_started, last_pu, pu_placed_time, pu_x, pu_y, pu_placed, game,
                        dir, &tile_size, renderer, window, &size);
                    menu_lasted = SDL_GetTicks();
                    skoci = true;
                    running = false;
                }
            }
        }
        SDL_Delay(20);
    }
    SDL_DestroyTexture(mn_t);
    SDL_DestroyTexture(ni_t);
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text);
    SDL_FreeSurface(text2);
    SDL_DestroyTexture(text2_texture);
}

void kill_enemy(int i, int j, int** map, int** enemies, SDL_Renderer* renderer, int** explosion) {
    score += (enemies[i][j] / 4 + 1) * 200;
    enemies[i][j] = 0;
    tanks_left--;
    explosion[i][j] = 12;
    explosion[i][j] = 12;
}

void kill_player(int tile_size, int pu) {
    if (pu == 3) return;
    if (lives) {
        lives--;
        tank.x = N / 2 * tile_size;
        tank.y = (N - 3) * tile_size;
    }
}

void shoot(SDL_Renderer* renderer, int** bullets, int tile_size, int** map, int** enemies, int pu, int** explosion) {
    SDL_Surface* bullet_s = SDL_LoadBMP("images/bullet.bmp");
    SDL_Texture* bullet_t = SDL_CreateTextureFromSurface(renderer, bullet_s);
    SDL_FreeSurface(bullet_s);

    SDL_Rect tile[MAX][MAX];
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            tile[i][j].x = i * tile_size + tile_size / 4;
            tile[i][j].y = j * tile_size + tile_size / 4;
            tile[i][j].w = tile_size / 2;
            tile[i][j].h = tile_size / 2;
        }
    }

    SDL_Rect select_tile;
    select_tile.x = 0;
    select_tile.y = 0;
    select_tile.w = tile_size;
    select_tile.h = tile_size;

    SDL_Delay(50);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            switch (bullets[i][j]) {
            case 1:
                if (i > 0 && enemies[i - 1][j]) {
                    kill_enemy(i - 1, j, map, enemies, renderer, explosion);
                    bullets[i - 1][j] = 1;
                }
                else if (i - 1 == tank.x / tile_size && j == tank.y / tile_size) {
                    kill_player(tile_size, pu);
                    explosion[i - 1][j] = 12;
                }
                SDL_RenderCopyEx(renderer, bullet_t, &select_tile, &tile[i][j], 270, NULL, SDL_FLIP_NONE);
                if (i > 0 && !(map[i - 1][j] >= 7 && map[i - 1][j] <= 10)) {
                    if (!bullets[i - 1][j]) bullets[i - 1][j] = 1;
                    else bullets[i - 1][j] = 0;
                }
                else if (i > 0 && (map[i - 1][j] == 7 || map[i - 1][j] == 8 || map[i - 1][j] == 9 || (map[i - 1][j] == 10 && pu == 6))) { //!!!
                    map[i - 1][j] = 2;
                    explosion[i - 1][j] = 12;
                    SDL_Delay(50);
                }
                else if (i > 0 && !(map[i - 1][j] == 7 || map[i - 1][j] == 8 || map[i - 1][j] == 9)) {
                    explosion[i - 1][j] = 12;
                    SDL_Delay(50);
                }
                bullets[i][j] = 0;
                break;
            case 2:
                if (j > 0 && enemies[i][j - 1]) {
                    kill_enemy(i, j - 1, map, enemies, renderer, explosion);
                    bullets[i][j - 1] = 1;
                }
                else if (i == tank.x / tile_size && j - 1 == tank.y / tile_size) {
                    kill_player(tile_size, pu);
                    explosion[i][j - 1] = 12;
                }
                SDL_RenderCopyEx(renderer, bullet_t, &select_tile, &tile[i][j], 0, NULL, SDL_FLIP_NONE);
                if (j > 0 && !(map[i][j - 1] >= 7 && map[i][j - 1] <= 10)) {
                    if (!bullets[i][j - 1]) bullets[i][j - 1] = 2;
                    else bullets[i][j - 1] = 0;
                }
                else if (j > 0 && (map[i][j - 1] == 7 || map[i][j - 1] == 8 || map[i][j - 1] == 9 || (map[i][j - 1] == 10 && pu == 6))) {//!!!
                    map[i][j - 1] = 2;
                    explosion[i][j - 1] = 12;
                    SDL_Delay(50);
                }
                else if (j > 0 && !(map[i][j - 1] == 7 || map[i][j - 1] == 8 || map[i][j - 1] == 9)) {
                    explosion[i][j - 1] = 12;
                    SDL_Delay(50);
                }
                bullets[i][j] = 0;
                break;
            default:
                break;
            }
        }
    }
    for (int i = N - 1; i >= 0; i--) {
        for (int j = N - 1; j >= 0; j--) {
            switch (bullets[i][j]) {
            case 3:
                if (i < N - 1 && enemies[i + 1][j]) {
                    kill_enemy(i + 1, j, map, enemies, renderer, explosion);
                    bullets[i + 1][j] = 1;
                }
                else if (i + 1 == tank.x / tile_size && j == tank.y / tile_size) {
                    kill_player(tile_size, pu);
                    explosion[i + 1][j] = 12;
                }
                SDL_RenderCopyEx(renderer, bullet_t, &select_tile, &tile[i][j], 90, NULL, SDL_FLIP_NONE);
                if (i < N - 1 && !(map[i + 1][j] >= 7 && map[i + 1][j] <= 10)) {
                    if (!bullets[i + 1][j]) bullets[i + 1][j] = 3;
                    else bullets[i + 1][j] = 0;
                }
                else if (i < N - 1 && (map[i + 1][j] == 7 || map[i + 1][j] == 8 || map[i + 1][j] == 9 || (map[i + 1][j] == 10 && pu == 6))) { //!!!
                    map[i + 1][j] = 2;
                    explosion[i + 1][j] = 12;
                    SDL_Delay(50);
                }
                else if (i < N - 1 && !(map[i + 1][j] == 7 || map[i + 1][j] == 8 || map[i + 1][j] == 9)) {
                    explosion[i + 1][j] = 12;
                    SDL_Delay(50);
                }
                bullets[i][j] = 0;
                break;
            case 4:
                if (j < N - 1 && enemies[i][j + 1]) {
                    kill_enemy(i, j + 1, map, enemies, renderer, explosion);
                    bullets[i][j + 1] = 1;
                }
                else if (i == tank.x / tile_size && j + 1 == tank.y / tile_size) {
                    kill_player(tile_size, pu);
                    explosion[i][j + 1] = 12;
                }
                SDL_RenderCopyEx(renderer, bullet_t, &select_tile, &tile[i][j], 180, NULL, SDL_FLIP_NONE);
                if (j < N - 1 && !(map[i][j + 1] >= 7 && map[i][j + 1] <= 10)) {
                    if (!bullets[i][j + 1]) bullets[i][j + 1] = 4;
                    else bullets[i][j + 1] = 0;
                }
                else if (j < N - 1 && (map[i][j + 1] == 7 || map[i][j + 1] == 8 || map[i][j + 1] == 9 || (map[i][j + 1] == 10 && pu == 6))) { //!!!
                    map[i][j + 1] = 2;
                    explosion[i][j + 1] = 12;
                    SDL_Delay(50);
                }
                else if (j < N - 1 && !(map[i][j + 1] == 7 || map[i][j + 1] == 8 || map[i][j + 1] == 9)) {
                    explosion[i][j + 1] = 12;
                    SDL_Delay(50);
                }
                bullets[i][j] = 0;
                break;
            default:
                break;
            }
        }
    }

}

void enemy_shoot(SDL_Renderer* renderer, int** bullets, int tile_size, int** map, int** enemies, int** explosion, int enemy_x, int enemy_y, int direction) {
    SDL_Surface* bullet_s = SDL_LoadBMP("images/bullet.bmp");
    SDL_Texture* bullet_t = SDL_CreateTextureFromSurface(renderer, bullet_s);
    SDL_FreeSurface(bullet_s);

    SDL_Rect tile[MAX][MAX];
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            tile[i][j].x = i * tile_size + tile_size / 4;
            tile[i][j].y = j * tile_size + tile_size / 4;
            tile[i][j].w = tile_size / 2;
            tile[i][j].h = tile_size / 2;
        }
    }

    SDL_Rect select_tile;
    select_tile.x = 0;
    select_tile.y = 0;
    select_tile.w = tile_size;
    select_tile.h = tile_size;

    // pucanje metka u zadatom pravcu
    switch (direction) {
    case 0: // levo
        if (enemy_x > 0 && map[enemy_x - 1][enemy_y] > 1 && map[enemy_x - 1][enemy_y] < 10 && !enemies[enemy_x - 1][enemy_y]) {
            bullets[enemy_x - 1][enemy_y] = 1;
            if (enemies[enemy_x][enemy_y] > 4) {
                enemies[enemy_x][enemy_y] = 5;
            }
            else {
                enemies[enemy_x][enemy_y] = 1;
            }
        }
        break;
    case 1: // gore
        if (enemy_y > 0 && map[enemy_x][enemy_y - 1] > 1 && map[enemy_x][enemy_y - 1] < 10 && !enemies[enemy_x][enemy_y - 1]) {
            bullets[enemy_x][enemy_y - 1] = 2;
            if (enemies[enemy_x][enemy_y] > 4) {
                enemies[enemy_x][enemy_y] = 6;
            }
            else {
                enemies[enemy_x][enemy_y] = 2;
            }
        }
        break;
    case 2: // desno
        if (enemy_x < N - 1 && map[enemy_x + 1][enemy_y] > 1 && map[enemy_x + 1][enemy_y] < 10 && !enemies[enemy_x + 1][enemy_y]) {
            bullets[enemy_x + 1][enemy_y] = 3;
            if (enemies[enemy_x][enemy_y] > 4) {
                enemies[enemy_x][enemy_y] = 7;
            }
            else {
                enemies[enemy_x][enemy_y] = 3;
            }
        }
        break;
    case 3: // dole
        if (enemy_y < N - 1 && map[enemy_x][enemy_y + 1] > 1 && map[enemy_x][enemy_y + 1] < 10 && !enemies[enemy_x][enemy_y + 1]) {
            bullets[enemy_x][enemy_y + 1] = 4;
            if (enemies[enemy_x][enemy_y] > 4) {
                enemies[enemy_x][enemy_y] = 8;
            }
            else {
                enemies[enemy_x][enemy_y] = 4;
            }
        }
        break;
    default:
        break;
    }

    SDL_DestroyTexture(bullet_t);
}

void startPu(int* power_up, int** map, int* last_pu, int* pu_placed, int* pu_x, int* pu_y, int* pu_started, int* pu_placed_time, int** enemies, SDL_Renderer* renderer, int** explosion) {
    *power_up = *pu_placed;
    *last_pu = getGameTime();
    *pu_started = getGameTime();
    *pu_placed_time = 0;
    *pu_x = -1;
    *pu_y = -1;
    *pu_placed = 0;
    if (*power_up == 7) {
        map[N / 2 - 1][N - 1] = map[N / 2 + 1][N - 1] = 10;
        map[N / 2 - 1][N - 2] = map[N / 2 + 1][N - 2] = map[N / 2][N - 2] = 10;
    }
    if (*power_up == 4) {
        shoot_cooldown = 300;
    }
    if (*power_up == 1) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (enemies[i][j]) {
                    kill_enemy(i, j, map, enemies, renderer, explosion);
                }
            }
        }
    }
}

void generate_enemy(SDL_Renderer* renderer, int** enemies, int tile_size) {
    // 1 - obican gleda levo, 2 - obican gleda napred, 3 - obican gleda desno, 4 - obican gleda dole
    // 5 - specijalni gleda levo, 6 - specijalni gleda napred, 7 - specijalni gleda desno, 8 - specijalni gleda dole

    SDL_Surface* enemy0 = SDL_LoadBMP("images/enemy0.bmp");
    SDL_Surface* enemy1 = SDL_LoadBMP("images/enemy1.bmp");
    SDL_Surface* enemy2 = SDL_LoadBMP("images/enemy2.bmp");
    SDL_Surface* enemy3 = SDL_LoadBMP("images/enemy3.bmp");
    SDL_Surface* spec0 = SDL_LoadBMP("images/spec0.bmp");
    SDL_Surface* spec1 = SDL_LoadBMP("images/spec1.bmp");
    SDL_Surface* spec2 = SDL_LoadBMP("images/spec2.bmp");
    SDL_Surface* spec3 = SDL_LoadBMP("images/spec3.bmp");

    SDL_Texture* enemy0_t = SDL_CreateTextureFromSurface(renderer, enemy0);
    SDL_Texture* enemy1_t = SDL_CreateTextureFromSurface(renderer, enemy1);
    SDL_Texture* enemy2_t = SDL_CreateTextureFromSurface(renderer, enemy2);
    SDL_Texture* enemy3_t = SDL_CreateTextureFromSurface(renderer, enemy3);
    SDL_Texture* spec0_t = SDL_CreateTextureFromSurface(renderer, spec0);
    SDL_Texture* spec1_t = SDL_CreateTextureFromSurface(renderer, spec1);
    SDL_Texture* spec2_t = SDL_CreateTextureFromSurface(renderer, spec2);
    SDL_Texture* spec3_t = SDL_CreateTextureFromSurface(renderer, spec3);

    SDL_FreeSurface(enemy0);
    SDL_FreeSurface(enemy1);
    SDL_FreeSurface(enemy2);
    SDL_FreeSurface(enemy3);
    SDL_FreeSurface(spec0);
    SDL_FreeSurface(spec1);
    SDL_FreeSurface(spec2);
    SDL_FreeSurface(spec3);

    SDL_Rect tile[MAX][MAX];
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            tile[i][j].x = i * tile_size;
            tile[i][j].y = j * tile_size;
            tile[i][j].w = tile_size;
            tile[i][j].h = tile_size;
        }
    }

    SDL_Rect select_tile;
    select_tile.x = 0;
    select_tile.y = 0;
    select_tile.w = tile_size;
    select_tile.h = tile_size;


    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            switch (enemies[i][j]) {
            case 1:
                SDL_RenderCopy(renderer, enemy0_t, &select_tile, &tile[i][j]);
                break;
            case 2:
                SDL_RenderCopy(renderer, enemy1_t, &select_tile, &tile[i][j]);
                break;
            case 3:
                SDL_RenderCopy(renderer, enemy2_t, &select_tile, &tile[i][j]);
                break;
            case 4:
                SDL_RenderCopy(renderer, enemy3_t, &select_tile, &tile[i][j]);
                break;
            case 5:
                SDL_RenderCopy(renderer, spec0_t, &select_tile, &tile[i][j]);
                break;
            case 6:
                SDL_RenderCopy(renderer, spec1_t, &select_tile, &tile[i][j]);
                break;
            case 7:
                SDL_RenderCopy(renderer, spec2_t, &select_tile, &tile[i][j]);
                break;
            case 8:
                SDL_RenderCopy(renderer, spec3_t, &select_tile, &tile[i][j]);
                break;
            default:
                break;
            }
        }
    }
    SDL_DestroyTexture(enemy0_t);
    SDL_DestroyTexture(enemy1_t);
    SDL_DestroyTexture(enemy2_t);
    SDL_DestroyTexture(enemy3_t);
    SDL_DestroyTexture(spec0_t);
    SDL_DestroyTexture(spec1_t);
    SDL_DestroyTexture(spec2_t);
    SDL_DestroyTexture(spec3_t);
}

bool not_in(int x, int* niz, int length) {
    for (int i = 0; i < length; i++) {
        if (niz[i] == x) return false;
    }
    return true;
}

void random_next(int x, int y, int xnas, int ynas, int** map, int** enemies, int next[2], int* dir) {
    int val[] = { 1, 7, 8, 9, 10, 11 };
    int val_length = 6;
    int move[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };
    int indices[] = { 0, 1, 2, 3 };

    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    next[0] = x;
    next[1] = y;

    for (int i = 0; i < 4; i++) {
        int idx = indices[i];
        int nx = x + move[idx][0];
        int ny = y + move[idx][1];
        if (nx >= 0 && nx < N && ny >= 0 && ny < N &&
            not_in(map[nx][ny], val, val_length) &&
            enemies[nx][ny] == 0 && !(nx == xnas && ny == ynas)) {
            next[0] = nx;
            next[1] = ny;
            *dir = idx;
            return;
        }
    }
}

void bfs_next(int x, int y, int x_tar, int y_tar, int xnas, int ynas, int** map, int** enemies, int next[2], int* dir) {
    int val[] = { 1, 7, 8, 9, 10, 11 };
    int val_length = 6;

    int** visited = malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        visited[i] = malloc(N * sizeof(int));
        memset(visited[i], -1, N * sizeof(int));
    }

    typedef struct {
        int x, y;
    } Queue;

    Queue* queue = malloc(N * N * sizeof(Queue));
    int front = 0, rear = 0;

    queue[rear++] = (Queue){ x, y }; //ubacujem u red trenutnu poziciju neprijatelja
    visited[x][y] = 0;

    int move[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };
    Queue fmove = { x, y }; // stavljam pocetni potez na invalid

    while (front != rear) {
        Queue curr = queue[front++];
        if (curr.x == x_tar && curr.y == y_tar) {
            fmove.x = x_tar;
            fmove.y = y_tar;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int nx = curr.x + move[i][0];
            int ny = curr.y + move[i][1];
            if (nx >= 0 && nx < N && ny >= 0 && ny < N &&
                not_in(map[nx][ny], val, val_length) &&
                visited[nx][ny] == -1 &&
                enemies[nx][ny] == 0 && !(nx == xnas && ny == ynas)) {
                visited[nx][ny] = visited[curr.x][curr.y] + 1;
                queue[rear++] = (Queue){ nx, ny };
                if (fmove.x == x && fmove.y == y) {
                    fmove.x = nx;
                    fmove.y = ny;
                    *dir = i;
                }
            }
        }
    }

    next[0] = fmove.x; next[1] = fmove.y;

    for (int i = 0; i < N; i++) {
        free(visited[i]);
    }
    free(visited);
    free(queue);
}

int tank_optioning(int diff) {
    static int br = 0;
    br++;
    switch (diff) {
    case 0: {
        if (br == 4) {
            br = 0;
            return 1;
        }
        else return 0;
    }
    case 1: {
        return rand() % 2;
    }
    case 2: {
        if (br == 2) {
            br = 0;
            return 1;
        }
        else return 0;
    }
    }
}

void spawn_enemies(int** enemies, int diff) {
    int spawn_points[2][2] = { {0,0}, {N - 1,0} };
    int typee[] = { 0,1 }; // 0 - obican, 1 - spec
    for (int i = 0; i < 2; i++) {
        int x = spawn_points[i][0], y = spawn_points[i][1];
        if (enemies[x][y] == 0) {
            int promenljiva = tank_optioning(diff);
            if (promenljiva == 0) {
                if (x == 0) {
                    enemies[x][y] = 3;
                }
                else {
                    enemies[x][y] = 1;
                }
            }
            else {
                if (x == 0) {
                    enemies[x][y] = 7;
                }
                else {
                    enemies[x][y] = 5;
                }
            }
        }
    }
}

void baza(int** map, int* bazax, int* bazay) {
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (map[i][j] == 11) {
                bazax = i;
                bazay = j;
                return;
            }
        }
    }
}

int check_tank_pos(int enemy_x, int enemy_y, int** map) {
    int tank_x = tank.x / tank.w;
    int tank_y = tank.y / tank.h;
    int i = 0, j = 0;
    if (enemy_x == tank_x) {
        j = enemy_y;
        while (j != tank_y) {
            if (enemy_y > tank_y) {
                j--;
            }
            else if (enemy_y < tank_y) {
                j++;
            }
        }
        return (enemy_x > tank_x) ? 1 : 3;
    }
    if (enemy_y == tank_y) {
        j = enemy_y;
        while (j != tank_y) {
            if (enemy_y > tank_y) {
                j--;
            }
            else if (enemy_y < tank_y) {
                j++;
            }
        }
        return (enemy_y > tank_y) ? 0 : 2;
    }
    return -1;
}

int check_base_pos(int enemy_x, int enemy_y, int** map) {
    int val[] = { 10, 11 };
    int val_length = 2;
    int base_x = N / 2;
    int base_y = N - 1;

    if (enemy_x == base_x) {
        int j = enemy_y;
        while (j != base_y) {
            if (!not_in(map[enemy_x][j], val, val_length)) {
                return -1;
            }
            if (enemy_y > base_y) {
                j--;
            }
            else if (enemy_y < base_y) {
                j++;
            }
        }
        return (enemy_y > base_y) ? 3 : 1;
    }

    if (enemy_y == base_y) {
        int i = enemy_x;
        while (i != base_x) {
            if (!not_in(map[i][enemy_y], val, val_length)) {
                return -1;
            }
            if (enemy_x > base_x) {
                i--;
            }
            else if (enemy_x < base_x) {
                i++;
            }
        }
        return (enemy_x > base_x) ? 0 : 2;
    }

    return -1;
}

void update_enemy_pos(SDL_Renderer* renderer, int** bullets, int tile_size, int** explosion, int** map, int** enemies, int N, int diff, int x_tar, int y_tar) {
    int** new_enemies = malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        new_enemies[i] = calloc(N, sizeof(int));
    }

    int next[2];
    int dir; // dir: 3-levo 2-gore 1-desno 0-dole
    int tankx = tank.x / tank.w;
    int tanky = tank.y / tank.h;
    int bazax = 0, bazay = 0;
    baza(map, &bazax, &bazay);
    Uint32 curr_time = getGameTime();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (enemies[i][j] > 0) {
                switch (diff) {
                case 0:
                case 1:
                    random_next(i, j, tankx, tanky, map, enemies, next, &dir);
                    break;
                case 2:
                    if (enemies[i][j] < 5) {
                        bfs_next(i, j, x_tar, y_tar, tankx, tanky, map, enemies, next, &dir);
                    }
                    else {
                        bfs_next(i, j, bazax, bazay, tankx, tanky, map, enemies, next, &dir);
                    }
                    break;
                default:
                    next[0] = i;
                    next[1] = j;
                    dir = -1;
                    break;
                }

                if (curr_time - enemy_last_turn[i][j] > turn_cooldown) {
                    enemy_last_turn[i][j] = curr_time;

                    if (enemies[i][j] < 5) {
                        switch (dir) {
                        case 0: new_enemies[next[0]][next[1]] = 4; break;
                        case 1: new_enemies[next[0]][next[1]] = 3; break;
                        case 2: new_enemies[next[0]][next[1]] = 2; break;
                        case 3: new_enemies[next[0]][next[1]] = 1; break;
                        default: new_enemies[i][j] = enemies[i][j]; break;
                        }
                    }
                    else {
                        switch (dir) {
                        case 0: new_enemies[next[0]][next[1]] = 8; break;
                        case 1: new_enemies[next[0]][next[1]] = 7; break;
                        case 2: new_enemies[next[0]][next[1]] = 6; break;
                        case 3: new_enemies[next[0]][next[1]] = 5; break;
                        default: new_enemies[i][j] = enemies[i][j]; break;
                        }
                    }

                    int player_shoot_dir = check_tank_pos(next[0], next[1], map);
                    if (player_shoot_dir != -1 && curr_time - enemy_last_shot[i][j] > shoot_cooldown) {
                        enemy_last_shot[i][j] = curr_time + turn_cooldown;
                        enemy_shoot(renderer, bullets, tile_size, map, enemies, explosion, next[0], next[1], player_shoot_dir);
                    }

                    int base_shoot_dir = check_base_pos(next[0], next[1], map);
                    if (base_shoot_dir != -1 && curr_time - enemy_last_shot[i][j] > shoot_cooldown) {
                        enemy_last_shot[i][j] = curr_time + turn_cooldown;
                        enemy_shoot(renderer, bullets, tile_size, map, enemies, explosion, next[0], next[1], base_shoot_dir);
                    }
                }
                else {
                    new_enemies[i][j] = enemies[i][j];
                }
            }
        }
    }

    for (int i = 0; i < N; i++) {
        memcpy(enemies[i], new_enemies[i], N * sizeof(int));
        free(new_enemies[i]);
    }
    free(new_enemies);
}

void powerUp(int* power_up, int* pu_started, int* last_pu, int* pu_placed_time, int** map, int* pu_x, int* pu_y, int* pu_placed, int** enemies, SDL_Renderer* renderer) {
    if (*power_up) {
        switch (*power_up) {
        case 4: // zvezda
            if (getGameTime() > *pu_started + 15000) {
                *pu_started = 0;
                *last_pu = getGameTime();
                *power_up = 0;
                shoot_cooldown = 500;
            }
            break;
        case 1: // ubij sve neprijatelje
        case 2: // zaledi neprijatelje
        case 3: // stit
        case 5: //zvezda 2
        case 6: // zvezda 3
            if (getGameTime() > *pu_started + 15000) {
                *pu_started = 0;
                *last_pu = getGameTime();
                *power_up = 0;
            }
            break;
        case 7: // lopata
            if (getGameTime() > *pu_started + 15000) {
                *pu_started = 0;
                *last_pu = getGameTime();
                *power_up = 0;
                map[N / 2 - 1][N - 1] = map[N / 2 + 1][N - 1] = 7;
                map[N / 2 - 1][N - 2] = map[N / 2 + 1][N - 2] = map[N / 2][N - 2] = 7;
            }
            break;
        default: // srce
            if (lives < 3)
                lives++;
            *power_up = 0;
            *pu_started = 0;
            *last_pu = getGameTime();
            break;
        }
    }
    else if (*pu_placed) {
        if (getGameTime() > *pu_placed_time + 10000) { // skloni mocku
            *last_pu = getGameTime();
            *pu_placed_time = 0;
            *pu_y = -1;
            *pu_x = -1;
            *pu_placed = 0;
        }
    }
    else if (getGameTime() > *last_pu + 10000) { // stavi mocku
        srand(time(NULL));
        *pu_placed = rand() % 8 + 1;
        *pu_x = rand() % N, * pu_y = rand() % N;
        while (map[*pu_x][*pu_y] < 2 || map[*pu_x][*pu_y] > 6 || enemies[*pu_x][*pu_y] || tank.x == *pu_y * tank.w || tank.y == *pu_x * tank.w) {
            *pu_x = rand() % N, * pu_y = rand() % N;
        }
        *pu_placed_time = getGameTime();
    }
}

void draw_explosion(SDL_Renderer* renderer, int** explosion, int tile_size, bool* new_exp) {
    SDL_Surface* explosion_s = SDL_LoadBMP("images/explosion.bmp");
    SDL_Texture* explosion_t = SDL_CreateTextureFromSurface(renderer, explosion_s);
    SDL_FreeSurface(explosion_s);

    SDL_Rect tile[MAX][MAX];
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            tile[i][j].x = i * tile_size;
            tile[i][j].y = j * tile_size;
            tile[i][j].w = tile_size;
            tile[i][j].h = tile_size;
        }
    }

    SDL_Rect select_tile;
    select_tile.w = 96;
    select_tile.h = 96;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (explosion[i][j] > 0) {
                if (explosion[i][j] == 11) *new_exp = true;
                select_tile.x = (12 - explosion[i][j]) * 96;
                select_tile.y = 0;
                SDL_RenderCopy(renderer, explosion_t, &select_tile, &tile[i][j]);
                explosion[i][j]--;
            }
        }
    }

    SDL_DestroyTexture(explosion_t);
}


void drawPowerUp(SDL_Renderer* renderer, int** map, int tile_size, int pu_x, int pu_y, int pu_placed, Uint32 pu_placed_time, Uint32 current_time) {
    if (pu_placed) {
        SDL_Surface* pu_s = NULL;
        switch (pu_placed) {
        case 1: pu_s = SDL_LoadBMP("images/bomb.bmp"); break;
        case 2: pu_s = SDL_LoadBMP("images/time.bmp"); break;
        case 3: pu_s = SDL_LoadBMP("images/shield.bmp"); break;
        case 4: pu_s = SDL_LoadBMP("images/one.bmp"); break;
        case 5: pu_s = SDL_LoadBMP("images/two.bmp"); break;
        case 6: pu_s = SDL_LoadBMP("images/three.bmp"); break;
        case 7: pu_s = SDL_LoadBMP("images/shovel.bmp"); break;
        case 8: pu_s = SDL_LoadBMP("images/life.bmp"); break;
        }
        SDL_Texture* pu_t = SDL_CreateTextureFromSurface(renderer, pu_s);
        SDL_FreeSurface(pu_s);

        Uint32 elapsed = current_time - pu_placed_time;
        Uint32 remaining_time = 10000 - elapsed;

        bool visible = true;
        if (remaining_time < 3000) {
            if ((remaining_time / 300) % 2 == 0) {
                visible = false;
            }
        }

        if (visible) {
            SDL_Rect rect = { pu_x * tile_size, pu_y * tile_size, tile_size, tile_size };
            SDL_RenderCopy(renderer, pu_t, NULL, &rect);
        }

        SDL_DestroyTexture(pu_t);
    }
}

void drawHUD(SDL_Renderer* renderer, int power_up, int tile_size, bool meni_hover, bool zavrsi_hover, int size) {

    SDL_Surface* bg_s = SDL_LoadBMP("images/bg.bmp");
    SDL_Texture* bg_t = SDL_CreateTextureFromSurface(renderer, bg_s);
    SDL_FreeSurface(bg_s);

    TTF_Font* font;
    font = TTF_OpenFont("fonts/Mantey-Black.ttf", 24);
    SDL_Surface* text;
    SDL_Texture* text_texture;
    SDL_Color color = { 255, 255, 255 };
    char tekst[50];

    int heart_size = size / 6;

    SDL_Rect right_hud_rect = { N * tile_size, 0, size, (N + 1) * tile_size };
    SDL_RenderCopy(renderer, bg_t, NULL, &right_hud_rect);
    SDL_DestroyTexture(bg_t);

    text = TTF_RenderText_Solid(font, "Zivoti: ", color);
    text_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_Rect dest = { N * tile_size + 20, text->h, text->w, text->h };
    SDL_RenderCopy(renderer, text_texture, NULL, &dest);

    sprintf(tekst, "Rezultat:     %d", score);
    text = TTF_RenderText_Solid(font, tekst, color);
    text_texture = SDL_CreateTextureFromSurface(renderer, text);
    dest.y += text->h + 25;
    dest.w = text->w;
    SDL_RenderCopy(renderer, text_texture, NULL, &dest);

    SDL_Surface* heart_s = SDL_LoadBMP("images/life.bmp");
    SDL_Texture* heart_t = SDL_CreateTextureFromSurface(renderer, heart_s);
    SDL_FreeSurface(heart_s);
    for (int i = 0; i < lives; i++) {
        SDL_Rect heart_rect = { N * tile_size + size / 3 + (i % 3) * heart_size, (i / 3) * heart_size + text->h / 3 * 2, heart_size, heart_size };
        SDL_RenderCopy(renderer, heart_t, NULL, &heart_rect);
    }
    SDL_DestroyTexture(heart_t);

    SDL_Rect pu_border_rect = { N * tile_size + size / 2 - size / 10, size / 2, size / 5 + 4, size / 5 + 4 };
    SDL_Rect line = { N * tile_size, 0, 5, (N + 1) * tile_size };
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 250);
    SDL_RenderFillRect(renderer, &line);

    text = TTF_RenderText_Solid(font, "Bonus:", color);
    text_texture = SDL_CreateTextureFromSurface(renderer, text);
    dest.y += text->h * 2 + 20;
    dest.w = text->w;
    SDL_RenderCopy(renderer, text_texture, NULL, &dest);

    SDL_Surface* pu_s = NULL;
    switch (power_up) {
    case 1: pu_s = SDL_LoadBMP("images/bomb.bmp"); SDL_RenderFillRect(renderer, &pu_border_rect); break;
    case 2: pu_s = SDL_LoadBMP("images/time.bmp"); SDL_RenderFillRect(renderer, &pu_border_rect); break;
    case 3: pu_s = SDL_LoadBMP("images/shield.bmp"); SDL_RenderFillRect(renderer, &pu_border_rect); break;
    case 4: pu_s = SDL_LoadBMP("images/one.bmp"); SDL_RenderFillRect(renderer, &pu_border_rect); break;
    case 5: pu_s = SDL_LoadBMP("images/two.bmp"); SDL_RenderFillRect(renderer, &pu_border_rect); break;
    case 6: pu_s = SDL_LoadBMP("images/three.bmp"); SDL_RenderFillRect(renderer, &pu_border_rect); break;
    case 7: pu_s = SDL_LoadBMP("images/shovel.bmp"); SDL_RenderFillRect(renderer, &pu_border_rect); break;
    }
    if (pu_s != NULL) {
        SDL_Texture* pu_t = SDL_CreateTextureFromSurface(renderer, pu_s);
        SDL_FreeSurface(pu_s);
        SDL_Rect pu_rect = { N * tile_size + size / 2 - size / 10 + 2, size / 2 + 2, size / 5, size / 5 };
        SDL_RenderCopy(renderer, pu_t, NULL, &pu_rect);
        SDL_DestroyTexture(pu_t);
    }

    meni_rect = (SDL_Rect){ N * tile_size + size / 6, N * tile_size - 3 * size / 5 + 3, size / 3 * 2, size / 5 - 6 };
    zavrsi_rect = (SDL_Rect){ N * tile_size + size / 6, N * tile_size - 2 * size / 5 + 3, size / 3 * 2, size / 5 - 6 };

    if (meni_hover) {
        SDL_SetRenderDrawColor(renderer, 190, 190, 190, 100);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 250);
    }
    SDL_RenderFillRect(renderer, &meni_rect);

    if (zavrsi_hover) {
        SDL_SetRenderDrawColor(renderer, 190, 190, 190, 100);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 250);
    }
    SDL_RenderFillRect(renderer, &zavrsi_rect);

    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text);

    SDL_Surface* hudr_s = SDL_LoadBMP("images/hudr.bmp");
    SDL_Texture* hudr_t = SDL_CreateTextureFromSurface(renderer, hudr_s);
    SDL_FreeSurface(hudr_s);
    SDL_Rect hudr_rect = { N * tile_size + meni_rect.w / 4, N * tile_size - 3 * size / 5, meni_rect.w, 2 * size / 5 };
    SDL_RenderCopy(renderer, hudr_t, NULL, &hudr_rect);
    SDL_DestroyTexture(hudr_t);
}

void start_animation(SDL_Renderer* renderer, int window_width, int window_height, Mix_Music* intro) {
    SDL_Texture* textures[55];
    char filename[50];

    Mix_PlayMusic(intro, 0);

    for (int i = 0; i < 55; i++) {
        sprintf(filename, "images/tank_%03d.bmp", i);
        SDL_Surface* frame = SDL_LoadBMP(filename);
        textures[i] = SDL_CreateTextureFromSurface(renderer, frame);
        SDL_FreeSurface(frame);
    }
    SDL_Rect render_quad = { 0, 0, window_width, window_height };
    Uint32 start_time = SDL_GetTicks();
    Uint32 current_time;
    int frame_duration = 70;
    while (1) {
        current_time = SDL_GetTicks();
        if (current_time - start_time > 3850) {
            break;
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        int frame_index = (current_time - start_time) / frame_duration;
        SDL_RenderCopy(renderer, textures[frame_index], NULL, &render_quad);
        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }
    for (int i = 0; i < 55; i++) {
        SDL_DestroyTexture(textures[i]);
    }
}

void set_game(int*** map, int*** enemies, int*** explosion, int*** directions, int*** bonuses, int*** bullets, int* power_up,
    int* pu_started, int* last_pu, int* pu_placed_time, int* pu_x, int* pu_y, int* pu_placed, int* game, int* dir,
    int* tile_size, SDL_Renderer* renderer, SDL_Window* window, int* size) {

    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    *size = current.h / SIZE * SIZE;
    *tile_size = *size / N;

    angle = 0;
    look = 1;
    score = 0, lives = 2, tanks_left = bot_difficulty;
    *map = (int**)calloc(N, sizeof(int*));
    if (*map == NULL) {
        perror("Greska u alokaciji memorije za mapu.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
    for (int i = 0; i < N; i++)
        (*map)[i] = (int*)calloc(N, sizeof(int));
    make_map(*map);

    *enemies = (int**)calloc(N, sizeof(int*));
    for (int i = 0; i < N; i++)
        (*enemies)[i] = (int*)calloc(N, sizeof(int));

    *explosion = (int**)calloc(N, sizeof(int*));
    for (int i = 0; i < N; i++)
        (*explosion)[i] = (int*)calloc(N, sizeof(int));

    *directions = (int**)calloc(N, sizeof(int*));
    for (int i = 0; i < N; i++)
        (*directions)[i] = (int*)calloc(N, sizeof(int));

    *bonuses = (int**)calloc(N, sizeof(int*));
    for (int i = 0; i < N; i++)
        (*bonuses)[i] = (int*)calloc(N, sizeof(int));

    *bullets = (int**)calloc(N, sizeof(int*));
    if (*bullets == NULL) {
        perror("Greska u alokaciji memorije za metkove.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
    for (int i = 0; i < N; i++)
        (*bullets)[i] = (int*)calloc(N, sizeof(int));

    tank.x = N / 2 * *tile_size;
    tank.y = (N - 3) * *tile_size;
    tank.w = *tile_size;
    tank.h = *tile_size;

    *power_up = 0;
    *pu_started = 0;
    *last_pu = 0;
    *pu_placed_time = 0;
    *pu_x = -1;
    *pu_y = -1;
    *pu_placed = 0;

    *game = 1;
    *dir = -1; // 0 levo, 1 gore, 2 desno, 3 dole
}

void draw_settings(SDL_Renderer* renderer, int window_width, int window_height, int*** map, int*** enemies, int*** explosion, int*** directions,
    int*** bonuses, int*** bullets, int* power_up, int* pu_started, int* last_pu, int* pu_placed_time,
    int* pu_x, int* pu_y, int* pu_placed, int* game, int* dir, int* tile_size, SDL_Window* window,
    int* size, Mix_Music* music) {

    SDL_Cursor* hand_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_Cursor* arrow_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

    SDL_Surface* leftarrow_s = SDL_LoadBMP("images/leftarrow.bmp");
    SDL_Texture* leftarrow_t = SDL_CreateTextureFromSurface(renderer, leftarrow_s);
    SDL_FreeSurface(leftarrow_s);
    SDL_Surface* rightarrow_s = SDL_LoadBMP("images/rightarrow.bmp");
    SDL_Texture* rightarrow_t = SDL_CreateTextureFromSurface(renderer, rightarrow_s);
    SDL_FreeSurface(rightarrow_s);
    SDL_Surface* potvrdi_s = SDL_LoadBMP("images/potvrdi.bmp");
    SDL_Texture* potvrdi_t = SDL_CreateTextureFromSurface(renderer, potvrdi_s);
    SDL_Surface* difficulty_s = SDL_LoadBMP(diff);
    SDL_Texture* difficulty_t = SDL_CreateTextureFromSurface(renderer, difficulty_s);
    SDL_Surface* velicina_mape_s = SDL_LoadBMP("images/vm.bmp");
    SDL_Texture* velicina_mape_t = SDL_CreateTextureFromSurface(renderer, velicina_mape_s);
    SDL_FreeSurface(velicina_mape_s);
    SDL_Surface* tezina_botova_s = SDL_LoadBMP("images/tb.bmp");
    SDL_Texture* tezina_botova_t = SDL_CreateTextureFromSurface(renderer, tezina_botova_s);
    SDL_FreeSurface(tezina_botova_s);
    SDL_Surface* map_size_s = SDL_LoadBMP(mapsize);
    SDL_Texture* map_size_t = SDL_CreateTextureFromSurface(renderer, map_size_s);
    SDL_Surface* skin_s = SDL_LoadBMP(curr_tank);
    SDL_Texture* skin_t = SDL_CreateTextureFromSurface(renderer, skin_s);

    bool running = true;
    bool hover1 = false, hover2 = false, hover3 = false, hover4 = false, hover5 = false, hover6 = false, hover7 = false;
    int old_diff = bot_difficulty, old_size = N;
    while (running) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect tb_rect = { (window_width - *size) / 4, window_height / 6, *size / 2, *size / 12 };
        SDL_RenderCopy(renderer, tezina_botova_t, NULL, &tb_rect);

        SDL_Rect difficulty_rect = { tb_rect.x + tb_rect.w + *size / 4, tb_rect.y, *size / 3, *size / 12 };
        SDL_RenderCopy(renderer, difficulty_t, NULL, &difficulty_rect);
        SDL_Rect leftarrow1_rect = { difficulty_rect.x - 50, difficulty_rect.y, 50, 50 };
        SDL_RenderCopy(renderer, leftarrow_t, NULL, &leftarrow1_rect);
        SDL_Rect rightarrow1_rect = { difficulty_rect.x + difficulty_rect.w, difficulty_rect.y, 50, 50 };
        SDL_RenderCopy(renderer, rightarrow_t, NULL, &rightarrow1_rect);

        SDL_Rect vm_rect = { (window_width - *size) / 4, window_height / 3, *size / 2, *size / 12 };
        SDL_RenderCopy(renderer, velicina_mape_t, NULL, &vm_rect);

        SDL_Rect map_size_rect = { vm_rect.x + vm_rect.w + *size / 4, vm_rect.y, *size / 9, *size / 12 };
        SDL_RenderCopy(renderer, map_size_t, NULL, &map_size_rect);
        SDL_Rect leftarrow2_rect = { map_size_rect.x - 50, map_size_rect.y, 50, 50 };
        SDL_RenderCopy(renderer, leftarrow_t, NULL, &leftarrow2_rect);
        SDL_Rect rightarrow2_rect = { map_size_rect.x + map_size_rect.w, map_size_rect.y, 50, 50 };
        SDL_RenderCopy(renderer, rightarrow_t, NULL, &rightarrow2_rect);

        SDL_Rect skin_rect = { (window_width - *size / 5) / 2, window_height / 2 + 20, *size / 5, *size / 5 };
        SDL_RenderCopy(renderer, skin_t, NULL, &skin_rect);
        SDL_Rect leftarrow3_rect = { skin_rect.x - 100, skin_rect.y + 40, 50, 50 };
        SDL_RenderCopy(renderer, leftarrow_t, NULL, &leftarrow3_rect);
        SDL_Rect rightarrow3_rect = { skin_rect.x + skin_rect.w + 50, skin_rect.y + 40, 50, 50 };
        SDL_RenderCopy(renderer, rightarrow_t, NULL, &rightarrow3_rect);

        SDL_Rect potvrdi_rect = { (window_width - 200) / 2, window_height - 80, 200, 50 };
        SDL_RenderCopy(renderer, potvrdi_t, NULL, &potvrdi_rect);
        SDL_RenderPresent(renderer);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            }
            else if (event.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                hover1 = SDL_PointInRect(&(SDL_Point) { x, y }, & leftarrow1_rect);
                hover2 = SDL_PointInRect(&(SDL_Point) { x, y }, & rightarrow1_rect);
                hover3 = SDL_PointInRect(&(SDL_Point) { x, y }, & leftarrow2_rect);
                hover4 = SDL_PointInRect(&(SDL_Point) { x, y }, & rightarrow2_rect);
                hover5 = SDL_PointInRect(&(SDL_Point) { x, y }, & potvrdi_rect);
                hover6 = SDL_PointInRect(&(SDL_Point) { x, y }, & leftarrow3_rect);
                hover7 = SDL_PointInRect(&(SDL_Point) { x, y }, & rightarrow3_rect);

                if (hover1 || hover2 || hover3 || hover4 || hover5 || hover6 || hover7) {
                    SDL_SetCursor(hand_cursor);
                }
                else {
                    SDL_SetCursor(arrow_cursor);
                }
                if (hover5) {
                    potvrdi_s = SDL_LoadBMP("images/potvrdis.bmp");
                    potvrdi_t = SDL_CreateTextureFromSurface(renderer, potvrdi_s);
                }
                else {
                    potvrdi_s = SDL_LoadBMP("images/potvrdi.bmp");
                    potvrdi_t = SDL_CreateTextureFromSurface(renderer, potvrdi_s);
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (SDL_PointInRect(&(SDL_Point) { x, y }, & leftarrow1_rect)) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    if (bot_difficulty == 15) {
                        strcpy(diff, "images/lako.bmp");
                        difficulty_s = SDL_LoadBMP(diff);
                        difficulty_t = SDL_CreateTextureFromSurface(renderer, difficulty_s);
                        bot_difficulty = 10;
                        enemy_speed = 600;
                        spawn_time = 10000;
                    }
                    else if (bot_difficulty == 20) {
                        strcpy(diff, "images/srednje.bmp");
                        difficulty_s = SDL_LoadBMP(diff);
                        difficulty_t = SDL_CreateTextureFromSurface(renderer, difficulty_s);
                        bot_difficulty = 15;
                        enemy_speed = 500;
                        spawn_time = 8000;
                    }
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & rightarrow1_rect)) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    if (bot_difficulty == 10) {
                        strcpy(diff, "images/srednje.bmp");
                        difficulty_s = SDL_LoadBMP(diff);
                        difficulty_t = SDL_CreateTextureFromSurface(renderer, difficulty_s);
                        bot_difficulty = 15;
                        enemy_speed = 500;
                        spawn_time = 6500;
                    }
                    else if (bot_difficulty == 15) {
                        strcpy(diff, "images/tesko.bmp");
                        difficulty_s = SDL_LoadBMP(diff);
                        difficulty_t = SDL_CreateTextureFromSurface(renderer, difficulty_s);
                        bot_difficulty = 20;
                        enemy_speed = 400;
                    }
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & leftarrow2_rect)) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    if (N == 15) {
                        strcpy(mapsize, "images/10.bmp");
                        map_size_s = SDL_LoadBMP(mapsize);
                        map_size_t = SDL_CreateTextureFromSurface(renderer, map_size_s);
                        N = 10;
                    }
                    else if (N == 20) {
                        strcpy(mapsize, "images/15.bmp");
                        map_size_s = SDL_LoadBMP(mapsize);
                        map_size_t = SDL_CreateTextureFromSurface(renderer, map_size_s);
                        N = 15;
                    }
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & rightarrow2_rect)) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    if (N == 10) {
                        strcpy(mapsize, "images/15.bmp");
                        map_size_s = SDL_LoadBMP(mapsize);
                        map_size_t = SDL_CreateTextureFromSurface(renderer, map_size_s);
                        N = 15;
                    }
                    else if (N == 15) {
                        strcpy(mapsize, "images/20.bmp");
                        map_size_s = SDL_LoadBMP(mapsize);
                        map_size_t = SDL_CreateTextureFromSurface(renderer, map_size_s);
                        N = 20;
                    }
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & leftarrow3_rect)) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    if (curr_tank[11] != '1') {
                        curr_tank[11] -= 1;
                        skin_s = SDL_LoadBMP(curr_tank);
                        skin_t = SDL_CreateTextureFromSurface(renderer, skin_s);
                    }
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & rightarrow3_rect)) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    if (curr_tank[11] != '5') {
                        curr_tank[11] += 1;
                        skin_s = SDL_LoadBMP(curr_tank);
                        skin_t = SDL_CreateTextureFromSurface(renderer, skin_s);
                    }
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & potvrdi_rect)) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    SDL_DisplayMode current;
                    SDL_GetCurrentDisplayMode(0, &current);
                    *size = current.h / SIZE * SIZE;
                    *tile_size = *size / N;

                    if (old_size != N || old_diff != bot_difficulty)
                    {
                        exist = 0;
                        set_game(map, enemies, explosion, directions, bonuses, bullets, power_up,
                            pu_started, last_pu, pu_placed_time, pu_x, pu_y, pu_placed, game,
                            dir, tile_size, renderer, window, size);
                    }
                    running = false;
                }
            }
        }
    }

    SDL_FreeSurface(difficulty_s);
    SDL_FreeSurface(map_size_s);
    SDL_FreeSurface(potvrdi_s);
    SDL_FreeSurface(skin_s);

    SDL_DestroyTexture(leftarrow_t);
    SDL_DestroyTexture(rightarrow_t);
    SDL_DestroyTexture(potvrdi_t);
    SDL_DestroyTexture(difficulty_t);
    SDL_DestroyTexture(velicina_mape_t);
    SDL_DestroyTexture(tezina_botova_t);
    SDL_DestroyTexture(map_size_t);
    SDL_DestroyTexture(skin_t);
}

void draw_menu(SDL_Renderer* renderer, int window_width, int window_height, int*** map, int*** enemies, int*** explosion, int*** directions,
    int*** bonuses, int*** bullets, int* power_up, int* pu_started, int* last_pu, int* pu_placed_time,
    int* pu_x, int* pu_y, int* pu_placed, int* game, int* dir, int* tile_size, SDL_Window* window,
    int* size, Mix_Music* music, Mix_Music* music2) {
    SDL_Cursor* hand_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_Cursor* arrow_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

    SDL_Surface* bt_s = SDL_LoadBMP("images/bt.bmp");
    SDL_Texture* bt_t = SDL_CreateTextureFromSurface(renderer, bt_s);
    SDL_FreeSurface(bt_s);
    SDL_Surface* ni_s = SDL_LoadBMP("images/novaigra.bmp");
    SDL_Texture* ni_t = SDL_CreateTextureFromSurface(renderer, ni_s);
    SDL_FreeSurface(ni_s);
    SDL_Surface* n_s = SDL_LoadBMP("images/nastavi.bmp");
    SDL_Texture* n_t = SDL_CreateTextureFromSurface(renderer, n_s);
    SDL_FreeSurface(n_s);
    SDL_Surface* pod_s = SDL_LoadBMP("images/podesavanja.bmp");
    SDL_Texture* pod_t = SDL_CreateTextureFromSurface(renderer, pod_s);
    SDL_FreeSurface(pod_s);
    SDL_Surface* rez_s = SDL_LoadBMP("images/rezultati.bmp");
    SDL_Texture* rez_t = SDL_CreateTextureFromSurface(renderer, rez_s);
    SDL_FreeSurface(rez_s);
    SDL_Surface* mus_s = SDL_LoadBMP(music_icon);
    SDL_Texture* mus_t = SDL_CreateTextureFromSurface(renderer, mus_s);

    SDL_Rect render_quad = { 0, 0, window_width, window_height };
    Uint32 start_time = SDL_GetTicks(), menu_started, menu_ended;
    Uint32 current_time;
    int fade_duration = 2000;

    SDL_Rect buttons[4];
    int button_width = 170;
    int button_height = 40;
    int button_spacing = 15;
    int button_x = (window_width - button_width) / 2;
    int first_button_y = (window_height - (5 * button_height + 3 * button_spacing));

    for (int i = 0; i < 4; i++) {
        buttons[i] = (SDL_Rect){ button_x, first_button_y + i * (button_height + button_spacing), button_width, button_height };
    }
    bool hover[4] = { false, false, false, false }, hover5 = false;
    bool running = true;
    menu_started = SDL_GetTicks();
    while (running) {
        current_time = SDL_GetTicks();
        Uint32 elapsed_time = current_time - start_time;
        if (elapsed_time > fade_duration) {
            elapsed_time = fade_duration;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        int opacity = (elapsed_time * 255) / fade_duration;
        SDL_SetTextureAlphaMod(bt_t, opacity);

        SDL_RenderCopy(renderer, bt_t, NULL, &render_quad);

        for (int i = 0; i < 4; i++) {
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 250);
            if ((hover[i] && i != 1) || (hover[i] && i == 1 && exist)) SDL_SetRenderDrawColor(renderer, 190, 190, 190, 100);
            SDL_RenderFillRect(renderer, &buttons[i]);
        }

        SDL_Rect sound = { button_height, window_height - 2 * button_height, button_height, button_height };
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 250);
        if (hover5) SDL_SetRenderDrawColor(renderer, 190, 190, 190, 100);
        SDL_RenderFillRect(renderer, &sound);

        SDL_RenderCopy(renderer, ni_t, NULL, &buttons[0]);
        SDL_RenderCopy(renderer, n_t, NULL, &buttons[1]);
        SDL_RenderCopy(renderer, pod_t, NULL, &buttons[2]);
        SDL_RenderCopy(renderer, rez_t, NULL, &buttons[3]);
        SDL_RenderCopy(renderer, mus_t, NULL, &sound);

        SDL_RenderPresent(renderer);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            }
            else if (event.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                hover[0] = SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[0]);
                hover[1] = SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[1]);
                hover[2] = SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[2]);
                hover[3] = SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[3]);
                hover5 = SDL_PointInRect(&(SDL_Point) { x, y }, & sound);

                if ((hover[0] || hover[2] || hover[3] || hover5) || (hover[1] && exist)) {
                    SDL_SetCursor(hand_cursor);
                }
                else {
                    SDL_SetCursor(arrow_cursor);
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[0])) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    if (play_music) Mix_PlayMusic(music2, 0);
                    set_game(map, enemies, explosion, directions, bonuses, bullets, power_up,
                        pu_started, last_pu, pu_placed_time, pu_x, pu_y, pu_placed, game,
                        dir, &tile_size, renderer, window, &size);
                    menu_ended = menu_lasted = SDL_GetTicks();
                    running = false;
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[1]) && exist) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    if (play_music) Mix_PlayMusic(music2, 0);
                    menu_ended = SDL_GetTicks();
                    menu_lasted += menu_ended - menu_started;
                    running = false;
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[2])) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    draw_settings(renderer, window_width, window_height, map, enemies, explosion, directions, bonuses, bullets,
                        power_up, pu_started, last_pu, pu_placed_time, pu_x, pu_y, pu_placed, game, dir, tile_size, window,
                        size, music);
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & buttons[3])) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    readHighScores(renderer, window_width, window_height, music);
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & sound)) {
                    if (play_music) {
                        Mix_PlayMusic(music, 0);
                        play_music = 0;
                        SDL_FreeSurface(mus_s);
                        SDL_DestroyTexture(mus_t);
                        strcpy(music_icon, "images/xmusic.bmp");
                        mus_s = SDL_LoadBMP(music_icon);
                        mus_t = SDL_CreateTextureFromSurface(renderer, mus_s);
                    }
                    else {
                        play_music = 1;
                        SDL_FreeSurface(mus_s);
                        SDL_DestroyTexture(mus_t);
                        strcpy(music_icon, "images/music.bmp");
                        mus_s = SDL_LoadBMP(music_icon);
                        mus_t = SDL_CreateTextureFromSurface(renderer, mus_s);
                    }
                }
            }
        }
        SDL_Delay(20);
    }
    SDL_DestroyTexture(bt_t);
    SDL_DestroyTexture(ni_t);
    SDL_DestroyTexture(n_t);
    SDL_DestroyTexture(pod_t);
    SDL_DestroyTexture(rez_t);
    SDL_FreeSurface(mus_s);
    SDL_DestroyTexture(mus_t);
}



void updateHighScore(int sc) {

    char datee[20], timee[20];

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(datee, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    sprintf(timee, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

    FILE* f = fopen("high_scores.txt", "r");
    if (!f) { printf("nece se otvori"); return; }

    int scores[10] = { 0 };
    char dates[10][20] = { {'\0'} }, times[10][20] = { {'\0'} }, linija[100];
    int i = 0;
    while (fgets(linija, 100, f)) {
        sscanf(linija, "%d %s %s\n", &scores[i], dates[i], times[i]);
        i++;
    }
    fclose(f);

    bool added = false;
    for (int k = 0; k < i; k++) {
        if (sc > scores[k]) {
            for (int j = i - 1; j > k; j--) {
                scores[j] = scores[j - 1];
                strcpy(dates[j], dates[j - 1]);
                strcpy(times[j], times[j - 1]);
            }
            scores[k] = sc;
            strcpy(dates[k], datee);
            strcpy(times[k], timee);
            added = true;
            if (i != 10) i++;
            break;
        }
    }
    if (!added && i < 10) {
        scores[i] = sc;
        strcpy(dates[i], datee);
        strcpy(times[i], timee);
        i++;
        added = true;
    }
    if (added) {
        f = fopen("high_scores.txt", "w");
        if (!f) { printf("nece se otvori"); return; }

        for (int k = 0; k < i; k++) {
            if (scores[k] == 0) break;
            fprintf(f, "%d %s %s\n", scores[k], dates[k], times[k]);
        }

        fclose(f);
    }
}

void readHighScores(SDL_Renderer* renderer, int width, int height, Mix_Music* music) {
    FILE* f = fopen("high_scores.txt", "r");
    if (!f) {
        printf("nece se otvori");
        return;
    }

    SDL_Cursor* hand_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_Cursor* arrow_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    int scores[10] = { 0 };
    char dates[10][20] = { {'\0'} }, times[10][20] = { {'\0'} }, linija[100];
    int br = 0;
    TTF_Font* font; TTF_Font* font2;
    font = TTF_OpenFont("fonts/Mantey-Black.ttf", 25);
    font2 = TTF_OpenFont("fonts/BAD GRUNGE.ttf", 70);
    SDL_Surface* text = NULL;
    SDL_Texture* text_texture = NULL;
    SDL_Color color = { 255, 255, 255 };
    SDL_Color red = { 255, 0, 0 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    text = TTF_RenderText_Solid(font2, "REZULTATI", color);
    text_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_Rect dest = { (width - text->w) / 2, 30, text->w, text->h };
    SDL_RenderCopy(renderer, text_texture, NULL, &dest);

    text = TTF_RenderText_Solid(font2, "NAZAD", color);
    text_texture = SDL_CreateTextureFromSurface(renderer, text);
    dest.x = (width - text->w) / 2;
    dest.y = height - text->h - 50;
    dest.w = text->w;
    dest.h = text->h;
    SDL_RenderCopy(renderer, text_texture, NULL, &dest);

    while (fgets(linija, 100, f)) {
        char line[100];
        sprintf(line, "%d. %s", br + 1, linija);
        text = TTF_RenderText_Solid(font, line, color);
        text_texture = SDL_CreateTextureFromSurface(renderer, text);
        SDL_Rect dest = { (width - text->w) / 2, text->h * br + width / 6, text->w, text->h };
        SDL_RenderCopy(renderer, text_texture, NULL, &dest);
        br++;
    }
    fclose(f);
    bool hover = false;
    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            }
            else if (event.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                hover = SDL_PointInRect(&(SDL_Point) { x, y }, & dest);
                if (hover) {
                    SDL_SetCursor(hand_cursor);
                    text = TTF_RenderText_Solid(font2, "NAZAD", red);
                    text_texture = SDL_CreateTextureFromSurface(renderer, text);
                    SDL_RenderCopy(renderer, text_texture, NULL, &dest);
                }
                else {
                    SDL_SetCursor(arrow_cursor);
                    text = TTF_RenderText_Solid(font2, "NAZAD", color);
                    text_texture = SDL_CreateTextureFromSurface(renderer, text);
                    SDL_RenderCopy(renderer, text_texture, NULL, &dest);
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (hover) {
                    if (play_music) Mix_PlayMusic(music, 0);
                    running = false;
                }
            }
        }
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text);

}

int main(int argc, char* argv[]) {
    initialize_enemy_timers();
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        perror("Greska u inicijalizaciji.");
        return 1;
    }
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);

    int size = current.h / SIZE * SIZE;
    int tile_size = size / N;
    int hud_width = size / 2;
    window = SDL_CreateWindow("Battle Tank", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        size + hud_width, size, SDL_WINDOW_SHOWN);
    if (!(int)window) {
        perror("Greska u kreiranju prozora.");
        SDL_Quit();
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!(int)renderer) {
        perror("Greska u kreiranju renederera");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() < 0) {
        printf("Error initializing SDL_ttf");
        return 1;
    }
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize!");
        return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error");
        return 1;
    }

    Mix_Music* music = Mix_LoadMUS("sounds/hover.wav");
    if (music == NULL) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }
    Mix_Music* shoot_sound = Mix_LoadMUS("sounds/shoot.mp3");
    if (shoot_sound == NULL) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }
    Mix_Music* exp_sound = Mix_LoadMUS("sounds/boom.mp3");
    if (exp_sound == NULL) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }
    Mix_Music* game_over_sound = Mix_LoadMUS("sounds/gameover.mp3");
    if (game_over_sound == NULL) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }
    Mix_Music* music2 = Mix_LoadMUS("sounds/start.mp3");
    if (music2 == NULL) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }
    Mix_Music* intro = Mix_LoadMUS("sounds/intro.mp3");
    if (intro == NULL) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    start_animation(renderer, size + hud_width, size, intro);

    int** map = NULL;
    int** enemies = NULL;
    int** explosion = NULL;
    int** directions = NULL;
    int** bonuses = NULL;
    int** bullets = NULL;

    int power_up = 0;
    int pu_started = 0, last_pu = 0, pu_placed_time = 0, pu_x = -1, pu_y = -1, pu_placed = 0;
    int game = 1;
    int dir = -1; // 0 levo, 1 gore, 2 desno, 3 dole
    bool left = false;

    SDL_Cursor* hand_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_Cursor* arrow_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    bool meni_hover = false, zavrsi_hover = false;
skok:
    skoci = false;
    draw_menu(renderer, size + hud_width, size, &map, &enemies, &explosion, &directions, &bonuses, &bullets,
        &power_up, &pu_started, &last_pu, &pu_placed_time, &pu_x, &pu_y, &pu_placed, &game, &dir, &tile_size, window,
        &size, music, music2);

    while (game) {

        SDL_Event event;
        int x, y;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game = 0;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    dir = 0;
                    look = 0;
                }
                else if (event.key.keysym.sym == SDLK_UP) {
                    dir = 1;
                    look = 1;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    dir = 2;
                    look = 2;
                }
                else if (event.key.keysym.sym == SDLK_DOWN) {
                    dir = 3;
                    look = 3;
                }
                else if (event.key.keysym.sym == SDLK_SPACE) {
                    Uint32 curr_time = getGameTime();
                    if (curr_time - last_shot > shoot_cooldown) {
                        if (play_music) Mix_PlayMusic(shoot_sound, 0);
                        x = tank.x / tank.w;
                        y = tank.y / tank.w;
                        bullets[x][y] = look + 1;
                        if (power_up == 5) {
                            if (look == 0) {
                                if (x > 0 && map[x - 1][y] > 1 && map[x - 1][y] < 7) {
                                    if (enemies[x - 1][y]) {
                                        kill_enemy(x - 1, y, map, enemies, renderer, explosion);
                                    }
                                    else {
                                        bullets[x - 1][y] = look + 1;
                                    }
                                }
                            }
                            else if (look == 1) {
                                if (y > 0 && map[x][y - 1] > 1 && map[x][y - 1] < 7) {
                                    if (enemies[x][y - 1]) {
                                        kill_enemy(x, y - 1, map, enemies, renderer, explosion);
                                    }
                                    else {
                                        bullets[x][y - 1] = look + 1;
                                    }
                                }
                            }
                            else if (look == 2) {
                                if (x < N - 1 && map[x][y] > 1 && map[x + 1][y] < 7) {
                                    if (enemies[x + 1][y]) {
                                        kill_enemy(x + 1, y, map, enemies, renderer, explosion);
                                    }
                                    else {
                                        bullets[x + 1][y] = look + 1;
                                    }
                                }
                            }
                            else {
                                if (y < N - 1 && map[x][y + 1] > 1 && map[x][y + 1] < 7) {
                                    if (enemies[x][y + 1]) {
                                        kill_enemy(x, y + 1, map, enemies, renderer, explosion);
                                    }
                                    else {
                                        bullets[x][y + 1] = look + 1;
                                    }
                                }
                            }
                        }
                        last_shot = curr_time;
                    }
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
                SDL_GetMouseState(&x, &y);
                meni_hover = SDL_PointInRect(&(SDL_Point) { x, y }, & meni_rect);
                zavrsi_hover = SDL_PointInRect(&(SDL_Point) { x, y }, & zavrsi_rect);

                if (meni_hover || zavrsi_hover) {
                    SDL_SetCursor(hand_cursor);
                }
                else {
                    SDL_SetCursor(arrow_cursor);
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&x, &y);
                if (SDL_PointInRect(&(SDL_Point) { x, y }, & meni_rect)) {
                    meni_hover = false, zavrsi_hover = false;
                    if (play_music) Mix_PlayMusic(music, 0);
                    exist = 1;
                    SDL_SetCursor(arrow_cursor);
                    goto skok;
                }
                else if (SDL_PointInRect(&(SDL_Point) { x, y }, & zavrsi_rect)) {
                    updateHighScore(score);
                    meni_hover = false, zavrsi_hover = false;
                    if (play_music) Mix_PlayMusic(music, 0);
                    SDL_SetCursor(arrow_cursor);
                    exist = 0;
                    menu_lasted = SDL_GetTicks();
                    goto skok;
                }
            }
        }
        generate_map(renderer, map, tile_size);
        shoot(renderer, bullets, tile_size, map, enemies, power_up, explosion);
        bool new_exp = false;
        draw_explosion(renderer, explosion, tile_size, &new_exp);
        if (play_music && new_exp) {
            Mix_PlayMusic(exp_sound, 0);
        }
        powerUp(&power_up, &pu_started, &last_pu, &pu_placed_time, map, &pu_x, &pu_y, &pu_placed, enemies, renderer);
        drawPowerUp(renderer, map, tile_size, pu_x, pu_y, pu_placed, pu_placed_time, getGameTime());
        move_tank(renderer, map, dir, enemies);

        Uint32 curr_time = getGameTime();
        if (curr_time - last_spawn > spawn_time) {
            spawn_enemies(enemies, diff);
            last_spawn = curr_time;
        }
        if (curr_time - last_move > enemy_speed && power_up != 2) {
            update_enemy_pos(renderer, bullets, tile_size, explosion, map, enemies, N, 2, tank.x, tank.y);
            last_move = curr_time;
        }
        if (tank.x / tank.w == pu_x && tank.y / tank.w == pu_y) {
            startPu(&power_up, map, &last_pu, &pu_placed, &pu_x, &pu_y, &pu_started, &pu_placed_time, enemies, renderer, explosion);
        }
        generate_enemy(renderer, enemies, tile_size);
        drawHUD(renderer, power_up, tile_size, meni_hover, zavrsi_hover, size / 2);

        if (!lives) {
            if (play_music) Mix_PlayMusic(game_over_sound, 0);
            game_over(renderer, size + hud_width, size, &map, &enemies, &explosion, &directions, &bonuses, &bullets,
                &power_up, &pu_started, &last_pu, &pu_placed_time, &pu_x, &pu_y, &pu_placed, &game, &dir, &tile_size, window,
                &size, music, music2);
        }
        if (bullets[N / 2][N - 1]) {
            if (play_music) Mix_PlayMusic(game_over_sound, 0);
            game_over(renderer, size + hud_width, size, &map, &enemies, &explosion, &directions, &bonuses, &bullets,
                &power_up, &pu_started, &last_pu, &pu_placed_time, &pu_x, &pu_y, &pu_placed, &game, &dir, &tile_size, window,
                &size, music, music2);
        }
        if (skoci) goto skok;
        dir = -1;
        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }
    free_enemy_timers();
    for (int i = 0; i < N; i++)
        free(map[i]);
    free(map);

    Mix_FreeMusic(intro);
    Mix_FreeMusic(music2);
    Mix_FreeMusic(game_over_sound);
    Mix_FreeMusic(exp_sound);
    Mix_FreeMusic(shoot_sound);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
