#include <SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define SIZE 600

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

void move_tank(SDL_Renderer *renderer, int N, SDL_Rect *tank, int **map, int dir){
    SDL_Surface *tank_s = SDL_LoadBMP("images/t5.bmp");
    SDL_Texture *tank_t = SDL_CreateTextureFromSurface(renderer, tank_s);

    int x = tank->x / tank->w;
    int y = tank->y / tank->w;

    SDL_Rect select_tile;
    select_tile.x = 0;
    select_tile.y = 0;
    select_tile.w = tank->w;
    select_tile.h = tank->w;

    switch (dir) {
        case 0: //levo
            if (x > 0 && map[x - 1][y] > 1 && map[x - 1][y] < 7) {
                tank->x = tank->x - tank->w;
            }
            break;
        case 1: //gore
            if (y > 0 && map[x][y - 1] > 1 && map[x][y - 1] < 7) {
                tank->y = tank->y - tank->w;
            }
            break;
        case 2: //desno
            if (x < N - 1 && map[x + 1][y] > 1 && map[x + 1][y] < 7) {
                tank->x = tank->x + tank->w;
            }
            break;
        case 3: //dole
            if (y < N - 1 && map[x][y + 1] > 1 && map[x][y + 1] < 7) {
                tank->y = tank->y + tank->w;
            }
            break;
        default:
            break;
    }

    SDL_RenderCopy(renderer, tank_t, &select_tile, tank);

    SDL_FreeSurface(tank_s);
    SDL_DestroyTexture(tank_t);
}

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        perror("Greska u inicijalizaciji.");
        return 1;
    }

    int N = 10; //dimenzije mape koje treba da se unose preko menija

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

    SDL_Rect tank;
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
                }
                else if (event.key.keysym.sym == SDLK_UP){
                    dir = 1;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT){
                    dir = 2;
                }
                else{
                    dir = 3;
                }
            }
        }
        generate_map(renderer, N, map, tile_size);
        move_tank(renderer, N, &tank, map, dir);
        dir = -1;
        SDL_RenderPresent(renderer);
        SDL_Delay(25);
    }

    for (int i = 0; i < N; i++)
        free(map[i]);
    free(map);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
