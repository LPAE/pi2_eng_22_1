// Inclui a biblioteca
#include <Wire.h>
// Inclui a biblioteca
#include <LiquidCrystal_I2C.h>
// Define o endereço LCD para 0x27 para um display de 16 caracteres e 2 linhas
LiquidCrystal_I2C lcd(0x27, 16, 2);

//#define reset 9 
#define mq2 A0
#define mq135 A1
#define buz 13
#define cooler 3
#define relay 6   
#define led 11
#define MAX_GAS 200

int gas_level_2;
int gas_level_135;
  
bool mq2_active = false;
bool mq135_active = false;

void setup() {
 
   Serial.begin(9600);

    pinMode(buz, OUTPUT);
    pinMode(cooler,OUTPUT);
    pinMode(relay,OUTPUT);
    pinMode(led,OUTPUT);
  
    // inicializa o LCD
    lcd.init();
    //liga a luz de fundo
    lcd.backlight();
    // Posição do cursor
    lcd.setCursor(0, 0);
    // Imprime uma mensagem no LCD
    lcd.print("MQ2:");
    // Posição do cursor
    lcd.setCursor(0, 1);
    // Imprime uma mensagem no LCD
    lcd.print("MQ135:");
  
 }


 const unsigned long periodo_mq2 = 2000;
 unsigned long tempo_mq2 = millis();

void tarefa_mq2(){
    
    unsigned long tempo_atual_mq2 = millis ();
    
  if(tempo_atual_mq2 - tempo_mq2 > periodo_mq2) {
    tempo_mq2 = tempo_atual_mq2;
    
      // Faz a leitura do sensor
     
     gas_level_2 = analogRead(mq2);
     lcd.setCursor(6,0);
     lcd.print(gas_level_2);
      
//checar valores em relacao a referencia do gas
      
        if(gas_level_2 >= MAX_GAS){
  
          mq2_active = true;
          lcd.setCursor(10,0);
          lcd.print("CRITICO");
      }
        else{
       
           mq2_active = false;
           lcd.setCursor(10,0);
           lcd.print("NORMAL");
      }
// Envia para o computador (serial) os dados
      Serial.print("Leitura MQ2: ");
      Serial.println(gas_level_2);
      Serial.println();
  }
}

const unsigned long periodo_mq135 = 2000;
unsigned long tempo_mq135 = millis()+1000;

void tarefa_mq135(){
    
  unsigned long tempo_atual_mq135 = millis ();
    
  if(tempo_atual_mq135 - tempo_mq135 > periodo_mq135) {
      tempo_mq135 = tempo_atual_mq135;

//Faz a leitura do sensor

        gas_level_135 = analogRead(mq135);
        lcd.setCursor(6,1);
        lcd.print(gas_level_135);
        
//checar valores em relacao a referencia do gas

         if(gas_level_135 >= MAX_GAS){
          mq135_active = true;
          lcd.setCursor(10,1);
          lcd.print("CRITICO");
      }
      
         else{
          mq135_active = false;
          lcd.setCursor(10,1);
          lcd.print("NORMAL ");
      }
//Envia para o computador (serial) os dados
    Serial.print("Leitura MQ135: ");
    Serial.println(gas_level_135);
    Serial.println();
  }
}

//Checar sensores 

void check_sensor(bool s1, bool s2){

  if( (s1==true) || (s2==true)){
   
   digitalWrite(cooler,HIGH);
   digitalWrite(buz,LOW);
 
    if (s1==true)
     {
       digitalWrite(relay,HIGH);
       digitalWrite(led,HIGH);
       
       
     }
  
  }
   else {
    digitalWrite(cooler,LOW);
    digitalWrite(buz,HIGH);

  }
}
void loop() {
  tarefa_mq2();
  tarefa_mq135();
  check_sensor(mq2_active, mq135_active);
}
