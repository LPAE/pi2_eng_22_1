#include <LiquidCrystal.h> //INCLUSÃO DE BIBLIOTECA
#include <DHT.h>
#include "Ultrasonic.h"

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //PORTAS DO ARDUINO QUE SÃO UTILIZADAS PELO SHIELD KEYPAD DISPLAY LCD

enum {NONE, CIMA, BAIXO, DIR, ESQ, SEL};
volatile uint8_t botao = NONE;

enum {INIT, E1, E2, E3, CONFIG, CONFIG_2};
enum {C1, C2};
volatile uint8_t tela_atual = INIT;
volatile uint8_t tela_a = C1;

//#define DEBUG_BOTAO
#define DEBUG_TELA

// Define a entrada analogica com o valor "1"
#define ENTRADA_ANALOGICA A1
#define ENTRADA_ANALOGICA2 A2
#define RELE 3
#define DHT22PIN 2

//Modelo do DHT
#define DHTTYPE  DHT22

DHT dht(DHT22PIN, DHTTYPE);

//Define os pinos para o trigger e echo
#define pino_trigger 11
#define pino_echo 12

//Inicializa o sensor nos pinos definidos acima
//Ultrasonic ultrasonic(pino_trigger, pino_echo);

// Recebe a leitura feita no pino analógico
int aRec, aRec2;
int lvl = 0, umi1 = 0, umi2 = 0;
float t, h;


void tarefa_exibetela01();
void tarefa_exibetela02();
void tarefa_exibetelaconfig();

void setup() {

  // Inicia a comunicação serial a 9600 bits por segundo
  Serial.begin(9600);

  // inicializa classe do sensor
  dht.begin();

  // Configura o pino do relé como saída
  pinMode (RELE, OUTPUT);

  lcd.begin(16, 2); //SETA A QUANTIDADE DE COLUNAS(16) E O NÚMERO DE LINHAS(2) DO DISPLAY. EM SUMA: UMA MATRIZ DE 16 COLUNAS E 2 LINHAS
  lcd.setCursor(0, 0); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1)
  lcd.print("Bem-vinde       "); //ESCREVE NA PRIMEIRA LINHA DO DISPLAY LCD
  delay(200);
  lcd.setCursor(0, 0); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1)
  lcd.print("                 "); //ESCREVE NA PRIMEIRA LINHA DO DISPLAY LCD
  lcd.setCursor(0, 0); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1)
  lcd.print("Easyplant"); //ESCREVE A FRASE NA PRIMEIRA LINHA DO DISPLAY LCD
  delay(200);
}

void loop() {

  tarefa_telas();
  tarefa_botoes();
  tarefa_solo();
  tarefa_setumi();
  tarefa_rele();
  tarefa_temp();

}


/* Essas variáveis são globais pois é necessário
   manter os valores independente do contexto de
   execução da função tarefa_botãoes */
const unsigned long periodo_tarefa_botoes = 100;
unsigned long tempo_tarefa_botoes = millis();

/* Tarefa 5: envia o valor analógico para o PC */
void tarefa_botoes() {
  unsigned long tempo_atual = millis();

  /* Hora de enviar os dados analógicos caso tenha passado 200 ms */
  if (tempo_atual - tempo_tarefa_botoes > periodo_tarefa_botoes) {
    tempo_tarefa_botoes = tempo_atual;

    if ((analogRead(0)) < 80)
      botao = DIR;
    else if ((analogRead(0)) < 200)
      botao = CIMA;
    else if ((analogRead(0)) < 400)
      botao = BAIXO;
    else if ((analogRead(0)) < 600)
      botao = ESQ;
    else if ((analogRead(0)) < 800) {
      botao = SEL;
    }
    else
      botao = NONE;

#ifdef DEBUG_BOTAO
    Serial.print("Valor : ");
    Serial.println(botao);
#endif

  }
}

/* Essas variáveis são globais pois é necessário
   manter os valores independente do contexto de
   execução da função tarefa_botãoes */
const unsigned long periodo_tarefa_telas = 500;
unsigned long tempo_tarefa_telas = millis();

/* Tarefa 5: envia o valor analógico para o PC */
void tarefa_telas() {
  unsigned long tempo_atual = millis();
  static int contador = 0;

  /* Hora de enviar os dados analógicos caso tenha passado 200 ms */
  if (tempo_atual - tempo_tarefa_telas > periodo_tarefa_telas) {
    tempo_tarefa_telas = tempo_atual;

    contador++;

    switch (tela_atual) {
      case INIT:
        tela_atual = E1;
        tarefa_exibetela01();
        contador = 0;
        break;

      case E1:
        tarefa_exibetela01();

        if (botao == SEL)
          tela_atual = CONFIG;
        else if (contador == 6) {
          tela_atual = E2;
          contador = 0;
        }
        break;

      case E2:
        tarefa_exibetela02();

        if (botao == SEL) {
          tela_atual = CONFIG;
        }
        else if (contador == 6) {
          tela_atual = E3;
          contador = 0;
        }
        break;

      case E3:
        tarefa_exibetelaconfig();

        if (botao == SEL) {
          tela_atual = CONFIG;
        }
        else if (contador == 6) {
          tela_atual = E1;
          contador = 0;
        }
        break;

      case CONFIG:
        tarefa_exibetelaconfig2();

        if (botao == CIMA) //UP
          lvl++;

        lvl &= 0x03; 

        if ( botao == DIR) {
          tela_atual = E1;
          contador = 0;

        }
        break;

      default:
        break;
    }

#ifdef DEBUG_TELA
    Serial.print("Tela : ");
    Serial.println(tela_atual);
#endif
  }
}


/* Tarefa 5: envia o valor analógico para o PC */
void tarefa_exibetela01() {

  /* Tela */

  lcd.setCursor(0, 0); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1)
  lcd.print("Temp= "); //ESCREVE A FRASE NA PRIMEIRA LINHA DO DISPLAY LCD
  lcd.print(float(t));
  lcd.print("C     ");
  lcd.setCursor(0, 1); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 2)
  lcd.print("Reservatorio=  80"); //ESCREVE A FRASE NA SEGUNDA LINHA DO DISPLAY
  lcd.print("%     ");

}

/* Tarefa 5: envia o valor analógico para o PC */
void tarefa_exibetela02() {

  /* Tela */

  lcd.setCursor(0, 0); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1)
  lcd.print("Umidade V1= "); //ESCREVE A FRASE NA PRIMEIRA LINHA DO DISPLAY LCD
  lcd.print(int(umi1));
  lcd.print("%        ");

  lcd.setCursor(0, 1); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 2)
  lcd.print("Umidade V2= "); //ESCREVE A FRASE NA SEGUNDA LINHA DO DISPLAY
  lcd.print(int(umi2));
  lcd.print("%    ");


}

/* Tarefa 5: envia o valor analógico para o PC */
void tarefa_exibetelaconfig() {

  /* Tela */

  lcd.setCursor(0, 0); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1)
  lcd.print("APERTE SEL PARA"); //ESCREVE A FRASE NA PRIMEIRA LINHA DO DISPLAY LCD
  lcd.setCursor(0, 1); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 2)
  lcd.print("CONFIGURAR           "); //ESCREVE A FRASE NA SEGUNDA LINHA DO DISPLAY


}

/* Tarefa 5: envia o valor analógico para o PC */
void tarefa_exibetelaconfig2() {

  /* Tela */
  lcd.setCursor(0, 0); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1)
  lcd.print("NIVEL ATUAL= "); //ESCREVE A FRASE NA PRIMEIRA LINHA DO DISPLAY LCD
  lcd.print(int(lvl));
  lcd.print("         ");
  lcd.setCursor(0, 1); //SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 2)
  lcd.print("PRESS UP      "); //ESCREVE A FRASE NA SEGUNDA LINHA DO DISPLAY

}

const unsigned long periodo_tarefa_solo = 30000;
unsigned long tempo_tarefa_solo = millis();

//Envia os valores interios para o PC
void tarefa_solo() {
  unsigned long tempo_atual = millis ();

  //Hora de enviar os dados caso tenha passado 1000 ms
  if (tempo_atual - tempo_tarefa_solo > periodo_tarefa_solo) {
    tempo_tarefa_solo = tempo_atual;

    // Faz a leitura do sensor
    aRec = analogRead(ENTRADA_ANALOGICA);
    aRec2 = analogRead(ENTRADA_ANALOGICA2);
  }
}


const unsigned long periodo_tarefa_rele = 10000;
unsigned long tempo_tarefa_rele = millis();

//Envia os valores interios para o PC
void tarefa_rele() {
  unsigned long tempo_atual = millis ();

  //Hora de enviar os dados caso tenha passado 1000 ms
  if (tempo_atual - tempo_tarefa_rele > periodo_tarefa_rele) {
    tempo_tarefa_rele = tempo_atual;

    if (lvl = 0) {
      if (aRec > 801 || aRec2 > 801) {
        // Liga o relé
        digitalWrite(RELE, HIGH);
      }
      else {
        // Desliga o relé
        digitalWrite(RELE, LOW);
      }
    }
    else if (lvl = 1) {

      if (aRec > 601 || aRec2 > 601) {
        // Liga o relé
        digitalWrite(RELE, HIGH);
      }
      else {
        // Desliga o relé
        digitalWrite(RELE, LOW);
      }
    }
    else if (lvl = 2) {

      if (aRec > 401 || aRec2 > 401) {
        // Liga o relé
        digitalWrite(RELE, HIGH);
      }
      else {
        // Desliga o relé
        digitalWrite(RELE, LOW);
      }

    }
  }
}

const unsigned long periodo_tarefa_setumi = 30000;
unsigned long tempo_tarefa_setumi = millis();

//Envia os valores interios para o PC
void tarefa_setumi() {
  unsigned long tempo_atual = millis ();

  //Hora de enviar os dados caso tenha passado 1000 ms
  if (tempo_atual - tempo_tarefa_setumi > periodo_tarefa_setumi) {
    tempo_tarefa_setumi = tempo_atual;

    if (aRec > 900)
      umi1 = 10;
    else if (aRec > 780)
      umi1 = 15;
    else if (aRec > 660)
      umi1 = 35;
    else if (aRec > 540)
      umi1 = 55;
    else if (aRec > 420)
      umi1 = 75;
    else
      umi1 = 95;

    if (aRec2 > 900)
      umi2 = 10;
    else if (aRec2 > 780)
      umi2 = 15;
    else if (aRec2 > 660)
      umi2 = 35;
    else if (aRec2 > 540)
      umi2 = 55;
    else if (aRec2 > 420)
      umi2 = 75;
    else
      umi2 = 95;
  }
}


const unsigned long periodo_tarefa_temp = 2000;
unsigned long tempo_tarefa_temp = millis();

//Envia os valores interios para o PC
void tarefa_temp() {
  unsigned long tempo_atual = millis ();

  //Hora de enviar os dados caso tenha passado 2000 ms
  if (tempo_atual - tempo_tarefa_temp > periodo_tarefa_temp) {
    tempo_tarefa_temp = tempo_atual;

    // Lê a umidade
    h = dht.readHumidity();
    // Lê a temperatura em Celsius
    t = dht.readTemperature();

  }
}

const unsigned long periodo_tarefa_aprox = 2000;
unsigned long tempo_tarefa_aprox = millis();

//Envia os valores interios para o PC
void tarefa_aprox() {
  unsigned long tempo_atual = millis ();

  //Hora de enviar os dados caso tenha passado 2000 ms
  if (tempo_atual - tempo_tarefa_aprox > periodo_tarefa_aprox) {
    tempo_tarefa_aprox = tempo_atual;

    //Le as informacoes do sensor, em cm e pol
    float cmMsec, inMsec;
    long microsec = 0;//ultrasonic.Timing();

    //Exibe informacoes no serial monitor
    //Serial.print("Distancia em cm: ");
    //Serial.print(cmMsec);
    //Serial.print(" - Distancia em polegadas: ");
    //Serial.println(inMsec);
  }
}
