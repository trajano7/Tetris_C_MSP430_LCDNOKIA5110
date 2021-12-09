//Library de entradas e saídas// LCD e joystick
#include "io_lib.h"
#include "gameInfo.h"
#include "gameControl.h"

//Definicao dos codigos necessarios para imprimir os numeros de 0 a 9 no LCD
volatile char numbers[10][10] ={ 0x7e, 0x7e, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7e, 0x7e, //0
                                 0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, //1
                                 0x78, 0x3c, 0x0e, 0x06, 0x06, 0x06, 0x16, 0x38, 0x7e, 0x7e, //2
                                 0x7e, 0x7e, 0x06, 0x06, 0x7e, 0x7e, 0x06, 0x06, 0x7e, 0x7e, //3
                                 0x66, 0x66, 0x66, 0x7e, 0x7e, 0x06, 0x06, 0x06, 0x06, 0x06, //4
                                 0x7e, 0x7e, 0x60, 0x60, 0x7e, 0x7e, 0x06, 0x06, 0x7e, 0x7e, //5
                                 0x7e, 0x7e, 0x60, 0x60, 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e, //6
                                 0x7e, 0x7e, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, //7
                                 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e, //8
                                 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e, 0x06, 0x06, 0x7e, 0x7e, //9
                               };


/////Funcoes relacionadas ao LCD

//Funcao que faz uma animacao para limpar a tela ao se resetar o jogo
void resetAnimation() {

    char i,j;
    char x=0,y=1;

      sendCommand(0x22);

      setCoordinate(x,y);

      for(i=0; i<=20; i++) {
         for(j=0; j<4; j++) {
            sendData(0xff);
            sendData(0xff);
            sendData(0xff);
            sendData(0xff);
            sendData(0xff);
            setCoordinate(x++, y);
         }
         delay_ms(100);
      }

      x=0;
      setCoordinate(x,y);

      for(i=0; i<=20; i++) {
         for(j=0; j<4; j++) {
            sendData(0x00);
            sendData(0x00);
            sendData(0x00);
            sendData(0x00);
            sendData(0x00);
            setCoordinate(x++, y);
         }
         delay_ms(100);
      }

      sendCommand(0x20);
      delay_ms(200);

}

//Mostra um dos char definidos na matriz numbers
void printChar(char number) {

    char i;

       for(i=0; i<10; i++) {
          sendData(numbers[number][i]);
       }

}

//Atualiza os pontos no LCD
void printPoints() {

      setCoordinate(5,0);
      printDec(points);

}

//Atualiza o high score no LCD
void printHighScore() {

      setCoordinate(47,0);
      printDec(highScore);

}

//Mostra um decimal no LCD
void printDec(int number) {

    unsigned volatile int div, aux;

     aux = number;

        div = aux/100;
        printChar(div);

        sendData(0x00);

        aux %= 100;
        div = aux/10;
        printChar(div);

        sendData(0x00);

        aux %= 10;
        printChar(aux);

}

//Funcao de interrupcao do timer A1 - conta o tempo para atualizar o LCD
#pragma vector = 49
__interrupt void isr_ta1_ifg(void) {

        flagUpdateLCD = TRUE;

}

//Funcao de configuracao da USCI B1 para operar com SPI
void USCI_B1_config() {

    /*

    Vcc ------------ 3.3v
    GND ------------ GND
    BL  ------------ resistor - 3.3v


    Din ------------ P3.0
    CLK ------------ P3.2
    CE  ------------ P2.7
    D/C ------------ P4.2
    RST ------------ p7.4

    */

       UCB0CTL1 |= UCSSEL__SMCLK | UCSWRST;               //Ativa o reset e escolhe o SMCLK
       UCB0CTL0 |= UCCKPH | UCMSB | UCMST | UCSYNC;
       UCB0BR0 |= 0x01; // 1:1
       UCB0BR1 = 0;

       UCB0CTL1 &= ~UCSWRST;

}


//Funcao responsavel por atualizar o LCD
void lcdUpdate() {

    /*   LCD tem 84x48 pixels, aqui, para cada posicao da gameplayMatriz 20x10, sao atribuidos 4 pixels para melhor vizualizacao
     *   logo, cada numero da matriz forma um bloco de 4x4 pixels, sendo assim, temos um espaco pro jogo de 80x40 no LCD.
     *   Sobram 8 na altura e 4 na largura.
     */

    unsigned char i,j,k=9;
    unsigned char data=0;
    unsigned char fstCollum = TRUE, lstCollum = FALSE;
    char line = 0, min2 = 0, max2 = 19;

       setCoordinate(0,1);

       for(i=1; i<=5; i++) //LCD esta operando em um modo onde sua altura (48 pixels) e saltada em 1 byte em 1 byte, sendo total de 5 bytes
       {
          for(j=min2; j<=max2; j++)   //Loop que vai passar por todas 20 linhas da gameplayMatrix
          {

             if((gameplayMatrix[j][k] == 1)) data |= 0x0f;   //Para cada linha verifica 2 colunas de uma vez, com excecao para a primeira e ultima, que sao tratadas de maneira diferente devido as sobras de pixels
             if((gameplayMatrix[j][k-1] == 1)) data |= 0xf0;

             sendData(data);
             sendData(data);
             sendData(data);
             sendData(data);      //manda a mesma data 4 vezes para formar quadrados 4x4 para cada numero da matriz

             data = 0;

           }

          //No modo que LCD esta trabalhando ele percorre as 84 colunas de uma em uma e as 48 linhas sao percorridas de byte em byte, sendo assim,
          //sao mandados 8 bits por vez que preenche cada um desses bytes.


        //As colunas da matriz de gameplay sao incrementadas aqui//sao acessadas duas por vez
        k -= 2;
        setCoordinate(0, i+1);       //Pula para o proximo byte

      }

}

//Funcao responsavel por alterar o cursor do LCD
void setCoordinate(char x, char y) {

         sendCommand(0x40 | y);
         sendCommand(0x80 | x);

}

//Funcao responsavel por limpar o LCD
void clearLCD() {

       setCoordinate(0,0);

       volatile char j;
       int i;

       for(i=0; i<504; i++) {
         sendData(0);
       }

       setCoordinate(0,0);

}

//Funcao que envia um dados para serem mostrados no LCD//Sao mandados 8 bytes de uma vez
void sendData(char data) {

       P2OUT &= ~BIT7;        //Habilita o escravo
       P4OUT |= BIT2;         //Coloca em modo data

       UCB0TXBUF = data;                     //Manda o dado ja antes porque o spi esta configurando para pegar no primeiro flanco
       while((UCB0IFG & UCTXIFG) == 0);      //Espera acabar a transmissao

       P2OUT |= BIT7;                        //Desabilita o escravo

    return;

}

//Funcao responsavel por mandar comandos para o LCD
void sendCommand(char command) {

       P2OUT &= ~BIT7;         //Habilita o escravo
       P4OUT &= ~BIT2;         //Coloca em modo comando

       UCB0TXBUF = command;                  //Manda o dado ja antes porque o spi esta configurando para pegar no primeiro flanco
       while((UCB0IFG & UCTXIFG) == 0);      //Espera acabar a transmissao

       P2OUT |= BIT7;

    return;

}

//Funcao de incializacao do LCD//Feito de acordo como especificado no datasheet e com referencias de outros codigos
void lcd_init() {

       P3SEL |= BIT0;          //Pino Din (3.0), entrada de dados/comandos do LCD
       P3SEL |= BIT2;          //Pino Clk (3.2), entrada de clock do LCD

       P2DIR |= BIT7;          //Pino CE (2.7), entrada de enable do LCD, ativa o LCD em nivel baixo
       P2OUT |= BIT7;

       P4DIR |= BIT2;          //Pino D/C (4.2), indica se o dado enviado sera dado ou comando
       P4OUT |= BIT2;          //comando em nivel baixo e dado em nivel alto

       P7DIR |= BIT4;          //Pino RST, reseta o LCD, necessario para incializacao, ativo em nivel baixo
       P7OUT |= BIT4;

       delay_ms(50);

       P7OUT &= (~BIT4);       //Ativa o reset
       delay_us(1);            //da um delay antes de desativar o reset
       P7OUT |= (BIT4);        //Desativa o reset

//sendCommand(0x21);
       sendCommand(0x21);      //instrucao function set, muda para o conjunto de instrucoes estendido, poem no modo de enderecamento horizontal e liga o LCD

       sendCommand(0x84);      //Instrucao que define o Vop, a tensao de operacao do LCD, foi usado formula de um datasheet para por em 3.3v
       sendCommand(0x06);      //Intrucao define o coeficiente de temperatura, relacionado ao contraste
       sendCommand(0x13);      //Instrucao define o Bias system

       sendCommand(0x20);      //muda para o conjunto de instrucoes normal
       sendCommand(0x0C);      //poem o display no normal mode

}

//funcao de delay usada para incializacao
void delay_us(unsigned int us)
{
    while (us)
    {
        // 1 for 1 Mhz set 16 for 16 MHz
        __delay_cycles(1);
        us--;
    }
}

//funcao de delay usada para incializacao
void delay_ms(unsigned int ms)
{
    while (ms)
    {
        // 1000 for 1MHz and 16000 for 16MHz
        __delay_cycles(1000);
        ms--;
    }
}
///////////////////////////////



//Funcoes relacionadas ao joystick

//Interrupcao para leitura do ADC//interrompe somente para ADC12MEM1
#pragma vector = ADC12_VECTOR //Interrupção de número 54
__interrupt void adc12_isr(void){

    char xaxis,yaxis;

    ADC12IV;

       xaxis = ADC12MEM0;
       yaxis = ADC12MEM1;

       if(xaxis >= 200)          //comando para mover para esquerda
       {
         command = moveLeft;
       }
         else if(xaxis <= 50)    //comando para mover para direita
         {
             command = moveRight;
         }

       if(yaxis >= 200)          //comando para mover para girar o bloco no sentido horario
       {
           command = rotate;
       }
         else if(yaxis <= 50)    //comando para mover para descer o bloco
         {
             command = moveDown;
         }

       newCommand = TRUE;           //flag que indica a presenca de um novo comando para a main

}


//adc esta configurado para fazer leituras em modo sequencia e repetido, sendo que o flag de controle eh o timer A0
void ad_config() {      //Configuracao do AD

       ADC12CTL0 &= ~ADC12ENC;    //Desabilita conversoes

       ADC12CTL0 = ADC12SHT0_3 | ADC12ON;
       ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SHS_1 | ADC12DIV_0 | ADC12SSEL_3 | ADC12CONSEQ_3;  //le em sequencia o 0 e o 1, em modo sequencia com repeticao controlado pelo timer A0
       ADC12CTL2 = ADC12TCOFF | ADC12RES_0;        //8bits

       ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_0;                 //Controlador do canal 0
       ADC12MCTL1 = ADC12SREF_0 | ADC12EOS | ADC12INCH_1;      //Controlador do canal 1

       ADC12IE |= ADC12IE1;                                    //Interrompe ao ler o canal 1

       ADC12IFG = 0;

       P6SEL |= BIT0 | BIT1;

       ADC12CTL0 |= ADC12ENC;    //Habilita conversoes

}


