#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <stdio.h>

#define SIZE 300

static char curr_tank[50] = "images/tank1.bmp";
int score = 0, lives = 2, tanks_left = 15, N = 15;
Uint32 last_shot = 0, shoot_cooldown = 500;
Uint32 last_move = 0, enemy_speed = 500;
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

void move_tank(SDL_Renderer *renderer, int N, int **map, int dir, int **enemies){
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
            if (x > 0 && map[x - 1][y] > 1 && map[x - 1][y] < 7 && !enemies[x - 1][y]) {
                tank.x = tank.x - tank.w;
            }
            break;
        case 1: //gore
            if (y > 0 && map[x][y - 1] > 1 && map[x][y - 1] < 7 && !enemies[x][y - 1]) {
                tank.y = tank.y - tank.w;
            }
            break;
        case 2: //desno
            if (x < N - 1 && map[x + 1][y] > 1 && map[x + 1][y] < 7 && !enemies[x + 1][y]) {
                tank.x = tank.x + tank.w;
            }
            break;
        case 3: //dole
            if (y < N - 1 && map[x][y + 1] > 1 && map[x][y + 1] < 7 && !enemies[x][y + 1]) {
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
void kill_enemy(int i, int j, int **map, int **enemies, SDL_Renderer *renderer){
    score += enemies[i][j] * 100;
    enemies[i][j] = 0;
    tanks_left--;

    SDL_Surface *explosion_s = SDL_LoadBMP("images/explosion.bmp");
    SDL_Texture *explosion_t = SDL_CreateTextureFromSurface(renderer, explosion_s);
    SDL_FreeSurface(explosion_s);
    SDL_Rect explosion_rect = {i * tank.w, j * tank.w, tank.w, tank.w};
    SDL_RenderCopy(renderer, explosion_t, NULL, &explosion_rect);
    SDL_DestroyTexture(explosion_t);
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

    SDL_Surface *explosion_s = SDL_LoadBMP("images/explosion.bmp");
    SDL_Texture *explosion_t = SDL_CreateTextureFromSurface(renderer, explosion_s);
    SDL_FreeSurface(explosion_s);

    SDL_Surface *explosion2_s = SDL_LoadBMP("images/explosion2.bmp");
    SDL_Texture *explosion2_t = SDL_CreateTextureFromSurface(renderer, explosion2_s);
    SDL_FreeSurface(explosion2_s);

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

    SDL_Rect explosion_rect;

    SDL_Delay(50);
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            switch(bullets[i][j]){
                case 1:
                    if(i > 0 && enemies[i - 1][j]){
                        kill_enemy(i - 1, j, map, enemies, renderer);
                        bullets[i - 1][j] = 1;
                    }
                    else if(i - 1 ==  tank.x && j == tank.y){
                        kill_player(N, tile_size);
                    }
                    SDL_RenderCopy(renderer, bullet_t, &select_tile, &tile[i][j]);
                    explosion_rect.x = tile[i - 1][j].x - tile_size / 2;
                    explosion_rect.y = tile[i - 1][j].y - tile_size / 2;
                    explosion_rect.w = tile_size;
                    explosion_rect.h = tile_size;
                    if (i > 0 && !(map[i - 1][j] >= 7 && map[i - 1][j] <= 10)) {
                        if (!bullets[i - 1][j]) bullets[i - 1][j] = 1;
                        else bullets[i - 1][j] = 0;
                    }
                    else if (i > 0 && (map[i - 1][j] == 7 || map[i - 1][j] == 8 || map[i - 1][j] == 9)) {
                        map[i - 1][j] = 2;
                        SDL_RenderCopy(renderer, explosion_t, NULL, &explosion_rect);
                        SDL_Delay(50);
                    }
                    else if (i > 0 && !(map[i - 1][j] == 7 || map[i - 1][j] == 8 || map[i - 1][j] == 9)) {
                        SDL_RenderCopy(renderer, explosion2_t, NULL, &explosion_rect);
                        SDL_Delay(50);
                    }
                    bullets[i][j] = 0;
                    break;
                case 2:
                    if(j > 0 && enemies[i][j - 1]){
                        kill_enemy(i, j - 1, map, enemies, renderer);
                        bullets[i][j - 1] = 1;
                    }
                    else if(i ==  tank.x && j - 1 == tank.y){
                        kill_player(N, tile_size);
                    }
                    SDL_RenderCopy(renderer, bullet_t, &select_tile, &tile[i][j]);
                    explosion_rect.x = tile[i][j - 1].x - tile_size / 2;
                    explosion_rect.y = tile[i][j - 1].y - tile_size / 2;
                    explosion_rect.w = tile_size;
                    explosion_rect.h = tile_size;
                    if (j > 0 && !(map[i][j - 1] >= 7 && map[i][j - 1] <= 10)) {
                        if (!bullets[i][j - 1]) bullets[i][j - 1] = 2;
                        else bullets[i][j - 1] = 0;
                    }
                    else if (j > 0 && (map[i][j - 1] == 7 || map[i][j - 1] == 8 || map[i][j - 1] == 9)) {
                        map[i][j - 1] = 2;
                        SDL_RenderCopy(renderer, explosion_t, NULL, &explosion_rect);
                        SDL_Delay(50);
                    }
                    else if (j > 0 && !(map[i][j - 1] == 7 || map[i][j - 1] == 8 || map[i][j - 1] == 9)) {
                        SDL_RenderCopy(renderer, explosion2_t, NULL, &explosion_rect);
                        SDL_Delay(50);
                    }
                    bullets[i][j] = 0;
                    break;
                default:
                    break;
            }
        }
    }
    for (int i = N - 1; i >= 0; i--){
        for (int j = N - 1; j >= 0; j--){
            switch(bullets[i][j]){
                case 3:
                    if(i < N - 1 && enemies[i + 1][j]){
                        kill_enemy(i+1, j, map, enemies, renderer);
                        bullets[i + 1][j] = 1;
                    }
                    else if(i+1 ==  tank.x && j == tank.y){
                        kill_player(N, tile_size);
                    }
                    SDL_RenderCopy(renderer, bullet_t, &select_tile, &tile[i][j]);
                    explosion_rect.x = tile[i + 1][j].x - tile_size / 2;
                    explosion_rect.y = tile[i + 1][j].y - tile_size / 2;
                    explosion_rect.w = tile_size;
                    explosion_rect.h = tile_size;
                    if (i < N - 1 && !(map[i + 1][j] >= 7 && map[i + 1][j] <= 10)) {
                        if (!bullets[i + 1][j]) bullets[i + 1][j] = 3;
                        else bullets[i + 1][j] = 0;
                    }
                    else if (i < N - 1 && (map[i + 1][j] == 7 || map[i + 1][j] == 8 || map[i + 1][j] == 9)) {
                        map[i + 1][j] = 2;
                        SDL_RenderCopy(renderer, explosion_t, NULL, &explosion_rect);
                        SDL_Delay(50);
                    }
                    else if (i < N - 1 && !(map[i + 1][j] == 7 || map[i + 1][j] == 8 || map[i + 1][j] == 9)) {
                        SDL_RenderCopy(renderer, explosion2_t, NULL, &explosion_rect);
                        SDL_Delay(50);
                    }
                    bullets[i][j] = 0;
                    break;
                case 4:
                    if(j < N - 1 && enemies[i][j+1]){
                        kill_enemy(i, j+1, map, enemies, renderer);
                        bullets[i][j + 1] = 1;
                    }
                    else if(i ==  tank.x && j+1 == tank.y){
                        kill_player(N, tile_size);
                    }
                    SDL_RenderCopy(renderer, bullet_t, &select_tile, &tile[i][j]);
                    explosion_rect.x = tile[i][j + 1].x - tile_size / 2;
                    explosion_rect.y = tile[i][j + 1].y - tile_size / 2;
                    explosion_rect.w = tile_size;
                    explosion_rect.h = tile_size;
                    if (j < N - 1 && !(map[i][j + 1] >= 7 && map[i][j + 1] <= 10)){
                        if(!bullets[i][j + 1]) bullets[i][j + 1] = 4;
                        else bullets[i][j + 1] = 0;
                    }
                    else if (j < N - 1 && (map[i][j + 1] == 7 || map[i][j + 1] == 8 || map[i][j + 1] == 9)) {
                        map[i][j + 1] = 2;
                        SDL_RenderCopy(renderer, explosion_t, NULL, &explosion_rect);
                        SDL_Delay(50);
                    }
                    else if (j < N - 1 && !(map[i][j + 1] == 7 || map[i][j + 1] == 8 || map[i][j + 1] == 9)) {
                        SDL_RenderCopy(renderer, explosion2_t, NULL, &explosion_rect);
                        SDL_Delay(50);
                    }
                    bullets[i][j] = 0;
                    break;
                default:
                    break;
            }
        }
    }

    SDL_DestroyTexture(explosion_t);
    SDL_DestroyTexture(bullet_t);
}

void generate_enemy(SDL_Renderer *renderer, int N, int **enemies, int tile_size) {
    // 1 - obican gleda levo, 2 - obican gleda napred, 3 - obican gleda desno, 4 - obican gleda dole
    // 5 - specijalni gleda levo, 6 - specijalni gleda napred, 7 - specijalni gleda desno, 8 - specijalni gleda dole

    SDL_Surface *enemy0 = SDL_LoadBMP("images/enemy0.bmp");
    SDL_Surface *enemy1 = SDL_LoadBMP("images/enemy1.bmp");
    SDL_Surface *enemy2 = SDL_LoadBMP("images/enemy2.bmp");
    SDL_Surface *enemy3 = SDL_LoadBMP("images/enemy3.bmp");
    SDL_Surface *spec0 = SDL_LoadBMP("images/spec0.bmp");
    SDL_Surface *spec1 = SDL_LoadBMP("images/spec1.bmp");
    SDL_Surface *spec2 = SDL_LoadBMP("images/spec2.bmp");
    SDL_Surface *spec3 = SDL_LoadBMP("images/spec3.bmp");

    SDL_Texture *enemy0_t = SDL_CreateTextureFromSurface(renderer, enemy0);
    SDL_Texture *enemy1_t = SDL_CreateTextureFromSurface(renderer, enemy1);
    SDL_Texture *enemy2_t = SDL_CreateTextureFromSurface(renderer, enemy2);
    SDL_Texture *enemy3_t = SDL_CreateTextureFromSurface(renderer, enemy3);
    SDL_Texture *spec0_t= SDL_CreateTextureFromSurface(renderer, spec0);
    SDL_Texture *spec1_t= SDL_CreateTextureFromSurface(renderer, spec1);
    SDL_Texture *spec2_t= SDL_CreateTextureFromSurface(renderer, spec2);
    SDL_Texture *spec3_t= SDL_CreateTextureFromSurface(renderer, spec3);

    SDL_FreeSurface(enemy0);
    SDL_FreeSurface(enemy1);
    SDL_FreeSurface(enemy2);
    SDL_FreeSurface(enemy3);
    SDL_FreeSurface(spec0);
    SDL_FreeSurface(spec1);
    SDL_FreeSurface(spec2);
    SDL_FreeSurface(spec3);

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


    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            switch(enemies[i][j]){
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

void powerUp(int *power_up, int *pu_started, int *last_pu, int *pu_placed_time, int **map, int *pu_x, int *pu_y, int *pu_placed, int **enemies, SDL_Renderer *renderer){
    if(*power_up){
        switch (*power_up) {
            case 1: // ubij sve neprijatelje
                for(int i = 0;i<N;i++){
                    for(int j = 0;j<N;j++){
                        if(enemies[i][j]){
                            kill_enemy(i, j, map, enemies, renderer);
                        }
                    }
                }
                *power_up = 0;
                *pu_started = 0;
                *last_pu = (int)(SDL_GetTicks());
                break;
            case 2: // zaledi neprijatelje
                if(SDL_GetTicks() > *pu_started + 15000){
                    *pu_started = 0;
                    *last_pu = (int)(SDL_GetTicks());
                    *power_up = 0;
                }
                break;
            case 3: // stit
                if(SDL_GetTicks() > *pu_started + 10000){
                    *pu_started = 0;
                    *last_pu = (int)(SDL_GetTicks());
                    *power_up = 0;
                }
                break;
            case 4: // samo dodaje poene
                score += 300;
                *power_up = 0;
                *pu_started = 0;
                *last_pu = (int)(SDL_GetTicks());
                break;
            case 5:
                *power_up = 0;
                *pu_started = 0;
                *last_pu = (int)(SDL_GetTicks());
                break;
            case 6:
                *power_up = 0;
                *pu_started = 0;
                *last_pu = (int)(SDL_GetTicks());
                break;
            case 7:
                *power_up = 0;
                *pu_started = 0;
                *last_pu = (int)(SDL_GetTicks());
                break;
            default:
                *power_up = 0;
                *pu_started = 0;
                *last_pu = (int)(SDL_GetTicks());
                break;
        }
    }
    else if(*pu_placed){
        if(SDL_GetTicks() > *pu_placed_time + 10000){ // skloni mocku
            *last_pu = (int)(SDL_GetTicks());
            *pu_placed_time = 0;
            *pu_y = -1;
            *pu_x = -1;
            *pu_placed = 0;
        }
    }
    else if(SDL_GetTicks() > *last_pu + 10000){ // stavi mocku
        srand(time(NULL));
        *pu_placed = rand() % 8 + 1;
        *pu_x = rand() % N, *pu_y = rand() % N;
        while(map[*pu_x][*pu_y] < 2 || map[*pu_x][*pu_y] > 6 || enemies[*pu_x][*pu_y] || tank.x == *pu_y * tank.w || tank.y == *pu_x * tank.w){
            *pu_x = rand() % N, *pu_y = rand() % N;
        }
        *pu_placed_time = (int)(SDL_GetTicks());
    }
}

void drawPowerUp(SDL_Renderer *renderer, int **map, int tile_size, int pu_x, int pu_y,int pu_placed){ // nacrta mocku na tablu
    // 1 - bomba, 2 - sat, 3 - stit, 4 - zvezda
    // 5 - dve zvezde, 6 - tri zvezde, 7 - lopata, 8 - srce
    if(pu_placed){
        SDL_Surface *pu_s = NULL;
        if(pu_placed == 1) {
            pu_s = SDL_LoadBMP("images/bomb.bmp");
        }
        else if(pu_placed == 2) {
            pu_s = SDL_LoadBMP("images/time.bmp");
        }
        else if(pu_placed == 3) {
            pu_s = SDL_LoadBMP("images/shield.bmp");
        }
        else if(pu_placed == 4) {
            pu_s = SDL_LoadBMP("images/one.bmp");
        }
        else if(pu_placed == 5) {
            pu_s = SDL_LoadBMP("images/two.bmp");
        }
        else if(pu_placed == 6) {
            pu_s = SDL_LoadBMP("images/three.bmp");
        }
        else if(pu_placed == 7) {
            pu_s = SDL_LoadBMP("images/shovel.bmp");
        }
        else {
            pu_s = SDL_LoadBMP("images/life.bmp");
        }
        SDL_Texture *pu_t = SDL_CreateTextureFromSurface(renderer, pu_s);
        SDL_FreeSurface(pu_s);
        SDL_Rect rect = {pu_x * tile_size, pu_y * tile_size, tile_size, tile_size};
        SDL_RenderCopy(renderer, pu_t, NULL, &rect);
        SDL_DestroyTexture(pu_t);
    }
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

    int **map = calloc(N, sizeof (int*));
    if (!(int)map) {
        perror("Greska u alokaciji memorije za mapu.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    for (int i = 0; i < N; i++)
        map[i] = calloc(N, sizeof (int));
    make_map(N, map);

    int **enemies = calloc(N, sizeof (int*));
    for (int i = 0; i < N; i++)
        enemies[i] = calloc(N, sizeof (int));

    int **bonuses = calloc(N, sizeof (int*));
    for (int i = 0; i < N; i++)
        bonuses[i] = calloc(N, sizeof (int));

    int **bullets = calloc(N, sizeof (int*));
    if (!(int)bullets) {
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

    int power_up = 0;
    int pu_started = 0, last_pu = 0, pu_placed_time = 0, pu_x = -1, pu_y = -1, pu_placed = 0;

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
        powerUp(&power_up, &pu_started, &last_pu, &pu_placed_time, map, &pu_x, &pu_y, &pu_placed, enemies, renderer);
        drawPowerUp(renderer, map, tile_size, pu_x, pu_y, pu_placed);
        move_tank(renderer, N, map, dir, enemies);
        Uint32 curr_time = SDL_GetTicks();
        if (curr_time - last_move > enemy_speed) {
            // FUNKCIJA ZA POMERANJE PROTIVNIKA
            last_move = curr_time;
        }
        if(tank.x / tank.w == pu_x && tank.y / tank.w == pu_y){
            power_up = pu_placed;
            last_pu = (int)(SDL_GetTicks());
            pu_started = (int)(SDL_GetTicks());
            pu_placed_time = 0;
            pu_x = -1;
            pu_y = -1;
            pu_placed = 0;
        }
        generate_enemy(renderer, N, enemies, tile_size);
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
