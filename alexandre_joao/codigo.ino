#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int ledPin_q=13;
int ledPin_s=12;
int ledPin_b=11;
void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);
  pinMode(ledPin_q, OUTPUT); // iniciando luz do quarto como saida
  pinMode(ledPin_s, OUTPUT); // iniciando luz da sala como saida
  pinMode(ledPin_b, OUTPUT); // iniciando luz do banheiro como saida
  pinMode(A0, INPUT); // iniciando botões do lcd como entrada
  pinMode(A2, INPUT); // iniciando ldr do quarto como entrada
  pinMode(A3, INPUT); // iniciando ldr da sala como entrada
  pinMode(A4, INPUT); // iniciando ldr do banheiro como entrada
  lcd.print("hello, world!");
}

const unsigned long periodo_tarefa_2 = 500; //tempo para acionar a função de Menu
unsigned long tempo_tarefa_2 = millis(); //ultimo momento que a função de Menu foi acionada
bool menu =0;
int opcao = 0; //define em que parte do Menu está
String msg = "Menu"; //mensagem da primeira linha
String msg2 = ""; //mensagem da segunda linha
String hor; //string que armazena a hora atual
int h_temp; //variavel para ajustar o horário
int segundo=0;
int horario = 18*60+30;
double led =1;
// definição da struct dos perfis
struct perfil{
  int luz_quarto; //valor que a luz do quarto vai manter em relação ao ldr
  int luz_sala; //valor que a luz do sala vai manter em relação ao ldr
  int luz_banheiro; //valor que a luz do banheiro vai manter em relação ao ldr
  int horario_noite; //horario que muda para o modo noturno
  bool usar_noite; //define se usará o modo noturno
  bool ligar_auto; //define se vai acionar com presença
};
//variavel para os sensores de presença
bool sensor_quarto = 0;
bool sensor_sala = 0;
bool sensor_banheiro = 0;

perfil dias= {1000,1000,1000,18*60+30,0,1}; //perfil que é utilizado
perfil temp= {1000,1000,1000,18*60+30,0,1}; //perfil temporário para atualizar os perfis
//perfis para armazenar e alterar rapidamente
perfil perfil1= {1000,1000,1000,18*60+30,0,1}; 
perfil perfil2= {1000,1000,1000,18*60+30,0,1};
perfil perfil3= {1000,1000,1000,18*60+30,0,1};

const unsigned long periodo_tarefa_1 = 1000; //tempo para acionar a função de iluminação
unsigned long tempo_tarefa_1 = millis(); //ultimo momento que a função de iluminação foi acionada

/* Tarefa 1: Ajusta a iluminação de acordo com os sensores e perfil */
void tarefa_1(){

  /* Obtém-se o tempo atual */
  unsigned long tempo_atual = millis();
  int ldr_quarto;
  int ldr_sala;
  int ldr_banheiro;
  bool interruptor_quarto;
  bool interruptor_sala;
  bool interruptor_banheiro;

  
  int led_quarto;
  int led_sala;
  int led_banheiro;
  int ajuste;
  int ajt_max=10;
  /* Hora de enviar os dados analógicos caso tenha passado 2000 ms */
  if(tempo_atual - tempo_tarefa_1 > periodo_tarefa_1) {
    tempo_tarefa_1 = tempo_atual;
    //interruptores para caso não queira usar os sensores de presença ou queira ligar as luzes sem alguém no local
    interruptor_quarto = digitalRead(42);
    interruptor_sala = digitalRead(43);
    interruptor_banheiro = digitalRead(44);

    //aciona luz caso estejas com o interruptor ligado ou no mmodo automático e tenha alguém no local, a menos que estaja no horário noturno
    if ((sensor_quarto && dias.ligar_auto && (!dias.usar_noite || (dias.horario_noite>horario)) )|| interruptor_quarto){
      ldr_quarto = analogRead(A2);
      if (ldr_quarto > dias.luz_quarto+50){ //reduz a luz caso o sensor estaja acima do perfil com uma tolerancia de 50(descoberto ser uma tolerancia muito baixa para a precisão do ldr)
        ajuste = (ldr_quarto - (dias.luz_quarto+50))/4;
        if (led_quarto-ajuste > 0 && ajuste <=ajt_max)
          led_quarto-=ajuste;
        else if (led_quarto-ajt_max > 0)
          led_quarto-=ajt_max;
        else
          led_quarto=0;
      }
      else if (ldr_quarto < dias.luz_quarto){ //aumenta a luz caso o sensor estaja abaixo do perfil
        ajuste = (dias.luz_quarto-ldr_quarto)/4;
        if (led_quarto+ajuste < 255 && ajuste <=ajt_max)
          led_quarto+=ajuste;
        else if (led_quarto+ajt_max > 255)
          led_quarto+=ajt_max;
        else
          led_quarto=255;
      }
    }
    else 
      led_quarto = 0;
    if ((sensor_sala && dias.ligar_auto && (!dias.usar_noite || (dias.horario_noite>horario)) )|| interruptor_sala){
      ldr_sala = analogRead(A3);
      if (ldr_sala > dias.luz_sala+50){
        ajuste = (ldr_sala - (dias.luz_sala+50))/4;
        if (led_sala-ajuste > 0 && ajuste <=ajt_max)
          led_sala-=ajuste;
        else if (led_sala-ajt_max > 0)
          led_sala-=ajt_max;
        else
          led_sala=0;
      }
      else if (ldr_sala < dias.luz_sala){
        ajuste = (dias.luz_sala-ldr_sala)/4;
        if (led_sala+ajuste < 255 && ajuste <=ajt_max)
          led_sala+=ajuste;
        else if (led_sala+ajt_max > 255)
          led_sala+=ajt_max;
        else
          led_sala=255;
      }
    }
    else 
      led_sala = 0;
    if ((sensor_banheiro && dias.ligar_auto) || interruptor_banheiro){
      ldr_banheiro = analogRead(A4);
      if (ldr_banheiro > dias.luz_banheiro+50){
        ajuste = (ldr_banheiro - (dias.luz_banheiro+50))/4;
        if (led_banheiro-ajuste > 0 && ajuste <=ajt_max)
          led_banheiro-=ajuste;
        else if (led_banheiro-ajt_max > 0)
          led_banheiro-=ajt_max;
        else
          led_banheiro=0;
      }
      else if (ldr_banheiro < dias.luz_banheiro){
        ajuste = (dias.luz_banheiro-ldr_banheiro)/4;
        if (led_banheiro+ajuste < 255 && ajuste <=ajt_max)
          led_banheiro+=ajuste;
        else if (led_banheiro+ajt_max > 255)
          led_banheiro+=ajt_max;
        else
          led_banheiro=255;
      }
    }
    else 
      led_banheiro = 0;
    //altera o valor pwm da luz
    analogWrite(ledPin_q,led_quarto);
    analogWrite(ledPin_s,led_sala);
    analogWrite(ledPin_b,led_banheiro);
  }
}

/* Tarefa 2: Menu */
void tarefa_2(){

  /* Obtém-se o tempo atual */
  unsigned long tempo_atual = millis();
  int botao = 0; 
  hor = String(int(horario/60))+":"+String(horario%60);
  /* Hora de enviar os dados analógicos caso tenha passado 2000 ms */
  if(tempo_atual - tempo_tarefa_2 > periodo_tarefa_2) {
    tempo_tarefa_2 = tempo_atual;
    botao = analogRead(A0);
    //limpa e escreve a mensagem atual
    lcd.clear();
    lcd.setCursor(8-int(msg.length()/2), 0);
    lcd.print(msg);
    lcd.setCursor(8-int(msg2.length()/2), 1);
    lcd.print(msg2);
    if (botao>=0&&botao<50){ //direita
      switch (opcao) {
          //primeiro bloco seleciona o que quer fazer
          case 0: //alterar horarios
            opcao = 1;
            msg2 = "Ajt. Com. Hor.";
            break;
          case 1:
            opcao = 6;
            msg2 = "Sel. Ajt. Com.";
            break;
          case 6:
            opcao = 11;
            msg2 = "Hor. Sel. Ajt.";
            break;
          case 11:
            opcao = 0;
            msg2 = "Com. Hor. Sel.";
            break;
          //quando está configurando o horário noturno seleciona se altera a hora ou minuto
          case 2:
            opcao =3;
            msg = "Horario|";
            break;
          case 3:
            opcao =2;
            msg = "|Horario";
            break;
          //define se quer salvar ou não as alterações em algum perfil
          case 4:
            opcao =5;
            msg2 = "sim NAO";
            break;
          case 5:
            opcao =4;
            msg2 = "SIM nao";
            break;
          //quando está configurando o horário atual seleciona se altera a hora ou minuto
          case 7:
            opcao =8;
            msg = "Ajustar|";
            break;
          case 8:
            opcao =7;
            msg = "|Ajustar";
            break;
          //define se vai salvar ou não a alteração do relógio
          case 9:
            opcao =10;
            msg2 = "sim NAO";
            break;
          case 10:
            opcao =9;
            msg2 = "SIM nao";
            break;
          //seleciona  o perfil que será utilizado
          case 12:
            opcao = 13;
            msg2 = "Per2 Per3 Per1";
            break;
          case 13:
            opcao = 14;
            msg2 = "Per1 Per2 Per3";
            break;
          case 14:
            opcao = 12;
            msg2 = "Per3 Per1 Per2";
            break;
          //define em qual perfil salva
          case 15:
            opcao = 16;
            msg2 = "Per2 Per3 Per1";
            break;
          case 16:
            opcao = 17;
            msg2 = "Per1 Per2 Per3";
            break;
          case 17:
            opcao = 15;
            msg2 = "Per3 Per1 Per2";
            break;
          //define qual valor do perfil vai alterar
          case 18:
            opcao = 19;
            msg2 = "Sala Ban. Qua.";
            break;
          case 19:
            opcao = 20;
            msg2 = "Auto Sala Ban.";
            break;
          case 20:
            opcao = 24;
            msg2 = "Noit Auto Sala";
            break;
          case 24:
            opcao = 25;
            msg2 = "Quar. Noit Auto";
            break;
          case 25:
            opcao = 18;
            msg2 = "Ban. Qua. Noit";
            break;
          //define se vai deixar a luz no automático
          case 26:
            opcao =27;
            msg2 = "sim NAO";
            break;
          case 27:
            opcao =26;
            msg2 = "SIM nao";
            break;
          //define se vai ter modo noturno
          case 28:
            opcao =29;
            msg2 = "sim NAO";
            break;
          case 29:
            opcao =28;
            msg2 = "SIM nao";
            break;
          default:
            // comando(s)
            break;
      }
    }
    else if(botao>=50&&botao<180){ //cima
      switch (opcao) {
          //altera o valor do hórario noturno
          case 2:
            temp.horario_noite+=60;
            if (int(temp.horario_noite/60)>= 24)
            {
              temp.horario_noite-=24*60;
            }
            msg2 = String(int(temp.horario_noite/60))+":"+String(temp.horario_noite%60);
            break;
          case 3:
            temp.horario_noite++;
            if (int(temp.horario_noite/60)>= 24)
            {
              temp.horario_noite-=24*60;
            }
            msg2 = String(int(temp.horario_noite/60))+":"+String(temp.horario_noite%60);
            break;
          //altera o valor do relógio
          case 7:
            h_temp+=60;
            if (int(h_temp/60)>= 24)
            {
              h_temp-=24*60;
            }
            msg2 = String(int(h_temp/60))+":"+String(h_temp%60);
            break;
          case 8:
            h_temp++;
            if (int(h_temp/60)>= 24)
            {
              h_temp-=24*60;
            }
            msg2 = String(int(h_temp/60))+":"+String(h_temp%60);
            break;
          //define o valor de luz do quarto
          case 21:
            temp.luz_quarto++;
            if (temp.luz_quarto>= 1023)
            {
              temp.luz_quarto=0;
            }
            msg2 = String(temp.luz_quarto);
            break;
          //define o valor de luz da sala
          case 22:
            temp.luz_sala++;
            if (temp.luz_sala>= 1023)
            {
              temp.luz_sala=0;
            }
            msg2 = String(temp.luz_sala);
            break;
          //define o valor de luz do banheiro
          case 23:
            temp.luz_banheiro++;
            if (temp.luz_banheiro>= 1023)
            {
              temp.luz_banheiro=0;
            }
            msg2 = String(temp.luz_banheiro);
            break;
          default:
            // comando(s)
            break;
      }
    }
    else if(botao>=180&&botao<360){ //baixo
      switch (opcao) {
          //altera o valor do hórario noturno
          case 2:
            if (temp.horario_noite/60< 1)
            {
              temp.horario_noite+=24*60;
            }
            temp.horario_noite-=60;
            msg2 = String(int(temp.horario_noite/60))+":"+String(temp.horario_noite%60);
            break;
          case 3:
            if (temp.horario_noite == 0)
            {
              temp.horario_noite+=24*60;
            }
            temp.horario_noite-=1;
            msg2 = String(int(temp.horario_noite/60))+":"+String(temp.horario_noite%60);
            break;
          //altera o valor do relógio
          case 7:
            if (h_temp/60< 1)
            {
              h_temp+=24*60;
            }
            h_temp-=60;
            msg2 = String(int(h_temp/60))+":"+String(h_temp%60);
            break;
          case 8:
            if (h_temp == 0)
            {
              h_temp+=24*60;
            }
            h_temp-=1;
            msg2 = String(int(h_temp/60))+":"+String(h_temp%60);
            break;
          //define o valor de luz do quarto
          case 21:
            if (temp.luz_quarto<= 0)
            {
              temp.luz_quarto=1024;
            }
            temp.luz_quarto--;
            msg2 = String(temp.luz_quarto);
            break;
          //define o valor de luz da sala
          case 22:
            if (temp.luz_sala<= 0)
            {
              temp.luz_sala=1024;
            }
            temp.luz_sala--;
            msg2 = String(temp.luz_sala);
            break;
          //define o valor de luz do banheiro
          case 23:
            if (temp.luz_banheiro<= 0)
            {
              temp.luz_banheiro=1024;
            }
            temp.luz_banheiro--;
            msg2 = String(temp.luz_banheiro);
            break;
          default:
            // comando(s)
            break;
      }
    }
    else if(botao>=360&&botao<500){ //esquerda
      switch (opcao) {
          case 0: //alterar horarios
            opcao = 11;
            msg2 = "Hor. Sel. Ajt.";
            break;
          case 1:
            opcao = 0;
            msg2 = "Com. Hor. Sel.";
            break;
          case 2:
            opcao =3;
            msg = "Horario|";
            break;
          case 3:
            opcao =2;
            msg = "|Horario";
            break;
          case 4:
            opcao =5;
            msg2 = "sim NAO";
            break;
          case 5:
            opcao =4;
            msg2 = "SIM nao";
            break;
          case 9:
            opcao =10;
            msg2 = "sim NAO";
            break;
          case 10:
            opcao =9;
            msg2 = "SIM nao";
            break;
          case 6:
            opcao = 1;
            msg2 = "Ajt. Com. Hor.";
            break;
          case 11:
            opcao = 6;
            msg2 = "Sel. Ajt. Com.";
            break;
          case 7:
            opcao =8;
            msg = "Ajustar|";
            break;
          case 8:
            opcao =7;
            msg = "|Ajustar";
            break;
          case 12:
            opcao = 14;
            msg2 = "Per1 Per2 Per3";
            break;
          case 13:
            opcao = 12;
            msg2 = "Per3 Per1 Per2";
            break;
          case 14:
            opcao = 13;
            msg2 = "Per2 Per3 Per1";
            break;
          case 15:
            opcao = 17;
            msg2 = "Per1 Per2 Per3";
            break;
          case 16:
            opcao = 15;
            msg2 = "Per3 Per1 Per2";
            break;
          case 17:
            opcao = 16;
            msg2 = "Per2 Per3 Per1";
            break;
          case 18:
            opcao = 25;
            msg2 = "Quar. Noit Auto";
            break;
          case 19:
            opcao = 18;
            msg2 = "Ban. Qua. Noit";
            break;
          case 20:
            opcao = 19;
            msg2 = "Sala Ban. Qua.";
            break;
          case 24:
            opcao = 20;
            msg2 = "Auto Sala Ban.";
            break;
          case 25:
            opcao = 24;
            msg2 = "Noit Auto Sala";
            break;
          case 26:
            opcao =27;
            msg2 = "sim NAO";
            break;
          case 27:
            opcao =26;
            msg2 = "SIM nao";
            break;
          case 28:
            opcao =29;
            msg2 = "sim NAO";
            break;
          case 29:
            opcao =28;
            msg2 = "SIM nao";
            break;
          default:
            // comando(s)
            break;
      }
    }
    else if(botao>=500&&botao<750){ //select
      if (menu){
        switch (opcao) {
          case 0: //alterar horarios
            // comando(s)
            msg = "|Horario";
            temp = dias;
            msg2 = String(int(dias.horario_noite/60))+":"+String(dias.horario_noite%60);
            opcao = 2;
            break;
          case 1: //seleciona qual valor do perfil vai mudar
            msg = "Alterar ref";
            opcao = 18;
            msg2 = "Ban. Qua. Sala";
            break;
          case 2: //finaliza alteração do horário noturno e pergunta se quer salvar
            msg = "Salvar?";
            opcao = 4;
            msg2 = "SIM nao";
            break;
          case 3:
            msg = "Salvar?";
            opcao = 4;
            msg2 = "SIM nao";
            break;
          case 4: //altualiza o perfil principal e pergunta em qual perfil quer salvar as alterações
            dias = temp;
            opcao = 15;
            msg = "Salva Perfil";
            msg2 = "Per3 Per1 Per2";
            break;
          case 5: //caso não queira salvar volta para o inicio do menu
            temp.horario_noite = dias.horario_noite;
            opcao = 0;
            msg = "Opcoes";
            msg2 = "Com. Hor. Ajt.";
            break;
          case 6: //define que vai ajustar o relógio
            opcao = 7;
            msg = "|Ajustar";
            h_temp = horario;
            msg2 = hor;
            break;
          case 7: //pergunta se vai salvar o relógio
            msg = "Salvar?";
            opcao = 9;
            msg2 = "SIM nao";
            break;
          case 8:
            msg = "Salvar?";
            opcao = 9;
            msg2 = "SIM nao";
            break;
          case 9: //salva alteração do relógio e volta pro inicio
            horario = h_temp;
            opcao = 0;
            msg = "Opcoes";
            msg2 = "Com. Hor. Sel.";
            break;
          case 10: //não salva alteração do relógio e volta pro inicio
            h_temp = horario;
            opcao = 0;
            msg = "Opcoes";
            msg2 = "Com. Hor. Sel.";
            break;
          case 11: //define que vai usar um perfil
            opcao = 12;
            msg = "Selec Perfil";
            msg2 = "Per3 Per1 Per2";
            break;
          //coloca o perfil selecionado como principal e volta o menu pro inicio
          case 12:
            dias = perfil1;
            opcao = 0;
            msg = "Opcoes";
            msg2 = "Com. Hor. Sel.";
            break;
          case 13:
            dias = perfil3;
            opcao = 0;
            msg = "Opcoes";
            msg2 = "Com. Hor. Sel.";
            break;
          case 14:
            dias = perfil2;
            opcao = 0;
            msg = "Opcoes";
            msg2 = "Com. Hor. Sel.";
            break;
          //salva alteração no perfil selecionado
          case 15:
            perfil1 = dias;
            opcao = 0;
            msg = "Opcoes";
            msg2 = "Com. Hor. Sel.";
            break;
          case 16:
            perfil3 = dias;
            opcao = 0;
            msg = "Opcoes";
            msg2 = "Com. Hor. Sel.";
            break;
          case 17:
            perfil2 = dias;
            opcao = 0;
            msg = "Opcoes";
            msg2 = "Com. Hor. Sel.";
            break;
          //define que vai mudar o valor de luz do cômodo selecionado
          case 18:
            temp = dias;
            opcao = 21;
            msg = "Quarto";
            msg2 = String(temp.luz_quarto);
            break;
          case 19:
            temp = dias;
            opcao = 22;
            msg = "Banheiro";
            msg2 = String(temp.luz_banheiro);
            break;
          case 20:
            temp = dias;
            opcao = 23;
            msg = "Sala";
            msg2 = String(temp.luz_sala);
            break;
          //pergunta se salva alteração
          case 21:
            msg = "Salvar?";
            opcao = 4;
            msg2 = "SIM nao";
            break;
          case 22:
            msg = "Salvar?";
            opcao = 4;
            msg2 = "SIM nao";
            break;
          case 23:
            msg = "Salvar?";
            opcao = 4;
            msg2 = "SIM nao";
            break;
          case 26:
            temp.ligar_auto=1;
            msg = "Salvar?";
            opcao = 4;
            msg2 = "SIM nao";
            break;
          case 27:
            temp.ligar_auto=0;
            msg = "Salvar?";
            opcao = 4;
            msg2 = "SIM nao";
            break;
          case 28:
            temp.usar_noite=1;
            msg = "Salvar?";
            opcao = 4;
            msg2 = "SIM nao";
            break;
          case 29:
            temp.usar_noite=0;
            msg = "Salvar?";
            opcao = 4;
            msg2 = "SIM nao";
            break;
          //define se vai usar a luz automática ou não
          case 24:
            msg = "Luz Auto?";
            temp = dias;
            opcao = 26;
            msg2 = "SIM nao";
            break;
          //define se vai usar o modo noturno ou não
          case 25:
            msg = "Noite Desl.?";
            temp = dias;
            opcao = 28;
            msg2 = "SIM nao";
            break;
          default:
            // comando(s)
            break;
        }
      }
      else{
        menu = 1;
        opcao = 0;
        msg = "Opcoes";
        
        msg2 = "Com. Hor. Sel.";
      }
    }
    //atualiza a mensagem na tela
    lcd.clear();
    lcd.setCursor(5-int(msg.length()/2), 0);
    lcd.print(msg);
    lcd.setCursor(8-int(msg2.length()/2), 1);
    lcd.print(msg2);
    lcd.setCursor(16-int(hor.length()), 0);
    lcd.print(hor);
  }
}
const unsigned long periodo_tarefa_3 = 2000;
unsigned long tempo_tarefa_3 = millis();

/* Tarefa 3: pega os valores dos sensores de presença */
void tarefa_3(){

  /* Obtém-se o tempo atual */
  unsigned long tempo_atual = millis();

  /* Hora de enviar os dados analógicos caso tenha passado 2000 ms */
  if(tempo_atual - tempo_tarefa_3 > periodo_tarefa_3) {
    tempo_tarefa_3 = tempo_atual;
    sensor_quarto = digitalRead(52);
    sensor_sala = digitalRead(51);
    sensor_banheiro = digitalRead(50);
  }
}
unsigned long UtlTime=millis();
void loop() {
  tarefa_1();
  tarefa_2();
  tarefa_3();
  //relógio
  if(millis()-UtlTime<0)   
  {     
    UtlTime=millis();   
  }   
  else   
  {     
    segundo=int((millis()-UtlTime)/1000);  
  }   
  if(segundo>59)   
  {     
    segundo=0;     
    horario++;     
    UtlTime=millis();     
    if(horario>=24*60)     
      horario=0;
  }
}
