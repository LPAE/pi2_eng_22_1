/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   Instituto Federal de Santa Catarina - Fpolis
   Projeto: Detecção de vazamento de água
   Alunos: Greicili dos Santos Ferreira e Matheus Locatelli
   Curso: Engenharia Eletrônica
   Disciplina: Projeto Integrador II
   Professores: Luiz Alberto de Azevedo e Renan Augusto Starke
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


//ESP CENTRAL
#include <ESP8266WiFi.h>
#include <espnow.h>

//Parâmetros para se conectar ao servidor do Blynk
#define BLYNK_TEMPLATE_ID "TMPLoBv0ciij"
#define BLYNK_DEVICE_NAME "Sensores de Fluxo"
#define BLYNK_AUTH_TOKEN "osru6BIva4kLW7345Y2EzU_44fBH7F55"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

//Definindo os pinos
#define BUZZER_PIN  D1 // Pino do buzzer
#define DESLIGA_BUZZER D2 //Pino com botão para desligar buzzer

//Autenticação no Blynk
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "arduino";
char pass[] = "esp-8266";

uint8_t broadcastAddress[] = {0x5C, 0xCF, 0x7F, 0x11, 0x67, 0xB0}; // Endereço MAC do ESP-SENSOR

//Definindo variáveis
bool buzzer; // Armazenar estado do botão de acionamento da válvula no Blynk
unsigned long tempo_inicial_reativar = 0; //Armazenar o tempo em que foi desligado o alarme
float tempo_reativar = 60000; // Definiu-se 1 min para reativar o acionamento do alarme
bool estado = 0; //Verificar se o buzzer já foi acionado com a detecção do vazamento

/*Para enviar*/
bool desativar = 0; //Informar se o usuário desativou o alarme
float tempo_vazamento = 60000; //Informação do tempo de fluxo constante para detectar vazamento
bool valvula = 0; //Estado da válvula

/*Para receber */
char nome[20];
bool vazamento_detectado = 0;
float vazao_sensor;
float quantidade_vazamento = 0;
float quantidade_vazamento_acumulado = 0;

/*Struct para o envio dos dados*/
typedef struct dados_envio {
  float tempo_vazamento;
  bool valvula;
} dados_envio_t;

/*Struct para o recebimento dos dados*/
typedef struct dados_recebidos {
  char nome[20];
  bool vazamento_detectado;
  float vazao_sensor;
  float quantidade_vazamento;
  float quantidade_vazamento_acumulado;
} dados_recebidos_t;

//Definindo variáveis do tipo de cada structs
dados_envio_t enviados;
dados_recebidos_t recebidos;

//Função chamada quando envia o dado para o ESP-SENSOR
void OnDataSent( uint8_t *mac_addr, uint8_t sendStatus) {
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(sendStatus == 1 ? "Delivery Success" : "Delivery Fail");
}

//Função chamada quando recebe o dado do ESP-SENSOR
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&recebidos, incomingData, sizeof(recebidos));

  strncpy(nome, recebidos.nome, 20);
  vazamento_detectado = recebidos.vazamento_detectado;
  vazao_sensor = recebidos.vazao_sensor;
  quantidade_vazamento = recebidos.quantidade_vazamento;
  quantidade_vazamento_acumulado = recebidos.quantidade_vazamento_acumulado;

  //Envia para o Blynk os dados recebidos do ESP SENSOR
  Blynk.virtualWrite(V0, vazao_sensor);
  Blynk.virtualWrite(V4, quantidade_vazamento);
  Blynk.virtualWrite(V5, vazamento_detectado);
  Blynk.virtualWrite(V8, quantidade_vazamento_acumulado);
}

/* FUNÇÕES PARA RECEBER OS VALORES DOS PINOS VIRTUAIS DEFINIDOS NO BLYNK */

//Desligar o buzzer
BLYNK_WRITE(V6) {
  buzzer = param.asInt();
  if (!buzzer) {
    desativar = 1;
    digitalWrite(BUZZER_PIN, HIGH); // desliga o buzzer
    tempo_inicial_reativar = millis();
  }
}

//Acionamento da válvula
BLYNK_WRITE(V2) {
  valvula = param.asInt();
}

//Tempo de vazamento
BLYNK_WRITE(V3) {
  tempo_vazamento = param.asFloat();
  tempo_vazamento = tempo_vazamento * 60000;
  Serial.println(tempo_vazamento);
}

//Tempo para retornar o funcionamento normal do sistema
BLYNK_WRITE(V1) {
  tempo_reativar = param.asFloat();
  tempo_reativar = tempo_reativar * 60000;
}

void setup() {
  //Inicializando o monior serial
  Serial.begin(115200);

  // Definindo o dispositivo como uma estação WiFi
  WiFi.mode(WIFI_STA);

  // Inicializando o ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Configuração para o ESP-NOW
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);

  // Registrando peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  // Registrando a função que será chamada quando for identificado o recebimento de um dado
  esp_now_register_recv_cb(OnDataRecv);

  //Autenticação no Blynk
  Blynk.begin(auth, ssid, pass);

  //Definindo o tipo dos pinos
  pinMode(BUZZER_PIN, OUTPUT); // Pino em que o buzzer está conectado
  pinMode(DESLIGA_BUZZER, INPUT_PULLUP); //botão para desligar o buzzer

  //Setando alguns valores iniciais
  desativar = 0;
  digitalWrite(BUZZER_PIN, HIGH); //Desligando o buzzer
  Blynk.virtualWrite(V6, 0); //Colocando em OFF o botão referente ao buzzer no Blynk
  Blynk.virtualWrite(V0, 0); //Fluxo
  Blynk.virtualWrite(V4, 0); //Quantidade de vazamento
  Blynk.virtualWrite(V5, 0); //Detecção de vazamento
  Blynk.virtualWrite(V8, 0); //Quantidade de vazamento acumulado
  Blynk.virtualWrite(V2, 0); //Colocando em OFF o botão referente à válvula no Blynk
  Blynk.virtualWrite(V1, 1); // Definindo 1 min para religar o buzzer
  Blynk.virtualWrite(V3, 1); // Definindo 1 minuto para o tempo de vazamento

}

const long periodo_tarefa_envio = 20;
unsigned long tempo_tarefa_envio = millis();

void tarefa_envio_dados() {
  unsigned long tempo_atual_envio = millis();
  if (tempo_atual_envio - tempo_tarefa_envio >= periodo_tarefa_envio) {
    tempo_tarefa_envio = tempo_atual_envio;

    // Set values to send
    enviados.tempo_vazamento = tempo_vazamento;
    enviados.valvula = valvula;
    // Serial.println(tempo_vazamento);
    //Serial.println(valvula);

    // Send message via ESP-NOW
    if (!esp_now_send(broadcastAddress, (uint8_t *) &enviados, sizeof(enviados)))
    {
      //Serial.println("tudo certo");
    }
  }
}

void loop() {
  //Chama a função para envio dos dados para o ESP-SENSOR
  tarefa_envio_dados();

  //Controlar o reacionamento do alarme
  unsigned long tempo_atual_reativar = millis();
  if ((tempo_atual_reativar - tempo_inicial_reativar > tempo_reativar) and desativar)
  {
    desativar = 0;
    estado = 0;
  }
  //Controle do acionamento do Buzzer
  if (vazamento_detectado and !desativar and !estado)
  {
    estado = 1;
    digitalWrite(BUZZER_PIN, LOW);
    Blynk.virtualWrite(V6, 1);
  }

  if (!vazamento_detectado)
  {
    estado = 0;
    digitalWrite(BUZZER_PIN, HIGH);
    Blynk.virtualWrite(V6, 0);
  }
  //Desligar o buzzer através de um botão físico
  if (digitalRead(DESLIGA_BUZZER) == LOW)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    tempo_inicial_reativar = millis();
    desativar = 1;
    Blynk.virtualWrite(V6, 0);
  }

  Blynk.run();
}
