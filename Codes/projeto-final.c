// Matheus Trajano do Nascimento - 17/0152227
#include <msp430.h> 
#include "io_lib.h"
#include "gameControl.h"
#include "gameInfo.h"

/*Conexos para o projeto
 *
 *  LCD
 *
    Vcc ------------ 3.3v
    GND ------------ GND
    BL  ------------ resistor 1k - 3.3v

    Din ------------ P3.0
    CLK ------------ P3.2
    CE  ------------ P2.7
    D/C ------------ P4.2
    RST ------------ p7.4

    Joystick

    Vcc ------------ 3.3v
    GND ------------ GND
    X   ------------ P6.0
    Y   ------------ P6.1
    SW  ------------ p6.2
*/

void TA2_config(void);
void TA0_config(void);
void TA1_config(void);

void io_config(void);

void resetGame();


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	int i;

	char canBlockMoveDown = TRUE;         //Flag que recebe resultado do movimento da peca
	char canBlockSpawn2 = TRUE;           //Flag que indica se a peca pode spawnar
	char completeLines = FALSE;           //Flag que indica se houve linhas completas

	highScore = 0;

       USCI_B1_config();
       lcd_init();
       clearLCD();

       resetGame();

       TA0_config();
       TA1_config();
       TA2_config();

       ad_config();

       io_config();

       __enable_interrupt(); //Hab. Geral (GIE=1)

	   while(1) {

	     if((P6IN & BIT2) == 0) {        //SW do joytstick, reseta o jogo
	       for(i=0; i<20000; i++);
	       resetGame();
	     }

	     //atualizacao do LCD
	     if(flagUpdateLCD == TRUE)
	     {
	       lcdUpdate();
	       if(completeLines == TRUE) {
	         printPoints();
	       }
	       completeLines = FALSE;
	       flagUpdateLCD = FALSE;
	     }

         //Comando do joystick
	     if(newCommand == TRUE)
	     {

           if((command != rotate) && (command != 0))
             moveBlock(command);
           else if(command == rotate)
             rotateRight();

           command = 0;

           newCommand = FALSE;

	     }

	    //Descida periodica da peca
	    if(goDown == TRUE)
	    {
	      canBlockMoveDown = moveBlock(50);
	      goDown = FALSE;
	    }

	    //Peca nao pode mais descer e sera criado outra
	    if(canBlockMoveDown == FALSE)
	    {

	      updateLinesInfo();
	      completeLines = checkLines();
	      canBlockSpawn2 = nextBlockBuild();
	      canBlockMoveDown = TRUE;

	      if(canBlockSpawn2 == FALSE)           //Se a peca nao pode spawnar, o jogo atual acaba e eh chamado o reset
	      {
	        canBlockSpawn2 = TRUE;
	        resetGame();
	      }

	      if(completeLines == TRUE) changeDifficult();

	    }

	 }

	return 0;

}


//Funcao que reseta o jogo
void resetGame() {

       __disable_interrupt(); //Desabilticao geral (GIE=0)

       newCommand = FALSE;
       flagUpdateLCD = FALSE;
       goDown = FALSE;
       command = 0;

       if(points > highScore) {
         highScore = points;
       }

       printHighScore();

       points = 0;

       resetLinesInfo();
       resetGameplayMatrix();

       changeDifficult();   ///reseta a dificuldade

       TA0R = 0;
       TA1R = 0;
       TA2R = 0;

       nextBlockBuild();
       printPoints();

       resetAnimation();

       __enable_interrupt(); //Hab. Geral (GIE=1)

    return;

}





//Configuracao dos timers e do IO
void TA2_config(void) {

    TA2CTL = TASSEL__ACLK | MC_1; //ACLK e Modo 1
    TA2CCTL0 = CCIE; //CCIFG interrompe
    TA2CCR0 = 24576; //16384; //Desce o bloco automaticamente a cada 0.5 segundos

}

//Configuracao do timer que conta o tempo de atualiacao do LCD
void TA1_config(void) {

    TA1CTL = TASSEL__ACLK | MC_1; //ACLK e Modo 1
    TA1CCTL0 = CCIE; //CCIFG interrompe
    TA1CCR0 = 1639; //Limite de contagem (0,05 s) // 20 vezes por segundo

}


// Configurar GPIO
void io_config(void) {           //Configuracao da GPIO

       P1DIR |= BIT0;      //Led1 = P1.0 = saída
       P1OUT &= ~BIT0;     //Led1 apagado

       P4DIR |= BIT7;      //Led2 = P4.7 = saída
       P4OUT &= ~BIT7;     //Led1 apagado

       P6DIR &= ~BIT2;     //SW = P6.2 = entrada
       P6REN |= BIT2;      //Habilitar resistor
       P6OUT |= BIT2;      //Habilitar pullup

       P2DIR &= ~BIT1; //P2.1 = entrada
       P2REN |= BIT1; //Habilitar resistor
       P2OUT |= BIT1; //Selecionar Pullup

}

//Timer que contra o conversor AD
void TA0_config(void){          //Configuracao do timer

       TA0CTL = TASSEL_1 | MC_1;   //32768 Hz
       TA0CCTL1 = OUTMOD_6; //Out = modo 6
       TA0CCR0 = 2750; //1 Hz - 0,015625 segundos entre cada leitura - 32 leituras por 0,5 segudos - 64 leituras por segundo
       TA0CCR1 = TA0CCR0/2; //Carga 50%

}
