//Library para controlar o jogo, calcular posicao nova da peca, rotacionar, etc
#include "gameControl.h"
#include "msp430.h"

//Aumenta a velocidade de queda do bloco
void changeDifficult() {

    static char currentVelocity = 0;

    if((points == 0)) {
        TA2R = 0;
        currentVelocity = 0;
        TA2CCR0 = 24576;
    }

    else if((points >= 200) && (currentVelocity == 0)) {
      TA2R = 0;
      currentVelocity++;
      TA2CCR0 = 16384;
    }

    else if((points >= 600) && (currentVelocity == 1)) {
      TA2R = 0;
      currentVelocity++;
      TA2CCR0 = 11469;
    }

    return;

}


//Interrupcao responsavel por comandar a descida automatica da peca
#pragma vector = 44
__interrupt void isr_ta2_ifg(void) {

    goDown = TRUE;

}

//Copia uma matriz para outra
void copyMatrix(signed char matriz1[4][2], signed char matriz2[4][2]) {
	
	int i,j;
	
	  for(i=0; i<4; i++) {
	     for(j=0; j<2; j++) {
		    matriz2[i][j] = matriz1[i][j];
	     }
	  }	   
	
      return;	
	
}

//Funcoes responsaveis por gerar um novo bloco

//Sorteia um numero entre 0 e 6 que sera o novo bloco gerado
int nextBlockDraw() {

    return rand() % 7;
	
}

//Funcao que retorna a receita do bloco sorteado//a receita eh usada para construir o bloco a partir de 4 quadrados
//um dos quadrados eh o pivo, como se fosse o centro de um sistema de coordenadas, e os outros sao gerados em volta dele
//a partir da receita
void nextBlockRecipe(signed char recipe[4][2], char choice) {
	
	char k = 0, i, j;
	//Matriz que armazena as receitas, cada linha eh a receita de uma peca, sendo 2 coordenas para cada quadrado
	//o quadrado pivo eh representado na primeira e segunda coluna, sempre com zero e zero, ja que eh a referencia
	signed char blocksRecipes[7][8] = {0,0,-1,0,0,1,0,-1,          //t Block
	                                   0,0,0,1,0,-1,-1,-1,         //ll Block
		                               0,0,0,1,0,-1,-1,+1,         //lr Block
		                               0,0,0,+1,0,-1,0,-2,         //s Block
		                               0,0,0,-1,-1,0,-1,-1,        //sq Block
		                               0,0,0,-1,-1,0,-1,+1,        //zr Block
		                               0,0,0,+1,-1,0,-1,-1,        //zl Block
							          };
	
	//A receita do bloco sorteado eh armazenada no vetor recebido como parametro
	for(i=0; i<4; i++) {
	   for(j=0; j<2; j++) {
		  recipe[i][j] = blocksRecipes[choice][k];
		  k++;
	   }
	}
	
}

//Funcao responsavel por verificar se o local de spawn de pecas, que eh no centro da primeira e segunda linha da area
//de jogo, esta vazio
char canBlockSpawn() {
	
	int i;
	
	   for(i=0; i<4; i++) {
		  if(gameplayMatrix[blockPosition[i][0]][blockPosition[i][1]] == 1)
            return FALSE;			  
	   }
	
	return TRUE;
	
}

//Funcao responsavel por construir o proximo bloco na posicao de spawn da peca
char nextBlockBuild() {
	
	char i, j;
	char anchor[2] = {1,5}, blockCopy[4][2];
	
	  currentBlock = nextBlockDraw();
	  nextBlockRecipe(currentBlockRecipe,currentBlock);              //eh feito uma copia da recipe do bloco atual, eh usada na parte de rotacao
	  
	  //Cria a matriz de localizacao incial da nova peca
	  for(i=0; i<4; i++) {
	    blockPosition[i][0] = anchor[0] + currentBlockRecipe[i][0];   //anchor eh a posicao de spawn dos quadrados pivos, entao os blocos sao gerados em torno de (1,5)
	    blockPosition[i][1] = anchor[1] + currentBlockRecipe[i][1];   //posicao de spawn eh somado com a receita do bloco
	  }
	  //Cria a matriz de localizacao incial da nova peca
	  
	  //Chama a funcao para verificar se o spawn do bloco esta vazio
	  if(canBlockSpawn() == FALSE) return FALSE;

	  //Coloca o bloco criado na matriz de jogo
	  for(i=0; i<4; i++) {
		 gameplayMatrix[blockPosition[i][0]][blockPosition[i][1]] = 1;  
	  }
	  
	  //Define a rotacao atual do bloco como a inicial (spawn)
	  currentBlockRotation = spawn;
		
      return TRUE;
		
}
/////////////////////////////////////////////////////

//Funcao responsavel por mover o bloco de acordo com o input
char moveBlock(char input) {
	
	int i;
	signed char sum=1, collum=0;
	signed char blockCopy[4][2] = {0,0,0,0};
	
	   getBlockPosition(blockCopy);
	
       switch(input) {
	      case moveRight:
	        sum = 1;
			collum = 1;
			break;
		  case moveLeft:
			sum = -1;
			collum = 1;
			break;
		  case moveDown:
			sum = 1;
			collum = 0;
			break;
		}

       //Soma as coordenas atuais do bloco um valor a depender do input// collum define se sera somado para ir para os lados ou para baixo
	   for(i=0; i<4; i++) 
		  blockCopy[i][collum] += sum;
	   
	   //Verifica se a nova posicao do bloco ja nao esta preenchida, nesse caso, o movimento na acontece
	   if((canPieceMove(blockCopy)) == FALSE) 
	     return FALSE;
	   
	   //Atualiza a matriz de jogo apagando a psoicao passada do bloco antes do movimento
	   for(i=0; i<4; i++) {
	      gameplayMatrix[blockPosition[i][0]][blockPosition[i][1]] = 0;
	   }
	   
	   //Atualiza a matriz de jogo colocando o bloco na nova posicao
	   for(i=0; i<4; i++) {
	      gameplayMatrix[blockCopy[i][0]][blockCopy[i][1]] = 1;
	   }
	   
	   //Coloca a posicao atual do bloco com a que foi calculada e armazenada na matriz copia
	   setBlockPosition(blockCopy);

	   return TRUE;
	
}

//Verifica se a peca pode se mover para uma determinada posicao
char canPieceMove(signed char nextBlockPosition[4][2]) {
	
	char line, collum, i, j;
	char fillSpace=TRUE;

	  //Verifica as quatro coordenadas da nova posicao da peca se ja esta preenchida
	  for(i=0; i<4; i++) {

		 //////////Verificacao dos limites do gamespace 
		 if(nextBlockPosition[i][0] == 20) return FALSE;
		 if(nextBlockPosition[i][1] == 10) return FALSE;
		 if(nextBlockPosition[i][1] == (-1)) return FALSE;
		 //////////Verificacao dos limites do gamespace 
		 
		 fillSpace = TRUE;
		 line = nextBlockPosition[i][0];
		 collum = nextBlockPosition[i][1];
		 //Verifica se as coordenas da nova posicao estao preenchidas
		 if(gameplayMatrix[line][collum] == 1) {
		   for(j=0; j<4; j++) {
			  if((blockPosition[j][0] == line) && (blockPosition[j][1] == collum)) {
				fillSpace = FALSE;
                break;				
			  }
		   }
		   if(fillSpace == TRUE) return FALSE;             //Retorna false se a peca nao pode se mover
		 }	 
	  }
	
	return TRUE;
		
}

//Funcao para rotacionar a peca no sentido horario
void rotateRight() {
	
	int i,j;
	signed char blockCopy[4][2];
	
	//As duas matrizes a seguir fazem parte de um sistema de rotacao do tetris, onde, caso nao seja possivel rotacionar a partir da posicao atual do
	//bloco eh testado uma outras possibilidades deslocando o bloco de uma certa maneira, se nenhum for possivel entao nao gira, isso permite wallkicks
	//cada linha da matriz corresponde a rotacao atual da peca
	signed char throwsRotationsLines[4][4] = {0, -1, 2, 2,
	                                          0, 1, -2, -2,
	                                          0, -1, 2, 2,
	                                          0, 1, -2, -2};
		
    signed char throwsRotationsCollums[4][4] = {-1, -1, 0, -1,
                                                1, 1, 0, 1,
                                                1, 1, 0, 1,
                                                -1, -1, 0, -1};
	
    //As duas matrizes a seguir seguem a mesma dieia do sistema de rotacao mas sao direcionadas unicamente para o bloco reto, que difere dos outros blocos
    //na rotacao
	signed char throwsRotationsLinesS[4][4] = {0, 0, 1, -2,
	                                           0, 0, -2, 1,
	                                           0, 0, 1, -2,
	                                           0, 0, 2, -1};

	signed char throwsRotationsCollumsS[4][4] = {-2, 1, -2, 1,
	                                             -1, 2, -1, 2,
	                                             -2, 1, -2, 1,
	                                             1, -2, 1, -2};
	  
	  if(currentBlock == 4) return;      //retorna se o bloco for o bloco quadrado, ja quele nao gira
	  
	  getBlockPosition(blockCopy);
	  
      if((rotateBlock(blockCopy)) == TRUE) return;     //testa para posicao original do bloco

      if(currentBlock != 3) {                          //Caso o bloco nao seja o reto, ja que ele usa uma matriz de rotacao unica para ele
        for(j=0; j<4; j++) {                           //Eh testado 4 possibildiades de ajuste para giro
		   getBlockPosition(blockCopy);
		   for(i=0; i<4; i++) {
			  blockCopy[i][0] += throwsRotationsLines[currentBlockRotation][j];         //atribui a possivel posicao futura a uma copia da matriz posicao
			  blockCopy[i][1] += throwsRotationsCollums[currentBlockRotation][j]; 
		   }	 
           if((rotateBlock(blockCopy)) == TRUE) return;                  //se a posicao testada for valida, retorna, caso nao, testa a proxima
	    }
	  }
	
	  else {                                               //Caso o bloco seja o bloco reto
	    for(j=0; j<4; j++) {
		   getBlockPosition(blockCopy);
		   for(i=0; i<4; i++) {
			  blockCopy[i][0] += throwsRotationsLinesS[currentBlockRotation][j]; 
			  blockCopy[i][1] += throwsRotationsCollumsS[currentBlockRotation][j]; 
		    }
           if((rotateBlock(blockCopy)) == TRUE) return;
	    }
	  }

    return;  
	
}

//Funcao que faz as alteracoes de rotacao em cima da matriz posicao recebida e testa se eh uma rotacao valida
char rotateBlock(signed char possibleBlockPosition[4][2]) {
	
	char i,aux;
	signed char blockCopy[4][2], temporaryBlockRecipe[4][2];
	
	  copyMatrix(currentBlockRecipe,temporaryBlockRecipe);
	  
	  for(i=0; i<4; i++) {
		 temporaryBlockRecipe[i][0] *= (-1);          //multiplicado por -1 para trocar o sinal de uma coluna devido ao sitema de rotacao, que envolve uma matriz de rotacao
	  }
	
	  for(i=0; i<4; i++) {
	     aux = temporaryBlockRecipe[i][0];
	     temporaryBlockRecipe[i][0] = temporaryBlockRecipe[i][1];
	     temporaryBlockRecipe[i][1] = aux;                             //troca coluna com linha, ou x com y, tambem tem haver com uso de matriz de rotacao
	     blockCopy[i][0] = temporaryBlockRecipe[i][0] + possibleBlockPosition[0][0];  
	     blockCopy[i][1] = temporaryBlockRecipe[i][1] + possibleBlockPosition[0][1]; 
		 
          if(currentBlock == 3) {              //Ajuste feito quando o bloco reto gira
			if(currentBlockRotation == spawn) blockCopy[i][0] += 1;	 
			if(currentBlockRotation == right) blockCopy[i][1] -= 1;	 
			if(currentBlockRotation == two) blockCopy[i][0] -= 1;	 
			if(currentBlockRotation == left) blockCopy[i][1] += 1;	 
		  } 	  
	  }
	
	  if((canPieceMove(blockCopy)) == FALSE) {          //Testa se a rotacao eh valida
		 return FALSE;
	  }
	
	  //Apaga a posicao passada do bloco da matriz de jogo
	  for(i=0; i<4; i++) {
	     gameplayMatrix[blockPosition[i][0]][blockPosition[i][1]] = 0;
	  }
	
	  //Insere o bloco nas novas coordenas na matriz de jogo
	  for(i=0; i<4; i++) {
	     gameplayMatrix[blockCopy[i][0]][blockCopy[i][1]] = 1;
	  }
	  
	  //Muda o estado atual de rotacao do bloco
	  currentBlockRotation++;
	  if(currentBlockRotation == 4) currentBlockRotation = spawn;

      setBlockPosition(blockCopy); 
	  copyMatrix(temporaryBlockRecipe,currentBlockRecipe);
	
	return TRUE;
	
}

//Zera o vetor linesInfo
void resetLinesInfo() {

    char i;

      for(i=0; i<20; i++)
         linesInfo[i] = 0;

}

//Atualiza o vetor linesInfo a cada bloco encaixada na pilha de blocos
void updateLinesInfo() {

	char i;
	  
	   for(i=0; i<4; i++) {
		  linesInfo[blockPosition[i][0]]++;
	   }
	  
	return;  
	
}

//Funcao que verifica se tem linhas completas
char checkLines() {
	
	signed char highestLine=-1, completeLines=0;
	char i;
	char erase=FALSE, eraseHappened=FALSE, lastLine=FALSE;
	
	  for(i=0; i<20; i++) {
	     if(linesInfo[i] == 10) {          //Cada linha tem 10 colunas, entao verifica se a linha i eh igual a 10
		   if(i == 19) lastLine = TRUE;
		   if(completeLines == 0) {        //Entra nesse if uma vez a cada conjunto de linhas verificadas
		     highestLine = i;              //Atribui a linha como a mais alta no espaço de jogo a ser apagada
			 erase = TRUE;
			 eraseHappened = TRUE;
		   }
		   completeLines++;                //Numero de linhas completas
	     }
         if((linesInfo[i] < 10) && (erase == TRUE)) {       //Ao achar a primeira linha completa, as quem abaixo dela sao verfificadas ate achar uma linha vazia
           if(completeLines == 4) points = 30;              //nesse momento, se erase for true, o conjunto de linhas de tamanho completeLines eh apagado
           else points += completeLines*5;                  //Pontua-se 5 pontos para cada linha eliminada e, em caso de tetris, sao 30 pontos
		   eraseLines(highestLine, completeLines);
           completeLines = 0;
           erase = FALSE;		   
		 }		 
	  }
	  
	if(lastLine == TRUE) {                       //Caso a ultima linha da area jogo esteja completa
	  eraseLines(highestLine, completeLines);
      if(completeLines == 4) points += 30;
      else points += completeLines*5;
	}
	   	
	return eraseHappened;
	
}

//Funcao responsavel por apagar as linhas completas, recebe a linha mais alta e a quantidade de linhas a serem apagadas a partir dela
void eraseLines(char highestLine, char completeLines) {

	signed char i, j, k=0;
	  
	  for(i=highestLine; i<(highestLine + completeLines); i++) {     //Zera todas as linhas que precisam ser apagadas
		 for(j=0; j<10; j++) {
			gameplayMatrix[i][j] = 0;			
		 } 
		 linesInfo[i] = 0;
	  }

	  for(i=(highestLine-1); i>=0; i--) {                   //Verifica quais linhas acima da highest tem algum bloco e puxa eles para baixo
		 if(linesInfo[i] != 0) {                            //para ocupar o espaco que ficou vago
           j=0;		   
		   while(j < linesInfo[i]) { 
			  if((gameplayMatrix[i][k]) == 1) {
			    gameplayMatrix[i][k] = 0;
			    gameplayMatrix[i + completeLines][k] = 1;
			    linesInfo[i + completeLines]++;
				j++;
		      }
			  k++; 
		   }
		   k=0;
		   linesInfo[i] = 0;
		 } 
		 else {
		   return; 
		 }  
	  }
	
	return;
		
}

