#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    SDL_Rect rect;
    int speed;
} Player;

typedef struct {
    SDL_Rect rect;
    int speed;
    bool active;
} FallingObject;

#define MAX_OBJECTS 5
FallingObject objects[MAX_OBJECTS];

void initObjects() {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i].rect.w = 20;
        objects[i].rect.h = 20;
        objects[i].rect.x = rand() % (800 - objects[i].rect.w);
        objects[i].rect.y = - (rand() % 600);
        objects[i].speed = 1 + rand() % 3;
        objects[i].active = true;
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Jogo de Captura", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    if (!window) {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erro ao criar renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Inicializar jogador
    Player player;
    player.rect.w = 100;
    player.rect.h = 20;
    player.rect.x = (800 - player.rect.w) / 2;
    player.rect.y = 580;
    player.speed = 70;

    initObjects();

    bool running = true;
    SDL_Event event;

    while (running) {
        // Processamento de eventos
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_a) {
                    player.rect.x -= player.speed;
                }
                if (event.key.keysym.sym == SDLK_d) {
                    player.rect.x += player.speed;
                }
            }
        }

        // Limitar o movimento do jogador à janela
        if (player.rect.x < 0) {
            player.rect.x = 0;
        }
        if (player.rect.x + player.rect.w > 800) {
            player.rect.x = 800 - player.rect.w;
        }

        bool game_over = false; // Flag para controle de fim de jogo

        // Atualizar objetos
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (objects[i].active) {
                objects[i].rect.y += objects[i].speed;

                // Verificar colisão com o jogador
                if (SDL_HasIntersection(&player.rect, &objects[i].rect)) {
                    objects[i].active = false;
                    // Incrementar pontuação (se desejar)
                }
            }

            // Verificar se o objeto caiu na parte inferior
            if (objects[i].rect.y > 600) {
                game_over = true; // Se algum objeto caiu, o jogo termina
            }

            // Reiniciar objeto se necessário
            if (!objects[i].active || objects[i].rect.y > 600) {
                objects[i].rect.x = rand() % (800 - objects[i].rect.w);
                objects[i].rect.y = - (rand() % 600);
                objects[i].speed = 2 + rand() % 5;
                objects[i].active = true;
            }
        }

        // Se o jogo estiver terminado, saia do loop
        if (game_over) {
            running = false; // Para o jogo
        }

        // Renderização
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Desenhar jogador
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &player.rect);

        // Desenhar objetos
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (objects[i].active) {
                SDL_RenderFillRect(renderer, &objects[i].rect);
            }
        }

        SDL_RenderPresent(renderer);

        // Controlar FPS
        SDL_Delay(16);
    }

    // Limpeza
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("Game Over!\n"); // Mensagem de fim de jogo
    return 0;
}
