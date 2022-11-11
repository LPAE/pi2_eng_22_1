//bibliotecas para o teclado
#include <Password.h>
#include <Keypad.h>

//biblioteca para o RFID
#include <SoftwareSerial.h>

//bibliotecas para o display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//biblioteca para RTC
#include <virtuabotixRTC.h>
#include <EEPROM.h>

// Define o pino do rele e buzzer
#define rele 2
#define buzzer 11

// Display
#define endereco  0x27
#define colunas   16
#define linhas    2

// Define pinos RTC
#define   clk         28
#define   dat         26
#define   rst         24

// Define valores iniciais para RTC
#define   segL       00
#define   minL       21
#define   horL       18
#define semana       3
#define   d_mesL     18
#define   mesL        7
#define   anoL     2022

//variáves EEPRON

#define espaco_eepron 1000

int contador;

int dia1;
int mes1;
int hora1;
int minutos1;

int dia2;
int mes2;
int hora2;
int minutos2;

int dia3;
int mes3;
int hora3;
int minutos3;

int dia4;
int mes4;
int hora4;
int minutos4;

int dia5;
int mes5;
int hora5;
int minutos5;

//declara objeto para o display
LiquidCrystal_I2C lcd(endereco, colunas, linhas);

//declara objeto para o RTC
virtuabotixRTC   myRTC(clk, dat, rst);

// declara o botão saida
int buttonsaida = 13;

// declara botão do display
int buttondisplay = 52;

//dados teclado
Password senha = Password("0000");      // Senha para liberação de acesso
Password musica = Password("0101");

// Define número de linhas e colunas do teclado
const byte linha = 4;
const byte coluna = 4;

//Define as notas musicais do buzzer
const int f = 349;
const int gS = 415;
const int a = 440;
const int cH = 523;
const int eH = 659;
const int fH = 698;

int counter = 0;

// Relaciona linha e colunas para determinação dos caracteres do teclado
char keys[linha][coluna] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Declara os pinos de interpretação das linha do teclado
byte pinolinha[linha] = {10, 9, 8, 7};
byte pinocoluna[coluna] = {6, 5, 4, 3};
Keypad keypad = Keypad(makeKeymap(keys), pinolinha, pinocoluna, linha, coluna);

//Declara dados para a leitura da tag
unsigned tag = 0;

int Guido = 3483478;
int Bruno = 5172814;

const int BUFFER_SIZE = 14;
const int DATA_SIZE = 10;
const int DATA_VERSION_SIZE = 2;
const int DATA_TAG_SIZE = 8;
const int CHECKSUM_SIZE = 2;

const unsigned long WAIT_TIME = 3000;

unsigned long Time = 0;
unsigned long LastRead = 0;

SoftwareSerial ssrfid = SoftwareSerial(12, 15);
uint8_t buffer[BUFFER_SIZE];
int buffer_index = 0;

void setup() {
  pinMode(rele, OUTPUT);               // Declara o pino do rele como um pino de Saída de sinal
  pinMode(buzzer, OUTPUT);             // Declara o pino do buzzer como um pino de Saída de sinal
  pinMode(buttonsaida, INPUT_PULLUP);  // Declara o pino de abertura da porta como um pino de Entrada de sinal
  pinMode(buttondisplay, INPUT_PULLUP);// Declara botao para ligar display
  digitalWrite(buzzer, HIGH);          // Inicializa o pino do buzzer com valor alto
  Serial.begin(9600);
  keypad.addEventListener(keypadEvent);
  keypad.setDebounceTime(5);           // Tempo de atraso para leitura das teclas.

  ssrfid.begin(9600);
  ssrfid.listen();
  lcd.init(); // Inicia a comunicação com o display

  

//Limpeza da EEPROM e set de dados do RTC(Carregar o arquivo primeiramente com essa parte para resetar a memória EEPROM e setar dos dados do RTC, depois carregar novamente com os dois comentados)

  //for (int nL = 0; nL < espaco_eepron; nL++) {
  //EEPROM.write(nL, 0);
  //}
  //myRTC.setDS1302Time(segL, minL, horL, semana, d_mesL, mesL, anoL);

  

//Declara os espaços na memária EEPROM que as váriaveis estão salvas
  contador = EEPROMReadInt(0);
  dia1 = EEPROMReadInt(2);
  mes1 = EEPROMReadInt(4);
  hora1 = EEPROMReadInt(6);
  minutos1 = EEPROMReadInt(8);
  dia2  = EEPROMReadInt(10);
  mes2 = EEPROMReadInt(12);
  hora2 = EEPROMReadInt(14);
  minutos2 = EEPROMReadInt(16);
  dia3 = EEPROMReadInt(18);
  mes3 = EEPROMReadInt(20);
  hora3 = EEPROMReadInt(22);
  minutos3 = EEPROMReadInt(24);
  dia4 = EEPROMReadInt(26);
  mes4 = EEPROMReadInt(28);
  hora4 = EEPROMReadInt(30);
  minutos4 = EEPROMReadInt(32);
  dia5 = EEPROMReadInt(34);
  mes5 = EEPROMReadInt(36);
  hora5 = EEPROMReadInt(38);
  minutos5 = EEPROMReadInt(40);
}

void loop() {
  keypad.getKey();

  botaodesaida();

  sensor_tag();

  button1();
}

// Função de ler e escrever na EEPROM
int EEPROMReadInt(int address) {
  byte hiByte = EEPROM.read(address);
  byte loByte = EEPROM.read(address + 1);

  return word(hiByte, loByte);
}
void EEPROMWriteInt(int address, int value) {
  byte hiByte = highByte(value);
  byte loByte = lowByte(value);

  EEPROM.write(address, hiByte);
  EEPROM.write(address + 1, loByte);
}

//Função do botão de saida
void botaodesaida() {
  if (digitalRead(buttonsaida) == LOW) // Se o botão for pressionado
  {
    digitalWrite(rele, HIGH);
    delay(2000);
    digitalWrite(rele, LOW);
  }
}

// TECLADO Realiza a leitura das teclas pressionadas e aguarda confirmação para verificar
void keypadEvent(KeypadEvent eKey) {
  switch (keypad.getState()) {
    case PRESSED:
      Serial.print("Digitado: ");
      Serial.println(eKey);
      digitalWrite(buzzer, LOW);
      delay(50);
      digitalWrite(buzzer, HIGH);
      switch (eKey) {
        case '*': verificasenha();
          break;
        default: {
            senha.append(eKey);
            musica.append(eKey);
          }
      }
  }
}

// TECLADO Verifica o senha digitada após pressionar *
void verificasenha() {
  Serial.print("Verificando, aguarde... ");
  if (senha.evaluate()) {
    gravar();
    Serial.println("Acionando rele... ");
    play(1);
    digitalWrite(rele, HIGH);
    delay(2000);
    digitalWrite(rele, LOW);
    senha.reset();
  }
  else if (musica.evaluate()) {
    play(0);
    musica.reset();
  }
  else {
    digitalWrite(rele, LOW);
    Serial.println("Senha Invalida !");
    play(2);
    senha.reset();
  }
}

// Função do buzzer
void beep(int note, int duration) {
  tone(buzzer, note, duration);

  delay(duration);

  digitalWrite(buzzer, HIGH);

  delay(50);

  counter++;
}

// Função saida do buzzer
void play(int n) {

  if (n == 0) {
    beep(a, 500);
    beep(a, 500);
    beep(a, 500);
    beep(f, 350);
    beep(cH, 150);
    beep(a, 500);
    beep(f, 350);
    beep(cH, 150);
    beep(a, 650);

    digitalWrite(buzzer, HIGH);
    delay(500);

    beep(eH, 500);
    beep(eH, 500);
    beep(eH, 500);
    beep(fH, 350);
    beep(cH, 150);
    beep(gS, 500);
    beep(f, 350);
    beep(cH, 150);
    beep(a, 650);

    digitalWrite(buzzer, HIGH);
    delay(500);
  }

  else if (n == 1) {
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH);
  }
  else if (n == 2) {
    tone(buzzer, 500);
    delay(500);
    noTone(buzzer);
    delay(50);
    tone(buzzer, 1500);
    delay(500);
    noTone(buzzer);
    delay(500);
    digitalWrite(buzzer, HIGH);
  }
}


// Funções do RFID
void sensor_tag() {

  Time = millis();

  if (ssrfid.available() > 0) {
    bool call_extract_tag = false;

    int ssvalue = ssrfid.read();
    if (ssvalue == -1) {
      return;
    }
    if (ssvalue == 2) {
      buffer_index = 0;
    }
    else if (ssvalue == 3) {
      call_extract_tag = true;
    }
    if (buffer_index >= BUFFER_SIZE) {
      Serial.println("Error: Buffer overflow detected!");
      return;
    }

    buffer[buffer_index++] = ssvalue;
    if (call_extract_tag == true) {
      if (buffer_index == BUFFER_SIZE) {
        if (Time >  LastRead + WAIT_TIME) {
          tag = extract_tag();
          tag_open();
          LastRead = Time;
        }
      }
      else {
        buffer_index = 0;
        return;
      }
    }
  }
}

unsigned extract_tag() {
  uint8_t msg_head = buffer[0];
  uint8_t *msg_data = buffer + 1;
  uint8_t *msg_data_version = msg_data;
  uint8_t *msg_data_tag = msg_data + 2;
  uint8_t *msg_checksum = buffer + 11;
  uint8_t msg_tail = buffer[13];

  Serial.println("--------");

  Serial.print("Message-Head: ");
  Serial.println(msg_head);

  Serial.println("Message-Data (HEX): ");
  for (int i = 0; i < DATA_VERSION_SIZE; ++i) {
    Serial.print(char(msg_data_version[i]));
  }
  Serial.println(" (version)");
  for (int i = 0; i < DATA_TAG_SIZE; ++i) {
    Serial.print(char(msg_data_tag[i]));
  }
  Serial.println(" (tag)");

  Serial.print("Message-Checksum (HEX): ");
  for (int i = 0; i < CHECKSUM_SIZE; ++i) {
    Serial.print(char(msg_checksum[i]));
  }
  Serial.println("");

  Serial.print("Message-Tail: ");
  Serial.println(msg_tail);

  Serial.println("--");

  long tag = hexstr_to_value(msg_data_tag, DATA_TAG_SIZE);
  Serial.print("Extracted Tag: ");
  Serial.println(tag);

  long checksum = 0;
  for (int i = 0; i < DATA_SIZE; i += CHECKSUM_SIZE) {
    long val = hexstr_to_value(msg_data + i, CHECKSUM_SIZE);
    checksum ^= val;
  }
  Serial.print("Extracted Checksum (HEX): ");
  Serial.print(checksum, HEX);
  if (checksum == hexstr_to_value(msg_checksum, CHECKSUM_SIZE)) {
    Serial.print(" (OK)");
  } else {
    Serial.print(" (NOT OK)");
  }

  Serial.println("");
  Serial.println("--------");

  return tag;
}

long hexstr_to_value(char *str, unsigned int length) {
  char* copy = malloc((sizeof(char) * length) + 1);
  memcpy(copy, str, sizeof(char) * length);
  copy[length] = '\0';

  long value = strtol(copy, NULL, 16);
  free(copy);
  return value;
}

void tag_open() {
  if (tag == Guido) {
    play(1);
    digitalWrite(rele, HIGH);
    delay(2000);
    digitalWrite(rele, LOW);
    tag = 0;
    gravar();
  }
  else if (tag != 0) {
    digitalWrite(rele, LOW);
    play(2);
  }
}


//Função do display
void button1() {
  if (digitalRead(buttondisplay) == LOW) // Se o botão for pressionado
  {
    lcd.backlight(); // LIGA A ILUMINAÇÃO DO DISPLAY
    lcd.clear(); // LIMPA O DISPLAY
    lcd.setCursor(0, 0); 
    lcd.print("Ultimos acessos:");
    lcd.setCursor(0, 1); 
    lcd.print("1)");
    lcd.setCursor(2, 1); 
    lcd.print(dia1);
    lcd.setCursor(4, 1);
    lcd.print("/");
    lcd.setCursor(5, 1); 
    lcd.print(mes1);
    lcd.setCursor(8, 1); 
    lcd.print("as");
    lcd.setCursor(11, 1); 
    lcd.print(hora1);
    lcd.setCursor(13, 1); 
    lcd.print(":");
    lcd.setCursor(14, 1); 
    lcd.print(minutos1);
    delay(3000);

    lcd.clear(); // LIMPA O DISPLAY
    lcd.setCursor(0, 0);
    lcd.print("2)");
    lcd.setCursor(2, 0);
    lcd.print(dia2);
    lcd.setCursor(4, 0);
    lcd.print("/");
    lcd.setCursor(5, 0);
    lcd.print(mes2);
    lcd.setCursor(8, 0);
    lcd.print("as");
    lcd.setCursor(11, 0);
    lcd.print(hora2);
    lcd.setCursor(13, 0);
    lcd.print(":");
    lcd.setCursor(14, 0);
    lcd.print(minutos2);
    lcd.setCursor(0, 1);
    lcd.print("3)");
    lcd.setCursor(2, 1);
    lcd.print(dia3);
    lcd.setCursor(4, 1);
    lcd.print("/");
    lcd.setCursor(5, 1); 
    lcd.print(mes3);
    lcd.setCursor(8, 1); 
    lcd.print("as");
    lcd.setCursor(11, 1);
    lcd.print(hora3);
    lcd.setCursor(13, 1); 
    lcd.print(":");
    lcd.setCursor(14, 1);
    lcd.print(minutos3);
    delay(3000);

    lcd.clear(); // LIMPA O DISPLAY
    lcd.setCursor(0, 0); 
    lcd.print("4)");
    lcd.setCursor(2, 0); 
    lcd.print(dia4);
    lcd.setCursor(4, 0); 
    lcd.print("/");
    lcd.setCursor(5, 0); 
    lcd.print(mes4);
    lcd.setCursor(8, 0);
    lcd.print("as");
    lcd.setCursor(11, 0);
    lcd.print(hora4);
    lcd.setCursor(13, 0);
    lcd.print(":");
    lcd.setCursor(14, 0);
    lcd.print(minutos4);
    lcd.setCursor(0, 1); 
    lcd.print("5)");
    lcd.setCursor(2, 1);
    lcd.print(dia5);
    lcd.setCursor(4, 1);
    lcd.print("/");
    lcd.setCursor(5, 1); 
    lcd.print(mes5);
    lcd.setCursor(8, 1); 
    lcd.print("as");
    lcd.setCursor(11, 1);
    lcd.print(hora5);
    lcd.setCursor(13, 1);
    lcd.print(":");
    lcd.setCursor(14, 1);
    lcd.print(minutos5);
    delay(3000);
    lcd.clear(); // LIMPA O DISPLAY
    lcd.setBacklight(0);
  }
}

// Função para salvar os dados de dia, mes, hora e minutos na EEPROM
void gravar() {
  contador++;
  EEPROMWriteInt(0, contador);
  switch (contador) {

    case 1:
      myRTC.updateTime();

      dia1 = myRTC.dayofmonth;
      mes1 = myRTC.month;
      hora1 = myRTC.hours;
      minutos1 = myRTC.minutes;

      EEPROMWriteInt(2, dia1);
      EEPROMWriteInt(4, mes1);
      EEPROMWriteInt(6, hora1);
      EEPROMWriteInt(8, minutos1);

      break;

    case 2:
      myRTC.updateTime();

      dia2 = myRTC.dayofmonth;
      mes2 = myRTC.month;
      hora2 = myRTC.hours;
      minutos2 = myRTC.minutes;

      EEPROMWriteInt(10, dia2);
      EEPROMWriteInt(12, mes2);
      EEPROMWriteInt(14, hora2);
      EEPROMWriteInt(16, minutos2);

      break;

    case 3:
      myRTC.updateTime();

      dia3 = myRTC.dayofmonth;
      mes3 = myRTC.month;
      hora3 = myRTC.hours;
      minutos3 = myRTC.minutes;

      EEPROMWriteInt(18, dia3);
      EEPROMWriteInt(20, mes3);
      EEPROMWriteInt(22, hora3);
      EEPROMWriteInt(24, minutos3);

      break;

    case 4:
      myRTC.updateTime();

      dia4 = myRTC.dayofmonth;
      mes4 = myRTC.month;
      hora4 = myRTC.hours;
      minutos4 = myRTC.minutes;

      EEPROMWriteInt(26, dia4);
      EEPROMWriteInt(28, mes4);
      EEPROMWriteInt(30, hora4);
      EEPROMWriteInt(32, minutos4);

      break;

    case 5:
      myRTC.updateTime();

      dia5 = myRTC.dayofmonth;
      mes5 = myRTC.month;
      hora5 = myRTC.hours;
      minutos5 = myRTC.minutes;

      EEPROMWriteInt(34, dia5);
      EEPROMWriteInt(36, mes5);
      EEPROMWriteInt(38, hora5);
      EEPROMWriteInt(40, minutos5);

      contador = 0;
      EEPROMWriteInt(0, contador);

      break;
  }
}
