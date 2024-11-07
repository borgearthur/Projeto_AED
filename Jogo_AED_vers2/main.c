#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_SPEED 15
#define MAX_OBJECTS_ON_SCREEN 6
#define INITIAL_SPEED 2
#define SPEED_INCREMENT 1
#define OBJECTS_PER_LEVEL 5
#define MAX_TOP_SCORES 10
#define MAX_NAME_LENGTH 10
#define SCORE_FILE "scores.txt"
#define MAX_PARTICLES 100 // Definindo o número máximo de partículas

// Enum para representar os diferentes estados do jogo
typedef enum {
    MENU,
    PLAYING,
    RANKING,
    ENTER_NAME,
    GAME_OVER,
    INSTRUCTIONS // Novo estado para a tela de instruções
} GameState;

// Estrutura para armazenar nome e pontuação
typedef struct {
    char name[MAX_NAME_LENGTH + 1];
    int score;
} ScoreEntry;

ScoreEntry topScores[MAX_TOP_SCORES];

// Estrutura para o jogador
typedef struct {
    Rectangle rect;
    int speed;
} Player;

// Estrutura para os objetos que caem
typedef struct FallingObject {
    Rectangle rect;
    int speed;
    bool active;
    int type; // 0 = linha reta, 1 = zigue-zague, 2 = Power-up especial
    float offset; // Não utilizado aqui
} FallingObject;

// Estrutura para partículas
typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;
    bool active;
    float lifeTime; // Tempo de vida da partícula
} Particle;

// Variáveis globais
int currentSpeed = INITIAL_SPEED;
int objectsCaptured = 0;
int totalObjectsCaptured = 0; // Novo: Número total de objetos capturados
int score = 0;
bool gameOver = false;
int lives = 3;
unsigned int lastSpawnTime = 0;
const int SPAWN_INTERVAL = 1000;
GameState gameState = MENU;
int totalObjectsSpawned = 0; // Variável para contar o total de objetos gerados
bool hasPowerUp = false; // Nova variável para indicar se o jogador possui o Power-up
Particle particles[MAX_PARTICLES]; // Array de partículas
float flashTimer = 0.0f; // Variável para o efeito de flash
float totalTimePlayed = 0.0f; // Novo: Tempo total jogado em segundos

// Protótipos das funções
void LoadScores();
void SaveScores();
void BubbleSortScores(ScoreEntry *scores, int size);
void UpdateTopScores(char *name, int newScore);
void InitParticles();
void CreateParticleEffect(Vector2 position, Color color);
void UpdateParticles(float deltaTime);
void DrawParticles();
FallingObject createFallingObject(int totalObjectsSpawned);
void updateFallingObjects(FallingObject *objects, int *activeObjects, Player player);
void updateSpawning(FallingObject *objects, int *activeObjects);
void resetGame(Player *player, FallingObject *fallingObjects, int *activeObjects, char *playerName, int *nameIndex, Texture2D playerTexture, float playerScale);

// Função para carregar as pontuações do arquivo
void LoadScores() {
    FILE *file = fopen(SCORE_FILE, "r");
    if (file) {
        for (int i = 0; i < MAX_TOP_SCORES; i++) {
            if (fscanf(file, "%10s %d", topScores[i].name, &topScores[i].score) != 2) {
                strcpy(topScores[i].name, "N/A");
                topScores[i].score = 0;
            }
        }
        fclose(file);
    }
}

// Função para salvar as pontuações no arquivo
void SaveScores() {
    FILE *file = fopen(SCORE_FILE, "w");
    if (file) {
        for (int i = 0; i < MAX_TOP_SCORES; i++) {
            fprintf(file, "%s %d\n", topScores[i].name, topScores[i].score);
        }
        fclose(file);
    }
}

// Função de ordenação Bubble Sort para as pontuações
void BubbleSortScores(ScoreEntry *scores, int size) {
    int swaps = 0; // Contador de trocas
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (scores[j].score < scores[j + 1].score) {
                ScoreEntry temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
                swaps++; // Incrementa cada vez que uma troca é feita
            }
        }
    }
    printf("Número de trocas realizadas: %d\n", swaps);
}

// Função para atualizar as top scores
void UpdateTopScores(char *name, int newScore) {
    for (int i = 0; i < MAX_TOP_SCORES; i++) {
        if (newScore > topScores[i].score) {
            for (int j = MAX_TOP_SCORES - 1; j > i; j--) {
                topScores[j] = topScores[j - 1];
            }
            strncpy(topScores[i].name, name, MAX_NAME_LENGTH);
            topScores[i].name[MAX_NAME_LENGTH] = '\0';
            topScores[i].score = newScore;
            break;
        }
    }
    BubbleSortScores(topScores, MAX_TOP_SCORES);
}

// Função para inicializar as partículas
void InitParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
}

// Função para criar um efeito de partículas
void CreateParticleEffect(Vector2 position, Color color) {
    // Vamos criar várias partículas de uma vez para um efeito melhor
    for (int i = 0; i < 10; i++) { // Cria 10 partículas
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!particles[j].active) {
                particles[j].position = position;
                particles[j].velocity = (Vector2){
                        (float)(rand() % 201 - 100) / 100.0f,
                        (float)(rand() % 201 - 100) / 100.0f
                };
                particles[j].radius = 3; // Tamanho menor para quadrados
                particles[j].color = color;
                particles[j].active = true;
                particles[j].lifeTime = 1.0f; // Vida de 1 segundo
                break;
            }
        }
    }
}

// Função para atualizar as partículas
void UpdateParticles(float deltaTime) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].position.x += particles[i].velocity.x * deltaTime * 100;
            particles[i].position.y += particles[i].velocity.y * deltaTime * 100;
            particles[i].lifeTime -= deltaTime;

            if (particles[i].lifeTime <= 0) {
                particles[i].active = false;
            }
        }
    }
}

// Função para desenhar as partículas
void DrawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            float size = particles[i].radius * 2; // Tamanho do quadrado
            DrawRectangle(
                    particles[i].position.x - particles[i].radius,
                    particles[i].position.y - particles[i].radius,
                    size,
                    size,
                    particles[i].color
            );
        }
    }
}

// Função para criar um objeto que cai
FallingObject createFallingObject(int totalObjectsSpawned) {
    FallingObject obj;
    obj.rect.width = 20;
    obj.rect.height = 20;
    obj.rect.x = rand() % (int)(SCREEN_WIDTH - obj.rect.width);
    obj.rect.y = -20;

    if ((totalObjectsSpawned + 1) % 20 == 0) {
        obj.type = 2; // Tipo especial para o Power-up
        obj.speed = currentSpeed;
    } else if ((totalObjectsSpawned + 1) % 11 == 0) {
        obj.type = 1; // Movimento em zigue-zague
        obj.speed = INITIAL_SPEED;
    } else {
        obj.type = 0; // Movimento em linha reta
        obj.speed = currentSpeed;
    }

    obj.active = true;
    obj.offset = 0; // Não utilizado
    return obj;
}

// Função para atualizar os objetos que caem
void updateFallingObjects(FallingObject *objects, int *activeObjects, Player player) {
    for (int i = 0; i < MAX_OBJECTS_ON_SCREEN; i++) {
        if (objects[i].active) {
            if (objects[i].type == 1) {
                objects[i].speed = INITIAL_SPEED;
            }

            switch (objects[i].type) {
                case 0: // Movimento em linha reta
                    objects[i].rect.y += objects[i].speed;
                    break;
                case 1: // Movimento em zigue-zague
                    objects[i].rect.y += objects[i].speed;
                    objects[i].rect.x += 0.5f * sin(objects[i].rect.y / 100.0f);
                    break;
                case 2: // Power-up piscante
                    objects[i].rect.y += objects[i].speed;
                    break;
            }

            // Verifica se houve colisão com o jogador
            if (CheckCollisionRecs(player.rect, objects[i].rect)) {
                if (objects[i].type == 2) { // Se for o Power-up
                    hasPowerUp = true;
                } else {
                    objectsCaptured++;
                    totalObjectsCaptured++; // Incrementa o total de objetos capturados
                    score += 10;

                    // Cria efeito de partículas ao capturar um objeto
                    CreateParticleEffect((Vector2){
                            objects[i].rect.x + objects[i].rect.width / 2,
                            objects[i].rect.y + objects[i].rect.height / 2
                    }, GREEN);
                }
                objects[i].active = false;
                (*activeObjects)--;
                if (objectsCaptured % OBJECTS_PER_LEVEL == 0) {
                    currentSpeed += SPEED_INCREMENT;
                }
            } else if (objects[i].rect.y > SCREEN_HEIGHT) {
                objects[i].active = false;
                (*activeObjects)--;
                lives--;

                // Cria efeito de partículas ao perder um objeto
                CreateParticleEffect((Vector2){
                        objects[i].rect.x + objects[i].rect.width / 2,
                        SCREEN_HEIGHT
                }, RED);

                // Inicia o efeito de flash
                flashTimer = 0.2f; // Duração do flash em segundos

                if (lives <= 0) {
                    gameState = GAME_OVER;
                }
            }
        }
    }
}

// Função para atualizar o spawn dos objetos que caem
void updateSpawning(FallingObject *objects, int *activeObjects) {
    unsigned int currentTime = (unsigned int)(GetTime() * 1000);
    if ((currentTime - lastSpawnTime) >= SPAWN_INTERVAL && *activeObjects < MAX_OBJECTS_ON_SCREEN) {
        for (int i = 0; i < MAX_OBJECTS_ON_SCREEN; i++) {
            if (!objects[i].active) {
                objects[i] = createFallingObject(totalObjectsSpawned);
                objects[i].active = true;
                (*activeObjects)++;
                lastSpawnTime = currentTime;
                totalObjectsSpawned++;
                break;
            }
        }
    }
}

// Função para reiniciar o jogo
void resetGame(Player *player, FallingObject *fallingObjects, int *activeObjects, char *playerName, int *nameIndex, Texture2D playerTexture, float playerScale) {
    score = 0;
    lives = 3;
    currentSpeed = INITIAL_SPEED;
    objectsCaptured = 0;
    totalObjectsCaptured = 0; // Reinicia o total de objetos capturados
    totalTimePlayed = 0.0f; // Reinicia o tempo total jogado
    *activeObjects = 0;
    *nameIndex = 0;
    playerName[0] = '\0';
    player->rect.width = playerTexture.width * playerScale;
    player->rect.height = playerTexture.height * playerScale;
    player->rect.x = (SCREEN_WIDTH - player->rect.width) / 2;
    player->rect.y = SCREEN_HEIGHT - player->rect.height; // Posiciona a base da imagem na base da tela
    player->speed = PLAYER_SPEED;
    for (int i = 0; i < MAX_OBJECTS_ON_SCREEN; i++) {
        fallingObjects[i].active = false;
    }
    totalObjectsSpawned = 0;
    hasPowerUp = false;
    InitParticles(); // Reinicia o sistema de partículas
    flashTimer = 0.0f; // Reinicia o flash
}

int main() {
    srand((unsigned int)time(NULL));
    LoadScores();

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de Captura com Raylib");
    SetExitKey(0);
    SetTargetFPS(60);

    InitParticles(); // Inicializa partículas

    // Carregando texturas
    Texture2D menuBackground = LoadTexture("marco_zero_menu.png");
    Texture2D gameBackground = LoadTexture("marco_zero_fotoDeFundo.png");
    Texture2D playerTexture = LoadTexture("turista.png"); // Carregar a textura do jogador

    float playerScale = 0.2f; // Fator de escala para o jogador (ajuste conforme necessário)

    Player player;
    player.rect.width = playerTexture.width * playerScale;
    player.rect.height = playerTexture.height * playerScale;
    player.rect.x = (SCREEN_WIDTH - player.rect.width) / 2;
    player.rect.y = SCREEN_HEIGHT - player.rect.height; // Posiciona a base da imagem na base da tela
    player.speed = PLAYER_SPEED;

    FallingObject fallingObjects[MAX_OBJECTS_ON_SCREEN] = { 0 };
    int activeObjects = 0;
    char playerName[MAX_NAME_LENGTH + 1] = "";
    int nameIndex = 0;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        switch (gameState) {
            case MENU:
                if (IsKeyPressed(KEY_ONE)) {
                    resetGame(&player, fallingObjects, &activeObjects, playerName, &nameIndex, playerTexture, playerScale);
                    gameState = PLAYING;
                } else if (IsKeyPressed(KEY_TWO)) {
                    gameState = INSTRUCTIONS; // Vai para a tela de instruções
                } else if (IsKeyPressed(KEY_THREE)) {
                    gameState = RANKING;
                } else if (IsKeyPressed(KEY_FOUR)) {
                    CloseWindow();
                    return 0;
                }
                break;

            case INSTRUCTIONS:
                if (IsKeyPressed(KEY_R)) {
                    gameState = MENU; // Volta para o menu principal
                }
                break;

            case PLAYING:
                if (IsKeyDown(KEY_LEFT)) player.rect.x -= player.speed;
                if (IsKeyDown(KEY_RIGHT)) player.rect.x += player.speed;

                // Limita o jogador dentro dos limites da tela
                if (player.rect.x < 0)
                    player.rect.x = 0;
                else if (player.rect.x + player.rect.width > SCREEN_WIDTH)
                    player.rect.x = SCREEN_WIDTH - player.rect.width;

                updateFallingObjects(fallingObjects, &activeObjects, player);
                updateSpawning(fallingObjects, &activeObjects);

                UpdateParticles(deltaTime);

                totalTimePlayed += deltaTime; // Incrementa o tempo total jogado

                // Atualiza o flashTimer
                if (flashTimer > 0.0f) {
                    flashTimer -= deltaTime;
                    if (flashTimer < 0.0f) {
                        flashTimer = 0.0f;
                    }
                }

                if (lives <= 0) {
                    gameState = GAME_OVER;
                }

                if (IsKeyPressed(KEY_SPACE) && hasPowerUp) {
                    for (int i = 0; i < MAX_OBJECTS_ON_SCREEN; i++) {
                        if (fallingObjects[i].active) {
                            fallingObjects[i].active = false;

                            // Cria efeito de partículas ao usar o Power-up
                            CreateParticleEffect((Vector2){
                                    fallingObjects[i].rect.x + fallingObjects[i].rect.width / 2,
                                    fallingObjects[i].rect.y + fallingObjects[i].rect.height / 2
                            }, ORANGE);
                        }
                    }
                    hasPowerUp = false;
                }
                break;

            case RANKING:
                if (IsKeyPressed(KEY_R)) {
                    gameState = MENU;
                }
                break;

            case ENTER_NAME:
            {
                int key = GetCharPressed();
                if ((key >= 32) && (key <= 125) && nameIndex < MAX_NAME_LENGTH) {
                    playerName[nameIndex++] = (char)key;
                    playerName[nameIndex] = '\0';
                }
                if (IsKeyPressed(KEY_BACKSPACE) && nameIndex > 0) {
                    playerName[--nameIndex] = '\0';
                }
                if (IsKeyPressed(KEY_ENTER) && nameIndex > 0) {
                    UpdateTopScores(playerName, score);
                    SaveScores();
                    gameState = RANKING;
                }
            }
                break;

            case GAME_OVER:
                if (IsKeyPressed(KEY_ENTER)) {
                    if (score > topScores[MAX_TOP_SCORES - 1].score) {
                        gameState = ENTER_NAME;
                    } else {
                        gameState = MENU;
                        resetGame(&player, fallingObjects, &activeObjects, playerName, &nameIndex, playerTexture, playerScale);
                    }
                }
                break;
        }

        BeginDrawing();

        switch (gameState) {
            case MENU:
                ClearBackground(RAYWHITE);
                DrawTexture(menuBackground, 0, 0, WHITE);
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, 0.4f));
                DrawText("Bem-vindo ao Jogo!", SCREEN_WIDTH / 2 - MeasureText("Bem-vindo ao Jogo!", 40) / 2, 100, 40, BLACK);
                DrawText("1. Iniciar Jogo", SCREEN_WIDTH / 2 - MeasureText("1. Iniciar Jogo", 20) / 2, 200, 20, BLACK);
                DrawText("2. Instruções", SCREEN_WIDTH / 2 - MeasureText("2. Instruções", 20) / 2, 250, 20, BLACK);
                DrawText("3. Ranking de Pontuações", SCREEN_WIDTH / 2 - MeasureText("3. Ranking de Pontuações", 20) / 2, 300, 20, BLACK);
                DrawText("4. Sair do Jogo", SCREEN_WIDTH / 2 - MeasureText("4. Sair do Jogo", 20) / 2, 350, 20, BLACK);
                DrawText("Escolha uma opção com o número correspondente", SCREEN_WIDTH / 2 - MeasureText("Escolha uma opção com o número correspondente", 20) / 2, 450, 20, BLACK);
                break;

            case INSTRUCTIONS:
                ClearBackground(RAYWHITE);
                DrawText("Instruções do Jogo", SCREEN_WIDTH / 2 - MeasureText("Instruções do Jogo", 30) / 2, 50, 30, BLACK);
                DrawText("Objetivo:", 100, 120, 20, DARKGRAY);
                DrawText("- Capturar os objetos que caem para ganhar pontos.", 120, 150, 18, GRAY);
                DrawText("- Evite deixar os objetos caírem no chão, você perderá vidas.", 120, 180, 18, GRAY);
                DrawText("Controles:", 100, 230, 20, DARKGRAY);
                DrawText("- Use as setas ESQUERDA e DIREITA para mover o jogador.", 120, 260, 18, GRAY);
                DrawText("- Pressione ESPAÇO para usar o Power-up quando disponível.", 120, 290, 18, GRAY);
                DrawText("Power-ups:", 100, 340, 20, DARKGRAY);
                DrawText("- Capture o objeto especial para ganhar um Power-up.", 120, 370, 18, GRAY);
                DrawText("- O Power-up elimina todos os objetos na tela quando usado.", 120, 400, 18, GRAY);
                DrawText("Pressione 'R' para retornar ao menu principal.", SCREEN_WIDTH / 2 - MeasureText("Pressione 'R' para retornar ao menu principal.", 18) / 2, SCREEN_HEIGHT - 50, 18, DARKGRAY);
                break;

            case PLAYING:
                ClearBackground(RAYWHITE);
                DrawTexture(gameBackground, 0, 0, WHITE);

                // Efeito de flash
                if (flashTimer > 0.0f) {
                    float flashIntensity = flashTimer / 0.2f; // Normaliza a intensidade
                    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(RED, flashIntensity * 0.5f));
                }

                // Desenhar o jogador com a imagem na posição correta
                DrawTextureEx(playerTexture, (Vector2){ player.rect.x, player.rect.y }, 0.0f, playerScale, WHITE);

                // Desenhar os objetos que caem
                for (int i = 0; i < MAX_OBJECTS_ON_SCREEN; i++) {
                    if (fallingObjects[i].active) {
                        Color objectColor = RED;
                        if (fallingObjects[i].type == 2) {
                            objectColor = ((int)(GetTime() * 10) % 2 == 0) ? YELLOW : ORANGE;
                        }
                        DrawRectangleRec(fallingObjects[i].rect, objectColor);
                    }
                }

                DrawParticles(); // Desenha partículas

                // Desenhar a pontuação e vidas
                DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
                DrawText(TextFormat("Lives: %d", lives), SCREEN_WIDTH - 120, 10, 20, RED);
                break;

            case RANKING:
                ClearBackground(BLACK);
                DrawText("TOP 10 Scores:", SCREEN_WIDTH / 2 - MeasureText("TOP 10 Scores:", 30) / 2, 50, 30, YELLOW);
                for (int i = 0; i < MAX_TOP_SCORES; i++) {
                    DrawText(TextFormat("%d. %s - %d", i + 1, topScores[i].name, topScores[i].score),
                             SCREEN_WIDTH / 2 - MeasureText(TextFormat("%d. %s - %d", i + 1, topScores[i].name, topScores[i].score), 20) / 2,
                             100 + (i * 30), 20, WHITE);
                }
                DrawText("Pressione R para voltar ao menu", SCREEN_WIDTH / 2 - MeasureText("Pressione R para voltar ao menu", 20) / 2,
                         SCREEN_HEIGHT - 50, 20, GRAY);
                break;

            case ENTER_NAME:
                ClearBackground(BLACK);
                DrawText("Digite seu Nome:", SCREEN_WIDTH / 2 - MeasureText("Digite seu Nome:", 20) / 2, SCREEN_HEIGHT / 2 - 80, 20, YELLOW);
                DrawText(playerName, SCREEN_WIDTH / 2 - MeasureText(playerName, 20) / 2, SCREEN_HEIGHT / 2 - 50, 20, WHITE);
                DrawText("Pressione ENTER para confirmar", SCREEN_WIDTH / 2 - MeasureText("Pressione ENTER para confirmar", 20) / 2, SCREEN_HEIGHT / 2 - 10, 20, GRAY);
                if (((int)(GetTime() * 2) % 2) == 0) {
                    DrawText("_", SCREEN_WIDTH / 2 + MeasureText(playerName, 20) / 2, SCREEN_HEIGHT / 2 - 50, 20, WHITE);
                }
                break;

            case GAME_OVER:
                ClearBackground(BLACK);
                DrawText("Game Over!", SCREEN_WIDTH / 2 - MeasureText("Game Over!", 40) / 2, SCREEN_HEIGHT / 2 - 150, 40, RED);
                DrawText(TextFormat("Score: %d", score), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Score: %d", score), 20) / 2, SCREEN_HEIGHT / 2 - 100, 20, WHITE);
                DrawText(TextFormat("Tempo Jogando: %.2f segundos", totalTimePlayed), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Tempo Jogando: %.2f segundos", totalTimePlayed), 20) / 2, SCREEN_HEIGHT / 2 - 70, 20, WHITE);
                DrawText(TextFormat("Objetos Capturados: %d", totalObjectsCaptured), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Objetos Capturados: %d", totalObjectsCaptured), 20) / 2, SCREEN_HEIGHT / 2 - 40, 20, WHITE);
                DrawText("Pressione ENTER para continuar", SCREEN_WIDTH / 2 - MeasureText("Pressione ENTER para continuar", 20) / 2, SCREEN_HEIGHT / 2 + 20, 20, YELLOW);
                break;
        }

        EndDrawing();
    }

    // Limpeza dos recursos
    UnloadTexture(menuBackground);
    UnloadTexture(gameBackground);
    UnloadTexture(playerTexture); // Limpa a textura do jogador
    CloseWindow();
    return 0;
}
