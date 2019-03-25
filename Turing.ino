#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //declaração do LCD 16x2

//constantes primeiro valor de 4 bits
  int bit1;
  int bit2;
  int bit3;
  int bit4;

//contantes segundo valor de 4 bits
  int _bit1;
  int _bit2;
  int _bit3;
  int _bit4;

//constantes dos 3 botões e do struct da maquina de Turing
  const int ledPin = 13;
  const int botao1=2,botao2=3,botao3=4;
  const int intervalo=500;
  const int estados_maximos=20;
  const int cinta_max=64;
  struct estado{
    char ordem_estrela;
    char ordem_barra;
    int seguinte_estrela;
    int seguinte_barra;
  };

  struct maquina{
    int estado_atual;
    int examinado;
    estado estados[estados_maximos];
    int estados_usados;
    boolean cinta[cinta_max];
    int passos;
  };

//Variaveis
long last_botao1,last_botao2,last_botao3;
//0 para entrar na ordem do *, 1 para transição de * ,2 para ordem de |, 3 para transição de | , 4 para fita , 5 para execução e 6 para parar.
volatile int fase=0;
maquina configuracao;

void setup() {
  Serial.begin(9600);
  attachInterrupt(0, forcar_reinicio, CHANGE);
  lcd.begin(16, 2);
  lcd.cursor();
  pinMode(ledPin, OUTPUT); 
  pinMode(botao1,INPUT);
  pinMode(botao2,INPUT);
  pinMode(botao3,INPUT);
  reiniciar(true);
}


void forcar_reinicio(){
  if(fase == 5){
    fase = 6;
  }
}


void reiniciar(boolean total){
  lcd.clear();
  configuracao.passos = 0;
  configuracao.estado_atual = 0;
  configuracao.examinado = (cinta_max/2)+1;
  configuracao.estados_usados = 0;
  for (int t=0;t<=cinta_max-1;t++){
    configuracao.cinta[t] = false;
  }
  if(total){
    for(int t=0;t<=estados_maximos-1;t++){
      configuracao.estados[t].ordem_estrela = '*';
      configuracao.estados[t].ordem_barra= '|';
      configuracao.estados[t].seguinte_estrela = 0;
      configuracao.estados[t].seguinte_barra = 0;
    }
  }
  else{
    fase = 4;
  }
}

void botao1_pressed(){
  if((millis() - last_botao1)>intervalo){
    switch (fase) {
      case 0:
      switch(configuracao.estados[configuracao.estados_usados].ordem_estrela){
        case '*':
          configuracao.estados[configuracao.estados_usados].ordem_estrela = '|';
        break;
        case '|':
          configuracao.estados[configuracao.estados_usados].ordem_estrela = 'r';
        break;
        case 'r':
          configuracao.estados[configuracao.estados_usados].ordem_estrela = 'l';
        break;
        case 'l':
          configuracao.estados[configuracao.estados_usados].ordem_estrela = 'h';
        break;
        case 'h':
          configuracao.estados[configuracao.estados_usados].ordem_estrela = '*';
        break;
      }
    break;
    case 1:
      configuracao.estados[configuracao.estados_usados].seguinte_estrela ++;
    break;
    case 2:
      switch(configuracao.estados[configuracao.estados_usados].ordem_barra){
        case '*':
          configuracao.estados[configuracao.estados_usados].ordem_barra = '|';
        break;
        case '|':
          configuracao.estados[configuracao.estados_usados].ordem_barra= 'r';
        break;
        case 'r':
          configuracao.estados[configuracao.estados_usados].ordem_barra = 'l';
        break;
        case 'l':
          configuracao.estados[configuracao.estados_usados].ordem_barra = 'h';
        break;
        case 'h':
          configuracao.estados[configuracao.estados_usados].ordem_barra = '*';
        break;
      }
    break;
    case 3:
      configuracao.estados[configuracao.estados_usados].seguinte_barra ++;
    break;
    case 4:
      configuracao.cinta[configuracao.examinado] = !configuracao.cinta[configuracao.examinado];
      configuracao.examinado ++;
    break;
    case 6:
      fase = 5;
      configuracao.passos = 0;
      configuracao.estado_atual = 0;
      delay(1000);
      transitar();
    break;
    }
  last_botao1 = millis();
  }
}

void botao2_pressed(){
  if((millis() - last_botao2)>intervalo){
    if (fase<3){
      fase++;
    }
    else if (fase == 3){
        configuracao.estados_usados ++;
        fase = 0;
      }
    else if (fase == 4){
          configuracao.examinado ++;
          }
    else{
      reiniciar(false);
    }
  last_botao2 = millis();
  }
}


void botao3_pressed(){
  if((millis() - last_botao3)>intervalo){
    if (fase < 4){
      fase = 4;
    }
    else if (fase == 4){
      fase = 5;
      delay(1000);
      transitar();
    }
    else{
      reiniciar(true);
      lcd.clear();
      fase = 0;
    }
  last_botao3 = millis();
  }
}


void transitar(){
  while(fase != 6){
    char orden;
    int seguinte;
      if(configuracao.cinta[configuracao.examinado]){
        orden = configuracao.estados[configuracao.estado_atual].ordem_barra;
        seguinte = configuracao.estados[configuracao.estado_atual].seguinte_barra;
      }
      else{
       orden = configuracao.estados[configuracao.estado_atual].ordem_estrela;
      seguinte = configuracao.estados[configuracao.estado_atual].seguinte_estrela;
      }
    configuracao.estado_atual = seguinte;
    switch(orden){
      case 'l':
        configuracao.examinado --;
      break;
      case 'r':
        configuracao.examinado ++;
      break;
      case '|':
        configuracao.cinta[configuracao.examinado] = !configuracao.cinta[configuracao.examinado];
      break;
      case '*':
        configuracao.cinta[configuracao.examinado] = !configuracao.cinta[configuracao.examinado];
      break;
      case 'h':
        fase = 6;
        for(int t = 0;t < 5;t++){
          digitalWrite(13,HIGH);
          delay(200);
          digitalWrite(13,LOW);
          delay(200);
        }
      break;
    }
    configuracao.passos ++;  
    desenhar(); 
    delay(1000); 
  } 
}

void desenhar(){
  switch (fase) {
    case 0:
    case 1:
    case 2:
    case 3:
      lcd.setCursor(0,0);
      lcd.print('q');
      lcd.print(configuracao.estados_usados);
      lcd.print(" * "); 
      lcd.print(configuracao.estados[configuracao.estados_usados].ordem_estrela);
      lcd.print(" "); 
      lcd.print(configuracao.estados[configuracao.estados_usados].seguinte_estrela);
      lcd.setCursor(0,1);
      lcd.print('q');
      lcd.print(configuracao.estados_usados);
      lcd.print(" | "); 
      lcd.print(configuracao.estados[configuracao.estados_usados].ordem_barra);
      lcd.print(" "); 
      lcd.print(configuracao.estados[configuracao.estados_usados].seguinte_barra);
      if(fase==0){
        lcd.setCursor(5,0);
      } 
      else if(fase == 1){
        lcd.setCursor(7,0);
      }
      else if(fase == 2){
        lcd.setCursor(5,1);
      }
      else if(fase == 3){
        lcd.setCursor(7,1);
      }
    break;
    case 4:
    case 5:
      lcd.setCursor(0,0);
      int aux = configuracao.examinado/16;
      for(int t = 0 ;t < 16;t++){
        if(configuracao.cinta[(aux*16)+t]){
          lcd.print('|');
        }
        else{
          lcd.print('*');
        }
      }
      lcd.setCursor(0,1);
      lcd.print("Estd:");
      lcd.print(configuracao.estado_atual); 
      lcd.print(" Trans:");
      lcd.print(configuracao.passos);
      lcd.setCursor(configuracao.examinado%16,0);
    break;
 }
}

void loop() {
  //Serial.println(fase);
  //Serial.println(configuracao.examinado);
  //Serial.println((millis() - last_botao1)/1000);
  
  delay(10);
  desenhar();
  if (digitalRead(botao1)==HIGH){
  botao1_pressed();
  }
  if (digitalRead(botao2)==HIGH){
  botao2_pressed();
  }
  if (digitalRead(botao3)==HIGH){
  botao3_pressed();
  }
}
