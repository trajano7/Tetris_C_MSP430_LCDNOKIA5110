#ifndef GAMECONTROL_H_
#define GAMECONTROL_H_
#include "gameInfo.h"

#define TRUE 1
#define FALSE 0

#define spawn 0    //estado de spawn da peça
#define right 1    //peca girou uma vez para direita a partir do spawn
#define left 3     //peça girou uma vez para esquerda a partir do spawn
#define two 2      //peça girou duas vezes a partir do spawn, girando em qualquer direcao

#define moveRight 51    //Comando para mover para direita
#define moveLeft 49     //Comando para mover para esquerda
#define moveDown 50     //Comando para mover para baixo
#define rotate 52       //Comando para rotacionar

volatile char command;  //Recebe o comando do joystick//Alterado na interrupcao do AD

signed char currentBlockRecipe[4][2];     //Armazena a recipe de formacao do bloco atual//usado na rotacao
char linesInfo[20];                       //A area de jogo possui 20x10, cada posicao desse vetor armazena a quantidade de colunas preenchidas da linha, usada para verificar linhas completas
signed currentBlock;                      //Indica qual o bloco ativo atual
char currentBlockRotation;                //Indica a posicao de rotacao atual do bloco

volatile char goDown;         //Flag alterada por interrupcao do timer A2, indica quando a peca por descer de maneira automatica

void changeDifficult();

void resetLinesInfo();        //Reseta o linesInfo

void copyMatrix(signed char matriz1[4][2], signed char matriz2[4][2]);   //Faz copia de matrizes

//Funcoes para gerar bloco novo
int nextBlockDraw();
void nextBlockRecipe(signed char recipe[4][2], char choice);
char nextBlockBuild();

//Funcoes para mover o bloco
char canPieceMove(signed char nextBlockPosition[4][2]);
char moveBlock(char input);
void rotateRight();
char rotateBlock(signed char possibleBlockPosition[4][2]);

//Funcoes para verificar linhas prontas e apagar elas
void updateLinesInfo();
char checkLines();

void eraseLines(char highestLine, char completeLines);

#endif

