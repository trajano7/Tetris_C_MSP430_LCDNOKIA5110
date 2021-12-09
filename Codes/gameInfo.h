#ifndef GAMEINFO_H_
#define GAMEINFO_H_

/////////////Biblioteca das informacoes a respeito da area de jogo e da peca atual
#include <stdio.h>


char gameplayMatrix[20][10];                  //matriz que armazena o estado da regiao de jogo
signed char blockPosition[4][2];              //matriz que armazena a informacao da peca ativa cada peca eh formada por 4 blocos
                                              //as coordenadas de cada um sao indicadas nessa matriz

int points;    //Armazena a pontuacao atual do jogo
int highScore; //Armazena a maior pontuacao entre todos os jogos

void resetGameplayMatrix();

void getBlockPosition(signed char blockCopy[4][2]);
void setBlockPosition(signed char blockCopy[4][2]);

#endif
