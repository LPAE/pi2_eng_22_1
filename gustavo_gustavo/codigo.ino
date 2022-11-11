//Includes:

#include <EEPROM.h>
#include <LiquidCrystal.h>
#include "dht.h"

//defines:

#define EEPROM_DEBUG_ON               //Habilita o debug da EEPROM via serial

#define D4_PIN          4             //Pinos do modulo LCD
#define D5_PIN          5
#define D6_PIN          6
#define D7_PIN          7
#define RS_PIN          8
#define EN_PIN          9
#define BLIGHT_PIN      10

#define RTC_SCL         A5            //Pinos RTC
#define RTC_SDA         A4

#define BUTTON_PIN      A0            //Pino de leitura dos botões (ADC)
#define BUTTON_RIGHT    0             //Codificação da função le_botao
#define BUTTON_UP       1
#define BUTTON_DOWN     2
#define BUTTON_LEFT     3
#define BUTTON_SELECT   4
#define BUTTON_NOTHING  5

#define DHT11_PIN       A2

#define RELAY_PIN       12

#define TAM_NOME        13                                          //Número máximo de caracteres permitidos ao nome dos perfis (incluindo o /n)
#define MAX_PERFIS      20                                          //Número máximo de perfis que podem ser criados
#define EEPROM_HEADER   (1024-(sizeof(perfil_t)*MAX_PERFIS))        //Número de bytes reservados na EEPROM antes do "vetor de perfil_t" salvo

//Typedefs:

typedef enum {CARREGA_P, MONITORA_P, TROCA_P, CONFIGURA_P} estado_t;
typedef void funcao_t();

typedef struct perfil {
 uint8_t status_perfil;    //1 para perfil válido, 0 para perfil removido
 uint8_t end_eeprom;       //Índice do perfil no "vetor perfil_t" da memoria EEPROM
 void *prox_perfil;        //Endereço do próximo perfil alocado na RAM
 void *perfil_ant;         //Endereço do perfil anterior alocado na RAM
 char nome[TAM_NOME];      //Nome atribuido pelo usuário
 uint8_t temperatura;      //Temperatura do perfil em graus Celsius
 uint8_t dom_h_inic;       //Armazena a hora do inicio
 uint8_t dom_min_inic;     //Armazena os minutos do inicio
 uint8_t dom_h_final;      //Armazena a hora do final
 uint8_t dom_min_final;    //Armazena os minutos do final
 uint8_t seg_h_inic;
 uint8_t seg_min_inic;
 uint8_t seg_h_final;
 uint8_t seg_min_final;
 uint8_t ter_h_inic;
 uint8_t ter_min_inic;
 uint8_t ter_h_final;
 uint8_t ter_min_final;
 uint8_t qua_h_inic;
 uint8_t qua_min_inic;
 uint8_t qua_h_final;
 uint8_t qua_min_final;
 uint8_t qui_h_inic;
 uint8_t qui_min_inic;
 uint8_t qui_h_final;
 uint8_t qui_min_final;
 uint8_t sex_h_inic;
 uint8_t sex_min_inic;
 uint8_t sex_h_final;
 uint8_t sex_min_final;
 uint8_t sab_h_inic;
 uint8_t sab_min_inic;
 uint8_t sab_h_final;
 uint8_t sab_min_final;
} perfil_t;

//Declaração de funções:

void inic_sistema(void);            //Função de Inicialização

void carrega_p (void);              //Funções de Estado
void monitora_p (void);
void troca_p (void);
void configura_p (void);

perfil_t *cria_perfil(perfil_t *perfil_dados);      //Funções de gerenciamento de perfis na RAM
void exclui_perfil(perfil_t *perfil);
void limpa_perfis(void);

void atualiza_mapa_eeprom (void);                   //Funções de acesso a EEPROM
void salva_perfil_eeprom (perfil_t *perfil);
perfil_t *carrega_perfil_eeprom(uint8_t end_eeprom);

uint8_t le_botao (void);                            //Demais Funções
void atualiza_display (uint8_t estado);

//Declaração de Variáveis Globais:

LiquidCrystal lcd(RS_PIN, EN_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);          //Cria o objeto do display - nomeado lcd
dht DHT;                                                                     //Cria o objeto do DHT - nomeado dht

funcao_t *vetor_de_estados[] = {carrega_p, monitora_p, troca_p, configura_p};

estado_t estado_atual;

perfil_t *perfil_inic, *perfil_final, *perfil_atual;

uint64_t tempo_atual;
uint64_t tempo_display;
uint64_t tempo_botao;
uint64_t tempo_temperatura;

uint8_t total_perfis;
uint32_t mapa_eeprom;

//Código Principal:

void setup() {
 Serial.begin(9600);
 lcd.begin(16, 2);
 pinMode(BLIGHT_PIN, OUTPUT);          //Seta os ports do arduino como input ou output
 pinMode(RELAY_PIN, OUTPUT);
 digitalWrite(BLIGHT_PIN, HIGH);       //Liga o Backlight do Display
 inic_sistema();                       //Inicia os Perfis no sistema
 estado_atual = CARREGA_P;
 perfil_atual = perfil_inic;
 Serial.println(F("Sistema Inicializado"));
}

void loop() {
 (*vetor_de_estados[estado_atual])();
}

//ESTADOS:

void carrega_p (void) //Estado responsável por ler os dados do perfil atual mediante o horario do RTC
{
 lcd.clear();
 lcd.print("CARREGA_P");
 delay(1000);
 estado_atual = MONITORA_P;
}

void monitora_p (void)  //Estado principal do sistema. Faz o monitoramento dos sensores e da interface do usuário e define a transição de estados
{
 uint64_t temperatura;
 uint8_t botao;
 uint8_t i;

tempo_atual = millis();

if ((tempo_atual - tempo_botao) > 1000) {
   botao = le_botao();
   if (botao == BUTTON_RIGHT) {
     estado_atual = TROCA_P;
   } else if (botao == BUTTON_SELECT) {
     estado_atual = CONFIGURA_P;
   } else if (botao == BUTTON_LEFT){
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print(perfil_atual->end_eeprom);
     lcd.print("- ");
     i=0;
     while((perfil_atual->nome[i]) != NULL){
       lcd.write(perfil_atual->nome[i]);
       i++;
     }
     lcd.setCursor(0,1);
     lcd.print(perfil_atual->temperatura);
     lcd.print("C  ");
     lcd.print(perfil_atual->dom_h_inic);
     lcd.setCursor(7,1);
     lcd.print(":");
     lcd.print(perfil_atual->dom_min_inic);
     lcd.setCursor(11,1);
     lcd.print(perfil_atual->dom_h_final);
     lcd.setCursor(13,1);
     lcd.print(":");
     lcd.print(perfil_atual->dom_min_final);
     delay(1000);
   }
   tempo_botao = tempo_atual;
 }else if ((tempo_atual - tempo_display) > 1000) {
   atualiza_display(MONITORA_P);
   tempo_display = tempo_atual;
 }

if ((tempo_atual - tempo_temperatura) > 5000) {
   DHT.read11(DHT11_PIN);
   temperatura = DHT.temperature;
   if (temperatura > 25) {
     digitalWrite(RELAY_PIN, HIGH);
   } else {
     digitalWrite(RELAY_PIN, LOW);
   }
   tempo_temperatura = tempo_atual;
 }
}

void troca_p (void) //Estado responsável pela troca rápida entre perfis (requisito da concepção do sistema)
{
 lcd.clear();
 lcd.print("TROCA_P");
 delay(1000);
 if (perfil_atual->prox_perfil != NULL) {
   perfil_atual = perfil_atual->prox_perfil;
 } else {
   perfil_atual = perfil_inic;
 }
 estado_atual = CARREGA_P;
}

void configura_p (void) //Estado responsável pela criação e exclusão de perfis
{
 uint8_t i;
 uint8_t botao;
 uint8_t letra;
 uint8_t coluna;
 uint8_t nome[TAM_NOME];
 perfil_t perfil_dados;
 perfil_t *perfil_temp;

lcd.clear();
 lcd.print("CIMA: CRIA_P");
 lcd.setCursor(0, 1);
 lcd.print("BAIXO: EXCLUI_P");
 delay(2000);

while (estado_atual == CONFIGURA_P) {
   tempo_atual = millis();
   if ((tempo_atual - tempo_botao) > 1000) {
     botao = le_botao();
     if (botao == BUTTON_UP) {
       lcd.clear();
       coluna = 0;
       letra = 'A';
       lcd.print("DIGITE O NOME:");
       lcd.setCursor(coluna, 1);
       lcd.write(letra);
       delay(2000);

while (estado_atual == CONFIGURA_P) {
         tempo_atual = millis();
         if ((tempo_atual - tempo_botao) > 1000) {
           botao = le_botao();
           if (botao == BUTTON_UP) {
             letra--;
             lcd.setCursor(coluna, 1);
             lcd.write(letra);
           } else if (botao == BUTTON_DOWN) {
             letra++;
             lcd.setCursor(coluna, 1);
             lcd.write(letra);
           } else if (botao == BUTTON_RIGHT) {
             if (coluna < (TAM_NOME - 1)) {
               nome[coluna] = letra;
               coluna++;
               letra = 'A';
               lcd.setCursor(coluna, 1);
               lcd.write(letra);
             }
           } else if (botao == BUTTON_LEFT) {
             if (coluna > 0) {
               lcd.setCursor(coluna, 1);
               lcd.write(" ");
               coluna--;
               letra = 'A';
               lcd.setCursor(coluna, 1);
               lcd.write(letra);
             }
           } else if (botao == BUTTON_SELECT) {
             nome[coluna] = letra;
             nome[coluna + 1] = NULL;
             for (i = 0; i < TAM_NOME; i++) {
               perfil_dados.nome[i] = nome[i];
             }
             lcd.clear();
             coluna = 0;
             letra = 0;
             lcd.print("HORARIO INICIAL:");
             lcd.setCursor(coluna, 1);
             lcd.print(letra);
             delay(2000);

while (estado_atual == CONFIGURA_P) {
               tempo_atual = millis();
               if ((tempo_atual - tempo_botao) > 1000) {
                 botao = le_botao();
                 if (botao == BUTTON_UP) {
                   if (letra > 0) {
                     letra--;
                     lcd.setCursor(coluna, 1);
                     lcd.print(letra);
                   }
                 } else if (botao == BUTTON_DOWN) {
                   if ((coluna == 0) && (letra < 24)) {
                     letra++;
                     lcd.setCursor(coluna, 1);
                     lcd.print(letra);
                   } else if ((coluna == 3) && (letra < 60)) {
                     letra += 5;
                     lcd.setCursor(coluna, 1);
                     lcd.print(letra);
                   }
                 } else if (botao == BUTTON_RIGHT) {
                   if (coluna == 0) {
                     perfil_dados.dom_h_inic = letra;
                     perfil_dados.seg_h_inic = letra;
                     perfil_dados.ter_h_inic = letra;
                     perfil_dados.qua_h_inic = letra;
                     perfil_dados.qui_h_inic = letra;
                     perfil_dados.sex_h_inic = letra;
                     perfil_dados.sab_h_inic = letra;
                     letra = 0;
                     lcd.setCursor(2, 1);
                     lcd.print(":");
                     coluna = 3;
                     lcd.setCursor(coluna, 1);
                     lcd.print(letra);
                   }
                 } else if (botao == BUTTON_LEFT) {
                   if (coluna == 3) {
                     lcd.setCursor(0, 1);
                     lcd.print("     ");
                     coluna = 0;
                     letra = 0;
                     lcd.setCursor(coluna, 1);
                     lcd.print(letra);
                   }
                 } else if (botao == BUTTON_SELECT) {
                   if (coluna == 3) {
                     perfil_dados.dom_min_inic = letra;
                     perfil_dados.seg_min_inic = letra;
                     perfil_dados.ter_min_inic = letra;
                     perfil_dados.qua_min_inic = letra;
                     perfil_dados.qui_min_inic = letra;
                     perfil_dados.sex_min_inic = letra;
                     perfil_dados.sab_min_inic = letra;
                     lcd.clear();
                     coluna = 0;
                     letra = 0;
                     lcd.print("HORARIO FINAL:");
                     lcd.setCursor(coluna, 1);
                     lcd.print(letra);
                     delay(2000);

while (estado_atual == CONFIGURA_P) {
                       tempo_atual = millis();
                       if ((tempo_atual - tempo_botao) > 1000) {
                         botao = le_botao();
                         if (botao == BUTTON_UP) {
                           if (letra > 0) {
                             letra--;
                             lcd.setCursor(coluna, 1);
                             lcd.print(letra);
                           }
                         } else if (botao == BUTTON_DOWN) {
                           if ((coluna == 0) && (letra < 24)) {
                             letra++;
                             lcd.setCursor(coluna, 1);
                             lcd.print(letra);
                           } else if ((coluna == 3) && (letra < 60)) {
                             letra += 5;
                             lcd.setCursor(coluna, 1);
                             lcd.print(letra);
                           }
                         } else if (botao == BUTTON_RIGHT) {
                           if (coluna == 0) {
                             perfil_dados.dom_h_final = letra;
                             perfil_dados.seg_h_final = letra;
                             perfil_dados.ter_h_final = letra;
                             perfil_dados.qua_h_final = letra;
                             perfil_dados.qui_h_final = letra;
                             perfil_dados.sex_h_final = letra;
                             perfil_dados.sab_h_final = letra;
                             letra = 0;
                             lcd.setCursor(2, 1);
                             lcd.print(":");
                             coluna = 3;
                             lcd.setCursor(coluna, 1);
                             lcd.print(letra);
                           }
                         } else if (botao == BUTTON_LEFT) {
                           if (coluna == 3) {
                             lcd.setCursor(0, 1);
                             lcd.print("     ");
                             coluna = 0;
                             letra = 0;
                             lcd.setCursor(coluna, 1);
                             lcd.print(letra);
                           }
                         } else if (botao == BUTTON_SELECT) {
                           if (coluna == 3) {
                             perfil_dados.dom_min_final = letra;
                             perfil_dados.seg_min_final = letra;
                             perfil_dados.ter_min_final = letra;
                             perfil_dados.qua_min_final = letra;
                             perfil_dados.qui_min_final = letra;
                             perfil_dados.sex_min_final = letra;
                             perfil_dados.sab_min_final = letra;
                             lcd.clear();
                             letra = 25;
                             lcd.print("QUAL A TEMPERAT:");
                             lcd.setCursor(0, 1);
                             lcd.print(letra);
                             delay(2000);

while (estado_atual == CONFIGURA_P) {
                               tempo_atual = millis();
                               if ((tempo_atual - tempo_botao) > 1000) {
                                 botao = le_botao();
                                 if (botao == BUTTON_UP) {
                                   letra--;
                                   lcd.setCursor(0, 1);
                                   lcd.print("  ");
                                   lcd.setCursor(0, 1);
                                   lcd.print(letra);
                                 } else if (botao == BUTTON_DOWN) {
                                   letra++;
                                   lcd.setCursor(0, 1);
                                   lcd.print(letra);
                                 } else if (botao == BUTTON_SELECT) {
                                   perfil_dados.temperatura = letra;
                                   lcd.clear();
                                   lcd.print("CRIANDO PERFIL");
                                   lcd.setCursor(0, 1);
                                   lcd.print("SEGUEM OS DADOS");
                                   delay(2000);
                                   lcd.clear();
                                   i = 0;
                                   while ((perfil_dados.nome[i]) != NULL) {
                                     lcd.write(perfil_dados.nome[i]);
                                     i++;
                                   }
                                   lcd.print("  ");
                                   lcd.print(perfil_dados.temperatura);
                                   lcd.print("C");
                                   lcd.setCursor(0, 1);
                                   lcd.print(perfil_dados.dom_h_inic);
                                   lcd.setCursor(2, 1);
                                   lcd.print(":");
                                   lcd.print(perfil_dados.dom_min_inic);
                                   lcd.setCursor(8, 1);
                                   lcd.print(perfil_dados.dom_h_final);
                                   lcd.setCursor(10, 1);
                                   lcd.print(":");
                                   lcd.print(perfil_dados.dom_min_final);

delay(5000);

cria_perfil(&perfil_dados);

estado_atual = CARREGA_P;
                                 }
                                 tempo_botao = tempo_atual;
                               }
                             }
                           }
                         }
                         tempo_botao = tempo_atual;
                       }
                     }
                   }
                 }
                 tempo_botao = tempo_atual;
               }
             }
           }
           tempo_botao = tempo_atual;
         }
       }
     } else if (botao == BUTTON_DOWN) {
       lcd.clear();
       perfil_temp = perfil_inic;
       lcd.print("QUAL EXCLUIR:");
       lcd.setCursor(0, 1);
       lcd.print(perfil_temp->end_eeprom);
       lcd.print("-");
       i = 0;
       while ((perfil_temp->nome[i]) != NULL) {
         lcd.write(perfil_temp->nome[i]);
         i++;
       }
       delay(2000);

while (estado_atual == CONFIGURA_P) {
         tempo_atual = millis();
         if ((tempo_atual - tempo_botao) > 1000) {
           botao = le_botao();
           if (botao == BUTTON_UP) {
             if (perfil_temp->perfil_ant != NULL) {
               perfil_temp = perfil_temp->perfil_ant;
               lcd.setCursor(0, 1);
               lcd.print("                ");
               lcd.setCursor(0, 1);
               lcd.print(perfil_temp->end_eeprom);
               lcd.print("-");
               i = 0;
               while ((perfil_temp->nome[i]) != NULL) {
                 lcd.write(perfil_temp->nome[i]);
                 i++;
               }
             }
           } else if (botao == BUTTON_DOWN) {
             if (perfil_temp->prox_perfil != NULL) {
               perfil_temp = perfil_temp->prox_perfil;
               lcd.setCursor(0, 1);
               lcd.print("                ");
               lcd.setCursor(0, 1);
               lcd.print(perfil_temp->end_eeprom);
               lcd.print("-");
               i = 0;
               while ((perfil_temp->nome[i]) != NULL) {
                 lcd.write(perfil_temp->nome[i]);
                 i++;
               }
             }
           } else if (botao == BUTTON_SELECT) {
             lcd.clear();
             lcd.print("EXCLUINDO PERFIL:");
             lcd.setCursor(0, 1);
             lcd.print(perfil_temp->end_eeprom);
             lcd.print("-");
             i = 0;
             while ((perfil_temp->nome[i]) != NULL) {
               lcd.write(perfil_temp->nome[i]);
               i++;
             }
             delay(5000);

exclui_perfil(perfil_temp);

estado_atual = CARREGA_P;
           }
           tempo_botao = tempo_atual;
         }
       }
     }
     tempo_botao = tempo_atual;
   }
 }
}

//Funções:

/*
  Função de Inicilização do sistema

Realiza a leitura de todos os dados salvos na EEPROM, inicializando a RAM com os perfis e ajustando as variáveis globais do código
*/

void inic_sistema(void)
{
 uint8_t i;
 perfil_t *perfil_temp, *perfil_ant;

perfil_ant = NULL;
 perfil_inic = NULL;
 total_perfis = 0;
 mapa_eeprom = 0;

for (i = 0; i < MAX_PERFIS; i++) {
   perfil_temp = carrega_perfil_eeprom(i);
   if ((perfil_temp->status_perfil) == 1) {
     if (total_perfis == 0) {
       perfil_inic = perfil_temp;
       perfil_inic->perfil_ant = NULL;
     }
     total_perfis++;
     mapa_eeprom = mapa_eeprom | (0b01 << i);          //cria o mapa_eeprom, com 1 nos endereços ocupados e 0 nos endereços preenchidos
     if (perfil_ant != NULL) {
       perfil_ant->prox_perfil = perfil_temp;
       perfil_temp->perfil_ant = perfil_ant;
     }
     perfil_ant = perfil_temp;
   }
 }
 perfil_final = perfil_ant;
 perfil_final->prox_perfil = NULL;
}
/*
  Função para a criação de novos perfis

Realiza a troca de interface com o usuário, realiza a leitura dos dados inseridos pelo usuário para criação do perfil
  Faz a alocação dinâmica de memoria RAM para o perfil criado e salva tais dados em uma região vazia da EEPROM
*/

perfil_t *cria_perfil(perfil_t *perfil_dados)
{
 uint8_t i, temperatura, h_inic, min_inic, h_final, min_final;
 perfil_t *perfil;

if (total_perfis > MAX_PERFIS) {      //Permite apenas a criação de 20 perfis
   //faz algo com o buzzer
   //envia algo pela serial
   return NULL;
 }

perfil = (perfil_t*)malloc(sizeof(perfil_t));     //Aloca o perfil na RAM
 if (perfil == NULL) {
   //faz algo com o buzzer
   //envia algo pela serial
   return NULL;
 }

if ((perfil_inic == NULL) || (perfil_final == NULL)) {          //verifica se já existe alguma estrutura de perfis na RAM;
   inic_sistema();                                               //tenta corrigir a estrutura lendo a EEPROM
 }
 if (perfil_final == NULL) {                         //caso não exista nada na EEPROM, cria uma estrutura do zero
   perfil_inic = perfil;
   perfil_final = perfil;
   perfil->perfil_ant = NULL;
   perfil->prox_perfil = NULL;
 } else {
   perfil->perfil_ant = perfil_final;              //caso já exista, inclui o novo perfil no final da estrutura da RAM
   perfil_final->prox_perfil = perfil;
   perfil->prox_perfil = NULL;
   perfil_final = perfil;
 }

for (i = 0; i < TAM_NOME; i++) {            //Insere os dados do perfil na estrutura
   perfil->nome[i] = perfil_dados->nome[i];
 }
 perfil->status_perfil = 1;
 perfil->temperatura = perfil_dados->temperatura;
 perfil->dom_h_inic = perfil_dados->dom_h_inic;
 perfil->dom_min_inic = perfil_dados->dom_min_inic;
 perfil->dom_h_final = perfil_dados->dom_h_final;
 perfil->dom_min_final = perfil_dados->dom_min_final;
 perfil->seg_h_inic = perfil_dados->seg_h_inic;
 perfil->seg_min_inic = perfil_dados->seg_min_inic;
 perfil->seg_h_final = perfil_dados->seg_h_final;
 perfil->seg_min_final = perfil_dados->seg_min_final;
 perfil->ter_h_inic = perfil_dados->ter_h_inic;
 perfil->ter_min_inic = perfil_dados->ter_min_inic;
 perfil->ter_h_final = perfil_dados->ter_h_final;
 perfil->ter_min_final = perfil_dados->ter_min_final;
 perfil->qua_h_inic = perfil_dados->qua_h_inic;
 perfil->qua_min_inic = perfil_dados->qua_min_inic;
 perfil->qua_h_final = perfil_dados->qua_h_final;
 perfil->qua_min_final = perfil_dados->qua_min_final;
 perfil->qui_h_inic = perfil_dados->qui_h_inic;
 perfil->qui_min_inic = perfil_dados->qui_min_inic;
 perfil->qui_h_final = perfil_dados->qui_h_final;
 perfil->qui_min_final = perfil_dados->qui_min_final;
 perfil->sex_h_inic = perfil_dados->sex_h_inic;
 perfil->sex_min_inic = perfil_dados->sex_min_inic;
 perfil->sex_h_final = perfil_dados->sex_h_final;
 perfil->sex_min_final = perfil_dados->sex_min_final;
 perfil->sab_h_inic = perfil_dados->sab_h_inic;
 perfil->sab_min_inic = perfil_dados->sab_min_inic;
 perfil->sab_h_final = perfil_dados->sab_h_final;
 perfil->sab_min_final = perfil_dados->sab_min_final;

total_perfis++;                   //Atualiza o total de perfis

salva_perfil_eeprom(perfil);      //Salva o perfil criado na EEPROM

return perfil;
}

/*
  Função para exclusão dos perfis criados

Realiza a correção dos ponteiros da lista encadeada de perfis e também seta o status do perfil para 0 (desativado)
*/

void exclui_perfil(perfil_t *perfil)
{
 perfil_t *perfil_ant, *prox_perfil;

if (perfil == NULL) {
   //faz algo com o buzzer
   //envia algo pela serial
   return;
 }

perfil_ant = perfil->perfil_ant;                          //carrega o perfil anterior e proximo perfil apontados pelo perfil a ser excluido
 prox_perfil = perfil->prox_perfil;

perfil_ant->prox_perfil = prox_perfil;                    //faz o perfil anterior ao perfil excluido apontar para o perfil sucessor, e vice versa
 prox_perfil->perfil_ant = perfil_ant;

if (perfil == perfil_inic) {
   perfil_inic = perfil_inic->prox_perfil;
 }

if (perfil == perfil_final) {
   perfil_final = perfil_final->perfil_ant;
 }

total_perfis--;

exclui_perfil_eeprom(perfil);

free(perfil);                                             //libera a RAM alocada para a estrutura do perfil excluido
}

/*
  Função para eliminação de todos os perfis

Seta o status de todos os perfis para 0 (removido)
*/

void limpa_perfis(void)
{
 uint8_t i;

perfil_t *perfil_temp1, *perfil_temp2;

perfil_temp1 = perfil_inic;

while (perfil_temp1 != NULL) {                        //Libera os perfis da RAM
   perfil_temp2 = perfil_temp1;
   perfil_temp1 = perfil_temp1->prox_perfil;
   free(perfil_temp2);
 }

total_perfis = 0;                                     //Ajusta as variáveis globais
 mapa_eeprom = 0;
 perfil_inic = NULL;
 perfil_final = NULL;

for (i = 0; i < MAX_PERFIS; i++) {                        //Seta o bit de status de todos os perfis da EEPROM para 0 (perfil removido)
   EEPROM.write((sizeof(perfil_t)*i) + EEPROM_HEADER, 0);
 }
}

/*
  Função de gerenciamento da EEPROM

Recebe uma estrutura do tipo perfil_t e a armazena na EEPROM
  Realiza a atualização do mapa da EEPROM na variável global
*/

void salva_perfil_eeprom (perfil_t *perfil)
{
 uint8_t i, end_eeprom;
 uint32_t mapa_temp = mapa_eeprom;

if (perfil == NULL) {
   //faz algo com o buzzer
   //envia algo pela serial
   return;
 }

end_eeprom = 0;
 while (mapa_temp & 0b01) {                  //Verifica se o LSB do mapa_eeprom é 0 (par), o que representa que o endereço "end_eeprom" da EEPROM está vazio
   mapa_temp = mapa_temp >> 1;               //Joga o próximo endereço da EEPROM para o LSB
   end_eeprom++;
 }

mapa_eeprom = mapa_eeprom | (0b01 << end_eeprom);     //Seta para 1 o endereço do mapa_eeprom na qual o perfil será salvo

perfil->end_eeprom = end_eeprom;                      //Salva o endereço EEPROM na estrutura do perfil

for (i = 0; i < sizeof(perfil_t); i++) {
   EEPROM.write((end_eeprom * sizeof(perfil_t)) + EEPROM_HEADER + i, ((uint8_t*)perfil)[i]);    //Salva perfil na EEPROM, byte a byte
 }

#ifdef EEPROM_DEBUG_ON
 Serial.print("Salva EEPROM: ");
 Serial.println(perfil->end_eeprom);
 Serial.print("Mapa EEPROM: ");
 Serial.println(mapa_eeprom, BIN);
#endif
}

/*
  Função de gerenciamento da EEPROM

Recebe um índice (endereço no vetor de perfil_t) do perfil a ser carregado
  Realiza a alocação dinâmica de memoria RAM para o perfil com os dados extraidos da EEPROM
*/

perfil_t *carrega_perfil_eeprom(uint8_t end_eeprom)
{
 uint8_t i;
 perfil_t *perfil;

perfil = (perfil_t*)malloc(sizeof(perfil_t));
 if (perfil == NULL) {
   //faz algo com o buzzer
   //envia algo pela serial
   return NULL;
 }

for (i = 0; i < sizeof(perfil_t); i++) {
   ((uint8_t*)perfil)[i] = EEPROM.read((end_eeprom * sizeof(perfil_t)) + EEPROM_HEADER + i);  //Carrega o perfil da EEPROM, byte a byte
 }

#ifdef EEPROM_DEBUG_ON
 Serial.print("Carrega EEPROM: ");
 Serial.println(end_eeprom);
 Serial.print("Mapa EEPROM: ");
 Serial.println(mapa_eeprom, BIN);
#endif

return perfil;
}

/*
  Função de Gerenciamento da EEPROM

Recebe uma estrutura de um perfil a ser removido e seta seu status para 0 (dasativado)
*/

void exclui_perfil_eeprom(perfil_t *perfil)
{
 uint32_t mask;

if (perfil == NULL) {
   //faz algo com o buzzer
   //envia algo pela serial
   return;
 }

EEPROM.write((sizeof(perfil_t) * (perfil->end_eeprom)) + EEPROM_HEADER, 0);   //seta o bit de status do perfil para 0 (perfil removido)

mask = 0b01 << (perfil->end_eeprom);          //coloca 1 no bit a ser zerado do mapa_eeprom (e 0 nos demais)
 mapa_eeprom = mapa_eeprom & (~mask);          //zera o bit do mapa_eeprom do endereço removido (o operador "~" equivale ao NOT)

#ifdef EEPROM_DEBUG_ON
 Serial.print("Exclui EEPROM: ");
 Serial.println(perfil->end_eeprom);
 Serial.print("Mapa EEPROM: ");
 Serial.println(mapa_eeprom, BIN);
#endif
}

void atualiza_display (uint8_t estado)
{
 float temperatura;
 lcd.clear();
 switch (estado) {
   case MONITORA_P:
     lcd.print("MONITORA_P");
     lcd.setCursor(13, 0);
     DHT.read11(DHT11_PIN);
     temperatura = DHT.temperature;
     lcd.print(temperatura);
     lcd.setCursor(15,0);
     lcd.print("C");
     lcd.setCursor(0, 1);        //Pula linha
     lcd.print(perfil_atual->end_eeprom);
     lcd.print("-");
     lcd.print(perfil_atual->nome);
     break;
 }
}

/*
  Função para leitura do botão pressionado no shield LCD

Retorna:
  0 - Direita
  1 - Cima
  2 - Baixo
  3 - Esquerda
  4 - Select
  5 - Nenhuma tecla pressionada

*/

uint8_t le_botao (void)
{
 uint8_t i;
 uint16_t leitura_adc;
 uint16_t limites_botao[] = {50, 150, 350, 500, 750, 1024};

leitura_adc = analogRead(BUTTON_PIN);

i = 0;

while (leitura_adc > limites_botao[i]) {
   i++;
 }

return i;
}

