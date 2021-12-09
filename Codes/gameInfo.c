//Library que guarda e manipula as informacoes basicas do jogo
#include "gameInfo.h"

//Reseta a matriz do jogo
void resetGameplayMatrix() {
	
	int i,j;
	
	  for(i=0; i<20; i++) {
	     for(j=0; j<10; j++) {
		    gameplayMatrix[i][j] = 0;
	     }
	  }	   
	
      return;	
	
}		

//Copia a posicao atual do bloco para outra matriz
void getBlockPosition(signed char blockCopy[4][2]) {
	
	int i,j;
	
	  for(i=0; i<4; i++) {
	     for(j=0; j<2; j++) {
		    blockCopy[i][j] = blockPosition[i][j];
	     }
	  }	   
	
      return;		
	
}

//Altera o valor da posicao atual da peca
void setBlockPosition(signed char blockCopy[4][2]) {
		
	int i,j;
	
	  for(i=0; i<4; i++) {
	     for(j=0; j<2; j++) {
		    blockPosition[i][j] = blockCopy[i][j];
	     }
	  }	   
	
      return;		
		
}
