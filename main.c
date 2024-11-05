#include "../include/raylib.h"
#include <stdio.h>

typedef struct {
    int x;
    int y;
    int valor; 
}Obj;

int main(){
    int larguraTela = 1280, alturaTela = 720;
        
    InitWindow(larguraTela, alturaTela, "Lugares no Recife");
    SetTargetFPS(60);
    
    Texture2D backgroundImage = LoadTexture("background.png");
}