/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   Instituto Federal de Santa Catarina - Fpolis
   Projeto: Detecção de vazamento de água
   Alunos: Greicili dos Santos Ferreira e Matheus Locatelli
   Curso: Engenharia Eletrônica
   Disciplina: Projeto Integrador II
   Professores: Luiz Alberto de Azevedo e Renan Augusto Starke
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <ESP8266WiFi.h>
#include <espnow.h>


uint8_t broadcastAddress[] = {0x18, 0xFE, 0x34, 0xD6, 0x78, 0x4D}; //Endereço MAC do ESP Central

#define RELE D1 // Definindo o pino do Relé
#define flow_pin D2 // Definindo o pino do sensor

//Definindo variáveis
float flowRate;    //Este é o valor que pretende-se calcular o fluxo
volatile int count; // Contagem dos pulsos

//Definindo variáveis para a detecção do vazamento
bool controle_fluxo_anterior = 0;  // Para controlar o início da contagem para detectar vazamento
float flowRate_anterior; // Armazena o fluxo anterior
unsigned long contagem_vazamento; // Tempo de vazamento
unsigned long contagem_vazamento_inicial; //Tempo de inicio do vazamento

/*Para receber */
float tempo_vazamento = 60000; // Definiu-se inicialmente 1 min de fluxo constante para a detecção do vazamento
bool valvula; // Estado da válvula

/*Para enviar*/
char nome[20] = "ESP8266-Sensor"; // Armazenar o nome do sensor
bool vazamento_detectado = 0;
float quantidade_vazamento_acumulado = 0;
float quantidade_vazamento = 0;

/*Struct para o recebimento dos dados entre os ESPs*/
typedef struct dados_recebidos {
  float tempo_vazamento;
  bool valvula;
} dados_recebidos_t;

/*Struct para o envio dos dados entre os ESPs*/
typedef struct dados_envio {
  char nome[20];
  bool vazamento_detectado;
  float vazao_sensor;
  float quantidade_vazamento;
  float quantidade_vazamento_acumulado;
} dados_envio_t;

//Criando uma variável do tipo de cada struct
dados_envio_t enviados;
dados_recebidos_t recebidos;


//Função chamada quando envia o dado para o ESP-Central
void OnDataSent( uint8_t *mac_addr, uint8_t sendStatus) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(sendStatus == 1 ? "Delivery Success" : "Delivery Fail");
}

//Função chamada quando recebe o dado do ESP-Central
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&recebidos, incomingData, sizeof(recebidos));

  tempo_vazamento = recebidos.tempo_vazamento;
  valvula = recebidos.valvula;
}

//Função para a contagem de pulsos do sensor
void ICACHE_RAM_ATTR Flow()
{
  count++;
}

void setup() {
  // Inicializando a porta serial
  Serial.begin(115200);

  // Definindo o dispositivo como uma estação WiFi
  WiFi.mode(WIFI_STA);

  // Inicializando o ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //Configuração para o ESP-NOW; Utiliza-se COMBO para definir que os ESPs recebem e transmitem dados
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);

  // Registrando peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  // Registrando a função que será chamada quando for identificado o recebimento de um dado
  esp_now_register_recv_cb(OnDataRecv);

  //Definindo o tipo dos pinos
  pinMode(RELE, OUTPUT); //Pino em que o rele está conectado
  pinMode(flow_pin, INPUT); //Pino em que o sensor está conectado

  attachInterrupt(digitalPinToInterrupt(flow_pin), Flow, RISING);  //Configura o interruptor 0 (pino D2 do ESP8266) para rodar a função "Flow"
  interrupts();   //Habilita o interrupção no ESP

  //Definindo valores iniciais nas variáveis
  count = 0;
}


const unsigned long periodo_tarefa_fluxo = 1000;
unsigned long tempo_tarefa_fluxo = millis();

/* Tarefa_fluxo: realizar os cálculos do fluxo*/
void tarefa_fluxo() {

  /* Obtém-se o tempo atual */
  unsigned long tempo_atual = millis();

  /* Enviar os dados a cada 1 segundo (periodo da tarefa) */
  if (tempo_atual - tempo_tarefa_fluxo > periodo_tarefa_fluxo) {
    tempo_tarefa_fluxo = tempo_atual;

    //Cálculos matemáticos
    flowRate = (count * 2.25);        //Conta os pulsos no último segundo e multiplica por 2,25mL, que é a vazão de cada pulso
    flowRate = flowRate * 60;        //Converte segundos em minutos, tornando a unidade de medida mL/min
    flowRate = flowRate / 1000;     //Converte mL em litros, tornando a unidade de medida L/min

    count = 0;      // Reseta o contador para iniciarmos a contagem em 0 novamente

    //flowRate = 36.2;


    //Armazenar valor anterior
    if (controle_fluxo_anterior == 0)
    {
      flowRate_anterior = flowRate;
      contagem_vazamento_inicial = millis();
    }

    //Verifica a presença de um fluxo constante
    if ((flowRate != 0) and (flowRate_anterior - 0.2 < flowRate) and (flowRate < flowRate_anterior + 0.2))
    {
      contagem_vazamento = millis() - contagem_vazamento_inicial;
      controle_fluxo_anterior = 1;
    }
    else
    {
      controle_fluxo_anterior = 0;
      contagem_vazamento = 0;
      vazamento_detectado = 0;
      quantidade_vazamento_acumulado += quantidade_vazamento;
      quantidade_vazamento = 0;
    }
    //Verifica se o tempo de vazamento definido pelo usuário foi alcançado
    if (contagem_vazamento > tempo_vazamento) {
      vazamento_detectado = 1;
      quantidade_vazamento = (flowRate * contagem_vazamento) / 60000;
      //Serial.println("vazamento detectado");
    }
  }
}

const unsigned long periodo_tarefa_envio = 300;
unsigned long tempo_tarefa_envio = millis();

//Função para enviar os dados para o ESP CENTRAL
void tarefa_envio_dados() {
  unsigned long tempo_atual_envio = millis();

  //Enviar os dados ao ESP-Central a cada 300 milissegundos
  if (tempo_atual_envio - tempo_tarefa_envio >= periodo_tarefa_envio) {
    tempo_tarefa_envio = tempo_atual_envio;

    //Definindo os valores a serem enviados
    strncpy(enviados.nome, nome , 20);
    enviados.vazamento_detectado = vazamento_detectado;
    enviados.vazao_sensor = flowRate;
    enviados.quantidade_vazamento = quantidade_vazamento;
    enviados.quantidade_vazamento_acumulado = quantidade_vazamento_acumulado;

    // Enviando os dados via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &enviados, sizeof(enviados));
  }
}

void loop() {
  /* Chama as funções */
  tarefa_fluxo();
  tarefa_envio_dados();

  /* Controle da Válvula */
  if (valvula == 1)
    digitalWrite(RELE, HIGH);

  else
    digitalWrite(RELE, LOW);
}
