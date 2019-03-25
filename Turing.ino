#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //declaração do LCD 16x2

//constantes primeiro valor de 4 bits
  const int bit1;
  const int bit2;
  const int bit3;
  const int bit4;

//contantes segundo valor de 4 bits
  const int _bit1;
  const int _bit2;
  const int _bit3;
  const int _bit4;

//constantes dos 3 botões e do struct da maquina de Turing
  const int boton1=2,boton2=3,boton3=4;
  const int intervalo=500;
  const int estados_maximos=20;
  const int cinta_max=64;
  struct estado{
    char orden_estrella;
    char orden_palo;
    int siguiente_estrella;
    int siguiente_palo;
  };

  struct maquina{
    int estado_actual;
    int escrutado;
    estado estados[estados_maximos];
    int estados_usados;
    boolean cinta[cinta_max];
    int pasos;
  };

//Variaveis
long last_boton1,last_boton2,last_boton3;
//0 para entrar na ordem do *, 1 para transição de * ,2 para ordem de |, 3 para transição de | , 4 para fita , 5 para execução e 6 para parar.
volatile int fase=0;
maquina configuracion;

void setup() {
  Serial.begin(9600);
  attachInterrupt(0, forzar_restart, CHANGE);
  lcd.begin(16, 2);
  lcd.cursor();
  pinMode(ledPin, OUTPUT); 
  pinMode(boton1,INPUT);
  pinMode(boton2,INPUT);
  pinMode(boton3,INPUT);
  reiniciar(true);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void forzar_restart(){
  if(fase==5){
    fase=6;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void reiniciar(boolean total){
  lcd.clear();
  configuracion.pasos=0;
  configuracion.estado_actual =0;
  configuracion.escrutado=(cinta_max/2)+1;
  configuracion.estados_usados=0;
  for (int t=0;t<=cinta_max-1;t++){
    configuracion.cinta[t] = false;
  }
  if(total){
    for(int t=0;t<=estados_maximos-1;t++){
      configuracion.estados[t].orden_estrella = '*';
      configuracion.estados[t].orden_palo= '|';
      configuracion.estados[t].siguiente_estrella = 0;
      configuracion.estados[t].siguiente_palo = 0;
    }
  }
  else{
    fase=4;
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void boton1_pressed(){
  if((millis() - last_boton1)>intervalo){
switch (fase) {
  case 0:
 switch(configuracion.estados[configuracion.estados_usados].orden_estrella){
    case '*':
    configuracion.estados[configuracion.estados_usados].orden_estrella = '|';
    break;
    case '|':
    configuracion.estados[configuracion.estados_usados].orden_estrella = 'r';
    break;
    case 'r':
    configuracion.estados[configuracion.estados_usados].orden_estrella = 'l';
    break;
    case 'l':
    configuracion.estados[configuracion.estados_usados].orden_estrella = 'h';
    break;
    case 'h':
    configuracion.estados[configuracion.estados_usados].orden_estrella = '*';
    break;
    }
break;
case 1:
  configuracion.estados[configuracion.estados_usados].siguiente_estrella ++;
break;
case 2:
    switch(configuracion.estados[configuracion.estados_usados].orden_palo){
    case '*':
    configuracion.estados[configuracion.estados_usados].orden_palo = '|';
    break;
    case '|':
    configuracion.estados[configuracion.estados_usados].orden_palo= 'r';
    break;
    case 'r':
    configuracion.estados[configuracion.estados_usados].orden_palo = 'l';
    break;
    case 'l':
    configuracion.estados[configuracion.estados_usados].orden_palo = 'h';
    break;
    case 'h':
    configuracion.estados[configuracion.estados_usados].orden_palo = '*';
    break;
    }
break;
case 3:
  configuracion.estados[configuracion.estados_usados].siguiente_palo ++;
break;
case 4:
  configuracion.cinta[configuracion.escrutado] = !configuracion.cinta[configuracion.escrutado];
  configuracion.escrutado ++;
break;
case 6:
  fase =5;
  configuracion.pasos=0;
  configuracion.estado_actual =0;
  delay(1000);
  transitar();
break;
}
last_boton1 = millis();
}
}


void boton2_pressed(){
  if((millis() - last_boton2)>intervalo){
    if (fase<3){
      fase++;
    }
    else if (fase==3){
        configuracion.estados_usados ++;
        fase =0;
      }
    else if (fase==4){
          configuracion.escrutado ++;
          }
    else{
      reiniciar(false);
    }
  last_boton2 = millis();
  }
}


void boton3_pressed(){
  if((millis() - last_boton3)>intervalo){
    if (fase<4){
      fase=4;
    }
    else if (fase==4){
      fase =5;
      delay(1000);
      transitar();
    }
    else{
      reiniciar(true);
      lcd.clear();
      fase = 0;
    }
  last_boton3 = millis();
  }
}


void transitar(){
while(fase!=6){
char orden;
int siguiente;
  if(configuracion.cinta[configuracion.escrutado]){
  orden = configuracion.estados[configuracion.estado_actual].orden_palo;
  siguiente = configuracion.estados[configuracion.estado_actual].siguiente_palo;
  }else{
  orden = configuracion.estados[configuracion.estado_actual].orden_estrella;
  siguiente = configuracion.estados[configuracion.estado_actual].siguiente_estrella;
  }
  configuracion.estado_actual = siguiente;
  switch(orden){
  case 'l':
  configuracion.escrutado --;
  break;
  case 'r':
  configuracion.escrutado ++;
  break;
  case '|':
  configuracion.cinta[configuracion.escrutado] = !configuracion.cinta[configuracion.escrutado];
  break;
  case '*':
  configuracion.cinta[configuracion.escrutado] = !configuracion.cinta[configuracion.escrutado];
  break;
   case 'h':
  fase = 6;
  for(int t=0;t<5;t++){
  digitalWrite(13,HIGH);
  delay(200);
  digitalWrite(13,LOW);
  delay(200);
  }
  break;
  
  }
configuracion.pasos ++;  
dibujar(); 
delay(1000); 
}
  
  
}

void dibujar(){
switch (fase) {
  case 0:
  case 1:
  case 2:
  case 3:
 lcd.setCursor(0,0);
 lcd.print('q');
 lcd.print(configuracion.estados_usados);
 lcd.print(" * "); 
 lcd.print(configuracion.estados[configuracion.estados_usados].orden_estrella);
 lcd.print(" "); 
 lcd.print(configuracion.estados[configuracion.estados_usados].siguiente_estrella);
 lcd.setCursor(0,1);
 lcd.print('q');
 lcd.print(configuracion.estados_usados);
 lcd.print(" | "); 
 lcd.print(configuracion.estados[configuracion.estados_usados].orden_palo);
 lcd.print(" "); 
 lcd.print(configuracion.estados[configuracion.estados_usados].siguiente_palo);
 if(fase==0){
 lcd.setCursor(5,0);
 }else if(fase==1){
 lcd.setCursor(7,0);
 }else if(fase==2){
 lcd.setCursor(5,1);
 }else if(fase==3){
 lcd.setCursor(7,1);
 }
 
  break;
  case 4:
  case 5:
  lcd.setCursor(0,0);
  int aux = configuracion.escrutado/16;
  for(int t=0;t<16;t++){
    if(configuracion.cinta[(aux*16)+t]){
  lcd.print('|');}
  else{
  lcd.print('*');}
  }
  lcd.setCursor(0,1);
  lcd.print("Estd:");
  lcd.print(configuracion.estado_actual); 
  lcd.print(" Trans:");
  lcd.print(configuracion.pasos);
  
  lcd.setCursor(configuracion.escrutado%16,0);
   break;
 }
}
 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  Serial.println(fase);
  Serial.println(configuracion.escrutado);
  Serial.println((millis() - last_boton1)/1000);
  
  delay(10);
  dibujar();
  if (digitalRead(boton1)==HIGH){
  boton1_pressed();
  }
  if (digitalRead(boton2)==HIGH){
  boton2_pressed();
  }
  if (digitalRead(boton3)==HIGH){
  boton3_pressed();
  }
}
