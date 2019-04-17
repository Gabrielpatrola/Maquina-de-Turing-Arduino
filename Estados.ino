//= Maquina de Turing ============================================================================

//------------------------------------------------------------------------------------------------
//Saidas de texto
boolean debugging = true;
// Caso falso, apertar o botao de passagem automatica (pin 12)
boolean stepThrough = true;
//Operacoes possiveis
String Werte[3] = {
  "Incremento", "Flip Flop", "Somador"
};
char WERTESIZE = 3;

#define MENU 'M'
#define INCREMENT 'I'
#define ADDER 'A'

//Escolhe a operacao a ser executada
char programm = MENU;



//-Konstanten---------------------------------------------------------------------------------------------
#define LEFT  '<'
#define RIGHT '>'
#define STAY  '|'

#define ONE   '1'
#define ZERO  '0'
#define BLANK '_'

#define READ 'r'
#define WRITE 'W'
#define MOVE 'M'
#define RESEND 'X'
#define RESENDACCEPT 'E'

struct state* STOPP;
//- Sounds ---------------------------------------------------------------------------------------
#define ReadS 800 //440
boolean sounds = true; // false;

unsigned const PIEZO = 13;
void makeSound(int freq) {
  if (sounds) {
    tone(PIEZO, freq);
    delay(45);
    noTone(PIEZO);
  }
}

//- Buttons --------------------------------------------------------------------------------------
unsigned const PIN_LEFT   = 9;
unsigned const PIN_RIGHT  = 10;
unsigned const PIN_CHOOSE = 11;
unsigned const PIN_OK     = 12;

//- LCD-Anzeige ----------------------------------------------------------------------------------
/*
#include <LiquidCrystal.h>
 int rsPin = 2;
 int ePin  = 4;
 int d4Pin = 5;
 int d5Pin = 6;
 int d6Pin = 7;
 int d7Pin = 8;
 LiquidCrystal lcd(rsPin, ePin, d4Pin, d5Pin, d6Pin, d7Pin);
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  //set the LCD address to 0x27 for a 16 chars and 2 line display
struct cell {
  char value; // '0', '1' & '_'(blank)
  struct cell *L;
  struct cell *R;
};


//- Infrarot-Steuerung ---------------------------------------------------------------------------
#include <IRremote.h>
#include <IRremoteInt.h>
IRsend irsend;
int RECV_PIN = A0;
IRrecv irrecv(RECV_PIN);
decode_results results;

//-Estados e Operacoes -----------------------------------------------------------------------------------------------
struct state {
  char write_ZERO;
  char write_ONE;
  char write_BLANK;

  char move_ZERO;
  char move_ONE;
  char move_BLANK;

  struct state *next_ZERO;
  struct state *next_ONE;
  struct state *next_BLANK;

  char name;
};


//-----------Parte que move o conteudo da fita da direita para esquerda--------
struct state *FlipProgramm() {
  struct state *state0 = (state*) malloc(sizeof(state));

  state0->name = '0';

  state0->write_ZERO  = ONE;
  state0->write_ONE   = ZERO;
  state0->write_BLANK = BLANK;

  state0->move_ZERO  = RIGHT;
  state0->move_ONE   = RIGHT;
  state0->move_BLANK = STAY;

  state0->next_ZERO  = state0;
  state0->next_ONE   = state0;
  state0->next_BLANK = STOPP;
  return state0;
}

//-----Incrementador da fita infinita-----------------------------
struct state *IncrementProgramm() {
  Serial.println("IncrementProgramm");
  struct state *state0 = (state*) malloc(sizeof(state));
  struct state *state1 = (state*) malloc(sizeof(state));
  struct state *state2 = (state*) malloc(sizeof(state));
  struct state *state3 = (state*) malloc(sizeof(state));

  state0->name = '0';
  state1->name = '1';
  state2->name = '2';
  state3->name = '3';

  Serial.println(state0->name);
  Serial.println(state1->name);
  Serial.println(state2->name);
  Serial.println(state3->name);

  state0->write_ZERO  = ZERO;
  state0->write_ONE   = ONE;
  state0->write_BLANK = BLANK;

  state0->move_ZERO  = RIGHT;
  state0->move_ONE   = RIGHT;
  state0->move_BLANK = LEFT;

  state0->next_ZERO  = state0;
  state0->next_ONE   = state0;
  state0->next_BLANK = state1;

  state1->write_ZERO  = ONE;
  state1->write_ONE   = ZERO;
  state1->write_BLANK = ONE;

  state1->move_ZERO  = RIGHT;
  state1->move_ONE   = LEFT;
  state1->move_BLANK = STAY;

  state1->next_ZERO  = state3;
  state1->next_ONE   = state1;
  state1->next_BLANK = state2;

  state2->write_ZERO  = ZERO;
  state2->write_ONE   = ONE;
  state2->write_BLANK = BLANK;

  state2->move_ZERO  = LEFT;
  state2->move_ONE   = LEFT;
  state2->move_BLANK = RIGHT;

  state2->next_ZERO  = state2;
  state2->next_ONE   = state2;
  state2->next_BLANK = state3;

  state3->write_ZERO  = ZERO;
  state3->write_ONE   = ONE;
  state3->write_BLANK = BLANK;

  state3->move_ZERO  = RIGHT;
  state3->move_ONE   = RIGHT;
  state3->move_BLANK = LEFT;

  state3->next_ZERO  = state3;
  state3->next_ONE   = state3;
  state3->next_BLANK = state1;
  return state0;
}

//----------Estados de aceitacao e rejeicao-----------------------------------
struct state *Stabilitytest() {
  Serial.println("Stabilitytest");

  struct state *state0 = (state*) malloc(sizeof(state));
  struct state *state1 = (state*) malloc(sizeof(state));
  struct state *state2 = (state*) malloc(sizeof(state));
  struct state *state3 = (state*) malloc(sizeof(state));

  state0->name = '0';
  state1->name = '1';
  state2->name = '2';
  state3->name = '3';

  state0->write_ZERO  = ZERO;
  state0->write_ONE   = ONE;
  state0->write_BLANK = BLANK;

  state0->move_ZERO  = LEFT;
  state0->move_ONE   = LEFT;
  state0->move_BLANK = LEFT;

  state0->next_ZERO  = state1;
  state0->next_ONE   = state1;
  state0->next_BLANK = state1;

  state1->write_ZERO  = ONE;
  state1->write_ONE   = ZERO;
  state1->write_BLANK = ONE;

  state1->move_ZERO  = LEFT;
  state1->move_ONE   = LEFT;
  state1->move_BLANK = LEFT;

  state1->next_ZERO  = state2;
  state1->next_ONE   = state2;
  state1->next_BLANK = state2;

  state2->write_ZERO  = ZERO;
  state2->write_ONE   = ONE;
  state2->write_BLANK = BLANK;

  state2->move_ZERO  = RIGHT;
  state2->move_ONE   = RIGHT;
  state2->move_BLANK = RIGHT;

  state2->next_ZERO  = state3;
  state2->next_ONE   = state3;
  state2->next_BLANK = state3;

  state3->write_ZERO  = ONE;
  state3->write_ONE   = ZERO;
  state3->write_BLANK = ONE;

  state3->move_ZERO  = RIGHT;
  state3->move_ONE   = RIGHT;
  state3->move_BLANK = RIGHT;

  state3->next_ZERO  = state0;
  state3->next_ONE   = state0;
  state3->next_BLANK = state0;
  return state0;
}

//------------Comando para escrever-----------------------------------
void writeCell(char val) {
  send(WRITE);
  send(val);
  //  Serial.print("Send Write ");
  //  Serial.println(val);
}

//------------Comando para o movimento da fita-----------------------------------
void moveTape(char val) {
  send(MOVE);
  send(val);
  Serial.print("Send Move ");
  Serial.println(val);
}

//--------------- Funcao inicio----------------------------------------------------------

void run(struct state *head) {

  Serial.println("Starte Simulation");
  boolean terminated = false;
  struct state* lastHead = head;
  int i = 0;
  char received = NULL;
  while (true) {
    received = NULL;


    Serial.print("Simuliere Schritt ");
    Serial.println(i);

    if (terminated) {
      Serial.print("Terminou no");
      lcd.setCursor(0, 1);
      lcd.print("Estado ");
      Serial.print(head->name);
      lcd.clear();
      lcd.print("Terminou no");
      lcd.setCursor(0, 1);
      lcd.print("Estado ");
      lcd.print(head->name);
        delay(190);
  tone(PIEZO, 440);
  delay(100);
  noTone(PIEZO);
  delay(70);
  tone(PIEZO, 392);
  delay(450);
  noTone(PIEZO);
  delay(190);
      break;
    }

    // ---------Ler a maquina------------
    send(READ);
    received = read();

    Serial.print("Received is:  ");
    Serial.println(received);

    // ---------Possivel erro de comunicacao------------
    if (received != RESEND) {
      printState(head, received);
      lastHead = head;


      //---Leitura e gravacao-----------------------
      waitForNextStep();
      // schreiben
      switch (received) {
        case ONE:
          writeCell(head->write_ONE);
          break;
        case ZERO:
          writeCell(head->write_ZERO);
          break;
        case BLANK:
          writeCell(head->write_BLANK);
          break;
      }

      //---Comando de movimento-----------------------------------------
      waitForNextStep();
      // bewegen
      switch (received) {
        case ONE:

          moveTape(head->move_ONE);
          break;
        case ZERO:

          moveTape(head->move_ZERO);
          break;
        case BLANK:

          moveTape(head->move_BLANK);
          break;
      }

      //---Mudanca de estado----------------------
      waitForNextStep();
      // Zustand wechseln
      Serial.println(received);
      switch (received) {
        case ONE:
          if (head->next_ONE == STOPP) {
            terminated = true;
            break;
          }
          i++;
          head = head->next_ONE;
          break;
        case BLANK:
          if (head->next_BLANK == STOPP) {
            terminated = true;
            break;
          }
          i++;
          head = head->next_BLANK;
          break;
        case ZERO:
          if (head->next_ZERO == STOPP) {
            terminated = true;
            break;
          }
          i++;
          head = head->next_ZERO;
          break;
      }
    }
    else {
      //-----------------------Caso de erro----------------------
      head = lastHead;
    }
  }
}


void setup() {
// lcd.init();                      // initialize the lcd
//  lcd.init();
  Serial.begin(9600);
  irrecv.enableIRIn();
   lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
  //lcd.cursor();
  Serial.println("~TurINO~ States");
  //runCounter();
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_OK, INPUT_PULLUP);
  pinMode(PIN_CHOOSE, INPUT_PULLUP);
  struct state* head;
  if (programm == MENU) {
    Serial.println("A");
    head = startMenu();
  }
  if (programm == INCREMENT) {
    Serial.println("B");
    head = IncrementProgramm();
  }
  if (programm == ADDER) {
    Serial.println("D");
    head = AddierProgramm();
      tone(PIEZO, 440);
  delay(100);
  noTone(PIEZO);
  delay(70);
  tone(PIEZO, 392);
  delay(450);
  noTone(PIEZO);
  delay(190);
  }
  printState(head, NULL);
  run(head);
}

//---------------Envio da operacao---------------------------------
void send(char a) {

  delay(100);
  irsend.sendSony(a, 12);
  Serial.print("Sent ");
  Serial.println(a);
  delay(40);
  // delay(100);
}


//---------------Operacao de leitura---------------------------------
char read() {
  irrecv.enableIRIn();
  char received = NULL;
  long time = millis();
  while (received == NULL) {
    delay(5);
    if (millis() - time > 1000) {
      Serial.println("Timed Out");
      lcd.clear();
      lcd.print("Erro na leitura");

      resetState();
      return RESEND;
    }
    if (irrecv.decode(&results)) {

      received = results.value;
      lcd.setCursor(0, 1);
      lcd.print('?');
      lcd.print(received);
      Serial.print("Received ");
      Serial.println(received);
      irrecv.resume();
      if (received == RESEND) {
        lcd.clear();
        lcd.print("Erro na leitura");
        delay(random(50));
        irsend.sendSony(RESENDACCEPT, 12);
        return (RESEND);
      }
      if (!check (received)) {
        Serial.println("wrong input");
        resetState();
        return RESEND;
      }

    }
    else {
      //   Serial.println("Waiting for read");
    }
  }
  return (received);
}

//---------------Sincronizacao---------------------------------
void resetState() {
  Serial.println("Reseting");
  char received = NULL;
  while (received != RESENDACCEPT) {
    Serial.println("Reseting");
    delay(random(30));
    irsend.sendSony(RESEND, 12);
    delay(20);
    irrecv.enableIRIn();
    for (int i = 0; i < 5; i++) {
      delay(50);
      if (irrecv.decode(&results)) {
        received = results.value;
        Serial.print("Received ");
        Serial.println(received);
        irrecv.resume();
        if (received == RESENDACCEPT) {
          Serial.println("Received RESENDACCEPT while resetting");
          delay(50);
          return;
        }
        if (received == RESEND) {
          Serial.println("Received RESEND while resetting");
          irsend.sendSony(RESENDACCEPT, 12);
          delay(50);
          return;
        }

      }
      received = NULL;
    }

  }
  return;
}




void loop() {

}

//--------------------Testa se o caractere escrito foi aceito------------------------
bool check(char a) {
  switch (a) {
    case ZERO:
      return true;
    case ONE:
      return true;
    case BLANK:
      return true;
    case LEFT:
      return true;
    case STAY:
      return true;
    case RIGHT:
      return true;
    case READ:
      return true;
    case MOVE:
      return true;
    case WRITE:
      return true;
  }
  return false;
}
//------------------------Entrada do usuario no pino 12-----------------
void waitForNextStep() {
  while ((digitalRead(PIN_OK)) && !stepThrough) {
    delay( 1);
  }
  while ((!digitalRead(PIN_OK)) && !stepThrough) {
    delay(1);
  }
}

//------------------------------Menu de selecao------------------------------------
struct state* startMenu() {
  lcd.print("Menu Principal");
  makeSound(440);
  makeSound(440);
  makeSound(600);
  makeSound(600);
  delay(1000);
  lcd.clear();
  int point = 0;
  lcd.print(Werte[0]);
  while (true) {
    if (!digitalRead(PIN_LEFT)) {
      if (point == 0) {
        point = WERTESIZE - 1;
      }
      else {
        point--;
      }
      lcd.clear();
      lcd.print(Werte[point]);
      delay(200);
    }
    if (!digitalRead(PIN_RIGHT)) {
      point = (point + 1) % WERTESIZE;
      lcd.clear();
      lcd.print(Werte[point]);
      delay(200);
    }
    if (!digitalRead(PIN_OK)) {
      break;
    }
  }

  struct state* ret;
  switch (point) {
    case 0:
      ret = IncrementProgramm();
      break;
    case 1:
      ret = FlipProgramm();
      break;
    case 2:
      ret = AddierProgramm();
      break;
  }
  lcd.clear();
  lcd.print("Automatico?");
  lcd.setCursor(0, 1);
  lcd.print(stepThrough);
  delay(500);
  stepThrough = stepThrough;
  while (true) {
    lcd.clear();
    lcd.print("Automatico?");
    lcd.setCursor(0, 1);
    lcd.print(stepThrough);
    while ((digitalRead(PIN_LEFT)) && digitalRead(PIN_RIGHT)) {
      if (!digitalRead(PIN_OK)) {
        break;
      }
    }
    if (!digitalRead(PIN_OK)) {
      break;
    }
    stepThrough = !stepThrough;
    delay(200);

  }


  return ret;

}
//---------------------Representacao do estado----------------------
void printState(struct state* head, char received) {
  Serial.print("Der State ist ");
  String toWrite = "Estado # ";
  if (head->name != NULL) {
    toWrite += head->name;
    toWrite += " lido ";
    Serial.println(head->name);

  }
  else {
    Serial.println("Nullpointer bei printState");
  }
  makeSound(440);
  makeSound(440);
  lcd.clear();
  lcd.print(toWrite);
  lcd.print(received);
  char dir;
  toWrite = "";



  switch (received) {
    case ONE:
      dir += head->move_ONE;
      break;
    case ZERO:
      dir += head->move_ZERO;
      break;
    case BLANK:
      dir += head->move_BLANK;
      break;
  }
  if (dir == '<') {
    toWrite += '<';
  }
  else {
    toWrite += ' ';
  }
  toWrite += ' ';

  switch (received) {
    case ONE:
      toWrite += head->write_ONE;
      break;
    case ZERO:
      toWrite += head->write_ZERO;
      break;
    case BLANK:
      toWrite += head->write_BLANK;
      break;
  }



  toWrite += ' ';
  if (dir == '>') {
    toWrite += '>';
  }
  else {
    toWrite += ' ';
  }
  toWrite += "    ";
  toWrite += "Prx:";
  switch (received) {
    case ONE:
      toWrite += head->next_ONE->name;
      break;
    case BLANK:
      toWrite += head->next_BLANK->name;
      break;
    case ZERO:
      toWrite += head->next_ZERO->name;
      break;
  }
  lcd.setCursor(0, 1);
  lcd.print(toWrite);
}

//-----programa de adicao-----------------------------
struct state *AddierProgramm() {
  Serial.println("IncrementProgramm");
  struct state *state0 = (state*) malloc(sizeof(state));
  struct state *state1 = (state*) malloc(sizeof(state));
  struct state *state2 = (state*) malloc(sizeof(state));
  struct state *state3 = (state*) malloc(sizeof(state));
  struct state *state4 = (state*) malloc(sizeof(state));
  struct state *state5 = (state*) malloc(sizeof(state));
  struct state *state6 = (state*) malloc(sizeof(state));
  struct state *state7 = (state*) malloc(sizeof(state));
  struct state *state8 = (state*) malloc(sizeof(state));

  state0->name = '0';
  state1->name = '1';
  state2->name = '2';
  state3->name = '3';
  state4->name = '4';
  state5->name = '5';
  state6->name = '6';
  state7->name = '7';
  state7->name = '8';

  state0->write_ZERO  = ONE;
  state0->move_ZERO  = LEFT;
  state0->next_ZERO  = state0;

  state0->write_ONE  = ZERO;
  state0->move_ONE  = LEFT;
  state0->next_ONE  = state1;
  //--------------------
  state1->write_ZERO  = ZERO;
  state1->move_ZERO  = LEFT;
  state1->next_ZERO  = state1;

  state1->write_BLANK  = BLANK;
  state1->move_BLANK  = LEFT;
  state1->next_BLANK  = state7;

  state1->write_ONE  = ONE;
  state1->move_ONE  = LEFT;
  state1->next_ONE  = state1;
  //--------------------
  state2->write_ZERO  = ONE;
  state2->move_ZERO  = LEFT;
  state2->next_ZERO  = state3;

  state2->write_BLANK  = ONE;
  state2->move_BLANK  = RIGHT;
  state2->next_BLANK  = state3;

  state2->write_ONE  = ZERO;
  state2->move_ONE  = LEFT;
  state2->next_ONE  = state2;
  //--------------------
  state3->write_ZERO  = ZERO;
  state3->move_ZERO  = RIGHT;
  state3->next_ZERO  = state3;

  state3->write_BLANK  = BLANK;
  state3->move_BLANK  = RIGHT;
  state3->next_BLANK  = state4;

  state3->write_ONE  = ONE;
  state3->move_ONE  = RIGHT;
  state3->next_ONE  = state3;
  //--------------------
  state4->write_ZERO  = BLANK;
  state4->move_ZERO  = RIGHT;
  state4->next_ZERO  = state8;

  state4->write_BLANK  = BLANK;
  state4->move_BLANK  = RIGHT;
  state4->next_BLANK  = state4;

  state4->write_ONE  = ONE;
  state4->move_ONE  = RIGHT;
  state4->next_ONE  = state6;
  //--------------------
  state5->write_ZERO  = ZERO;
  state5->move_ZERO  = STAY;
  state5->next_ZERO  = STOPP;

  state5->write_ONE = ONE;
  state5->move_ONE  = STAY;
  state5->next_ONE  = STOPP;

  state5->write_BLANK = BLANK;
  state5->move_BLANK  = LEFT;
  state5->next_BLANK  = state5;
  //--------------------
  state6->write_ZERO  = ZERO;
  state6->move_ZERO  = RIGHT;
  state6->next_ZERO  = state6;

  state6->write_BLANK  = BLANK;
  state6->move_BLANK  = LEFT;
  state6->next_BLANK  = state0;

  state6->write_ONE  = ONE;
  state6->move_ONE  = RIGHT;
  state6->next_ONE  = state6;
  //--------------------
  state7->write_ZERO  = ONE;
  state7->move_ZERO  = LEFT;
  state7->next_ZERO  = state3;

  state7->write_BLANK  = BLANK;
  state7->move_BLANK  = LEFT;
  state7->next_BLANK  = state7;

  state7->write_ONE  = ZERO;
  state7->move_ONE  = LEFT;
  state7->next_ONE  = state2;
  //--------------------
  state8->write_ZERO  = BLANK;
  state8->move_ZERO  = RIGHT;
  state8->next_ZERO  = state8;

  state8->write_BLANK  = BLANK;
  state8->move_BLANK  = LEFT;
  state8->next_BLANK  = state5;

  state8->write_ONE  = ONE;
  state8->move_ONE  = RIGHT;
  state8->next_ONE  = state6;

  return state0;
}
