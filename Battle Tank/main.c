#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#define SIZE 300

static char curr_tank[50] = "images/tank1.bmp";
int score = 0, lives = 2, tanks_left = 15, N = 15;
Uint32 last_shot = 0, shoot_cooldown = 500;
SDL_Rect tank;

void generate_map(SDL_Renderer *renderer, int N, int **map, int tile_size){

    SDL_Surface *surface1 = SDL_LoadBMP("images/t1.bmp");
    SDL_Surface *surface2 = SDL_LoadBMP("images/t2.bmp");
    SDL_Surface *surface3 = SDL_LoadBMP("images/t3.bmp");
    SDL_Surface *surface4 = SDL_LoadBMP("images/t4.bmp");
    SDL_Surface *surface5 = SDL_LoadBMP("images/t5.bmp");

    SDL_Texture *texture1 = SDL_CreateTextureFromSurface(renderer, surface1);
    SDL_Texture *texture2 = SDL_CreateTextureFromSurface(renderer, surface2);
    SDL_Texture *texture3 = SDL_CreateTextureFromSurface(renderer, surface3);
    SDL_Texture *texture4 = SDL_CreateTextureFromSurface(renderer, surface4);
    SDL_Texture *texture5 = SDL_CreateTextureFromSurface(renderer, surface5);

    SDL_FreeSurface(surface1);
    SDL_FreeSurface(surface2);
    SDL_FreeSurface(surface3);
    SDL_FreeSurface(surface4);
    SDL_FreeSurface(surface5);

    SDL_Rect tile[N][N];
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
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

    SDL_SetRenderDrawColor(renderer, 0x66, 0x66, 0xBB, 0xFF);
    SDL_RenderClear(renderer);
    SDL_Delay(25);
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            switch(map[i][j]){
                case 1:
                    SDL_RenderCopy(renderer, texture1, &select_tile, &tile[i][j]); //voda
                    break;
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                    SDL_RenderCopy(renderer, texture2, &select_tile, &tile[i][j]); //put
                    break;
                case 7:
                case 8:
                case 9:
                    SDL_RenderCopy(renderer, texture3, &select_tile, &tile[i][j]); //cigla
                    break;
                case 10:
                    SDL_RenderCopy(renderer, texture4, &select_tile, &tile[i][j]); //metal
                    break;
                case 11:
                    SDL_RenderCopy(renderer, texture5, &select_tile, &tile[i][j]); //baza
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

void make_map(int N, int **map){
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

void move_tank(SDL_Renderer *renderer, int N, int **map, int dir){
    SDL_Surface *tank_s = SDL_LoadBMP(curr_tank);
    SDL_Texture *tank_t = SDL_CreateTextureFromSurface(renderer, tank_s);

    int x = tank.x / tank.w;
    int y = tank.y / tank.w;

    SDL_Rect select_tile;
    select_tile.x = 0;
    select_tile.y = 0;
    select_tile.w = tank.w;
    select_tile.h = tank.w;

    switch (dir) {
        case 0: //levo
            if (x > 0 && map[x - 1][y] > 1 && map[x - 1][y] < 7) {
                tank.x = tank.x - tank.w;
            }
            break;
        case 1: //gore
            if (y > 0 && map[x][y - 1] > 1 && map[x][y - 1] < 7) {
                tank.y = tank.y - tank.w;
            }
            break;
        case 2: //desno
            if (x < N - 1 && map[x + 1][y] > 1 && map[x + 1][y] < 7) {
                tank.x = tank.x + tank.w;
            }
            break;
        case 3: //dole
            if (y < N - 1 && map[x][y + 1] > 1 && map[x][y + 1] < 7) {
                tank.y = tank.y + tank.w;
            }
            break;
        default:
            break;
    }
    SDL_RenderCopy(renderer, tank_t, &select_tile, &tank);

    SDL_FreeSurface(tank_s);
    SDL_DestroyTexture(tank_t);
}
void game_over(){
    // ovde treba da se prekine igra i ukljuci pocetni meni, prikazu high score - ovi...
    // to cemo kasnije
}
void kill_enemy(int i, int j, SDL_Renderer *renderer, int **map, int **enemies){
    score += enemies[i][j] * 100;
    enemies[i][j] = 0;
    tanks_left--;
}
void kill_player(int n, int tile_size){
    if(!lives){
        game_over();
    }
    else{
        lives--;
        tank.x = n / 2 * tile_size;
        tank.y = (n - 3) * tile_size;
    }
}

void shoot(SDL_Renderer *renderer, int N, int **bullets, int tile_size, int **map, int **enemies){
    SDL_Surface *bullet_s = SDL_LoadBMP("images/t4.bmp");
    SDL_Texture *bullet_t = SDL_CreateTextureFromSurface(renderer, bullet_s);
    SDL_FreeSurface(bullet_s);

    SDL_Rect tile[N][N];
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            tile[i][j].x = i * tile_size + tile_size / 2;
            tile[i][j].y = j * tile_size + tile_size / 2;
            tile[i][j].w = tile_size / 8;
            tile[i][j].h = tile_size / 8;
        }
    }

    SDL_Rect select_tile;
    select_tile.x = 0;
    select_tile.y = 0;
    select_tile.w = tile_size / 8;
    select_tile.h = tile_size / 8;

    SDL_Delay(50);
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            switch(bullets[i][j]){
                case 1:
                    if(i > 0 && enemies[i - 1][j]){
                        kill_enemy(i - 1, j, renderer, map, enemies);
                    }
                    else if(i - 1 ==  tank.x && j == tank.y){
                        kill_player(N, tile_size);
                    }
                    SDL_RenderCopy(renderer, bullet_t, &select_tile, &tile[i][j]);
                    if (i > 0 && !(map[i - 1][j] >= 7 && map[i - 1][j] <= 10)) {
                        if (!bullets[i - 1][j]) bullets[i - 1][j] = 1;
                        else bullets[i - 1][j] = 0;
                    }
                    else if (i > 0 && (map[i - 1][j] == 7 || map[i - 1][j] == 8 || map[i - 1][j] == 9)) {
                        map[i - 1][j] = 2;
                    }
                    bullets[i][j] = 0;
                    break;
                case 2:
                    if(j > 0 && enemies[i][j - 1]){
                        kill_enemy(i, j - 1, renderer, map, enemies);
                    }
                    else if(i ==  tank.x && j - 1 == tank.y){
                        kill_player(N, tile_size);
                    }
                    SDL_RenderCopy(renderer, bullet_t, &select_tile, &tile[i][j]);
                    if (j > 0 && !(map[i][j - 1] >= 7 && map[i][j - 1] <= 10)) {
                        if (!bullets[i][j - 1]) bullets[i][j - 1] = 2;
                        else bullets[i][j - 1] = 0;
                    }
                    else if (j > 0 && (map[i][j - 1] == 7 || map[i][j - 1] == 8 || map[i][j - 1] == 9)) {
                        map[i][j - 1] = 2;
                    }
                    bullets[i][j] = 0;
                    break;
            }
        }
    }
    for (int i = N - 1; i >= 0; i--){
        for (int j = N - 1; j >= 0; j--){
            switch(bullets[i][j]){
                case 3:
                    SDL_RenderCopy(renderer, bullet_t, &select_tile, &tile[i][j]);
                    if(i < N - 1 && enemies[i + 1][j]){
                        kill_enemy(i+1, j, renderer, map, enemies);
                    }
                    else if(i+1 ==  tank.x && j == tank.y){
                        kill_player(N, tile_size);
                    }
                    else if (i < N - 1 && !(map[i + 1][j] >= 7 && map[i + 1][j] <= 10)) {
                        if (!bullets[i + 1][j]) bullets[i + 1][j] = 3;
                        else bullets[i + 1][j] = 0;
                    }
                    else if (i < N - 1 && (map[i + 1][j] == 7 || map[i + 1][j] == 8 || map[i + 1][j] == 9)) {
                        map[i + 1][j] = 2;
                    }
                    bullets[i][j] = 0;
                    break;
                case 4:
                    SDL_RenderCopy(renderer, bullet_t, &select_tile, &tile[i][j]);
                    if(j < N - 1 && enemies[i][j+1]){
                        kill_enemy(i, j+1, renderer, map, enemies);
                    }
                    else if(i ==  tank.x && j+1 == tank.y){
                        kill_player(N, tile_size);
                    }
                    if (j < N - 1 && !(map[i][j + 1] >= 7 && map[i][j + 1] <= 10)){
                        if(!bullets[i][j + 1]) bullets[i][j + 1] = 4;
                        else bullets[i][j + 1] = 0;
                    }
                    else if (j < N - 1 && (map[i][j + 1] == 7 || map[i][j + 1] == 8 || map[i][j + 1] == 9)) {
                        map[i][j + 1] = 2;
                    }
                    bullets[i][j] = 0;
                    break;
            }
        }
    }

    SDL_DestroyTexture(bullet_t);
}

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        perror("Greska u inicijalizaciji.");
        return 1;
    }

    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);

    int size = current.h / SIZE * SIZE;
    int tile_size = size / N;
    window = SDL_CreateWindow("Battle Tank", SDL_WINDOWPOS_CENTERED,  SDL_WINDOWPOS_CENTERED,
                              size, size, SDL_WINDOW_SHOWN);
    if (!window) {
        perror("Greska u kreiranju prozora.");
        SDL_Quit();
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        perror("Greska u kreiranju renederera");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int **map;
    map = calloc(N, sizeof (int*));
    if (!map) {
        perror("Greska u alokaciji memorije za mapu.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    for (int i = 0; i < N; i++)
        map[i] = calloc(N, sizeof (int));
    make_map(N, map);

    int **enemies;
    enemies = calloc(N, sizeof (int*));
    for (int i = 0; i < N; i++)
        enemies[i] = calloc(N, sizeof (int));

    int **bullets;
    bullets = calloc(N, sizeof (int*));
    if (!bullets) {
        perror("Greska u alokaciji memorije za metkove.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    for (int i = 0; i < N; i++)
        bullets[i] = calloc(N, sizeof (int));

    tank.x = N / 2 * tile_size;
    tank.y = (N - 3) * tile_size;
    tank.w = tile_size;
    tank.h = tile_size;

    int game = 1;
    int dir = -1; //0 levo, 1 gore, 2 desno, 3 dole
    while (game){
        SDL_Event event;
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT) {
                game = 0;
            }
            else if (event.type == SDL_KEYDOWN){
                if (event.key.keysym.sym == SDLK_LEFT){
                    dir = 0;
                    strcpy(curr_tank, "images/tank0.bmp");
                }
                else if (event.key.keysym.sym == SDLK_UP){
                    dir = 1;
                    strcpy(curr_tank, "images/tank1.bmp");
                }
                else if (event.key.keysym.sym == SDLK_RIGHT){
                    dir = 2;
                    strcpy(curr_tank, "images/tank2.bmp");
                }
                else if (event.key.keysym.sym == SDLK_DOWN){
                    dir = 3;
                    strcpy(curr_tank, "images/tank3.bmp");
                }
                else if (event.key.keysym.sym == SDLK_SPACE){
                    Uint32 curr_time = SDL_GetTicks();
                    if (curr_time - last_shot > shoot_cooldown) {
                        int x = tank.x / tank.w;
                        int y = tank.y / tank.w;
                        bullets[x][y] = curr_tank[11] - '0' + 1;
                        last_shot = curr_time;
                    }
                }
            }
        }
        generate_map(renderer, N, map, tile_size);
        shoot(renderer, N, bullets, tile_size, map, enemies);
        move_tank(renderer, N, map, dir);
        dir = -1;
        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }

    for (int i = 0; i < N; i++)
        free(map[i]);
    free(map);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
