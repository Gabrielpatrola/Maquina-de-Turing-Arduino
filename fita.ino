//===========================================================================
  #define LEFT  '<'
  #define RIGHT '>'
  #define STAY  '|'
  
  #define ONE   '1'
  #define ZERO  '0'
  #define BLANK '_'
  
  #define READ  'r'
  #define WRITE 'W'
  #define MOVE  'M'
  
  #define RESEND 'X'
  #define RESENDACCEPT 'E'
  
  //- Sounds ---------------------------------------------------------------------------------------
  #define MoveS 220 //440
  #define WriteS 440
  boolean sounds = true; // false;
  
  unsigned const PIEZO = 13;
  void makeSound(int freq) {
    if (sounds) {
      tone(PIEZO, freq);
      delay(90);
      noTone(PIEZO);
    }
  }
  
  //- Buttons --------------------------------------------------------------------------------------
  unsigned const PIN_LEFT   = 9;
  unsigned const PIN_RIGHT  = 10;
  unsigned const PIN_CHOOSE = 11;
  unsigned const PIN_OK     = 12;
  
  //- LCD-Display ----------------------------------------------------------------------------------
  #include <LiquidCrystal.h>
  int rsPin = 2;
  int ePin  = 4;
  int d4Pin = 5;
  int d5Pin = 6;
  int d6Pin = 7;
  int d7Pin = 8;
  LiquidCrystal lcd(rsPin, ePin, d4Pin, d5Pin, d6Pin, d7Pin);
  
  /*
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x27,16,2);  //set the LCD address to 0x27 for a 16 chars and 2 line display
  struct cell {
    char value; // '0', '1' & '_'(blank)
    struct cell *L;
    struct cell *R;
  };
  */
  
  //- Sensor infravermelho ---------------------------------------------------------------------------
  #include <IRremote.h>
  #include <IRremoteInt.h>
  IRsend irsend;
  int RECV_PIN = A0;
  IRrecv irrecv(RECV_PIN);
  decode_results results;
  
  //- Representacao das operacoes ---------------------------------------------------------------
  /* ESTRUTURA
   *  ESTRUTURA DE DADOS PARA REPRESENTAR AS CELULAS 
   *  ARMAZENAR O VALOR DA CELULA
   *        (ZERO, UM OU BRANCO)
   *  L     representa a esquerda
   *  R     representa a direita
   */
  struct cell {
    char value;
    struct cell *L;
    struct cell *R;
  };
  
  /* estrutura  
   *  estrutura para exibir a fita  
   *  tamanho da fita
   *  ponteiro da cabeca para a celula atual da banda
   */
  struct tape {
    long size;
    struct cell *head;
  };
  
  /* newCell()
   *  cria nova celula
   *  retorna nova celula
   */
  struct cell *newCell() {
    struct cell *empty = (cell*) malloc(sizeof(cell));
    empty->value = BLANK;
    empty->L = NULL;
    empty->R = NULL;
    return empty;
  }
  
  /* newTape ()
    * Cria nova banda com uma célula em branco.
    * RETURN a nova banda
    */
  struct tape *newTape() {
    struct tape *emptytape = (tape*) malloc(sizeof(tape));
    emptytape->size = 1;
    emptytape->head = newCell();
    return emptytape;
  }
  
  /* writeTape (valores)
    * Criado um novo volume descrito com os valores de
    * Valores de seqüências de caracteres.
    * RETURN a nova banda
    */
  struct tape *writeTape(char values[]) {
    struct tape *tape = newTape();
    for (int i = 0; values[i] != '\0'; i++) {
      writeCell(tape->head, values[i]);
      tape = moveRight(tape);
    }
    printTape(tape);
    Serial.print("Band ");
    Serial.print(values);
    Serial.println(" wurde erstellt.");
    return tape;
  }
  
  /* newCell (célula da banda, direção)
    * Adiciona um novo dependendo da direção (ESQUERDA ou DIREITA)
    * Célula com valor BRANCO para a célula de banda transferida.
    * Células vizinhas existentes serão incluídas
    * Sobrescrito!
    * RETURN A cabeça de fita na nova célula
    */
  struct cell *newCell(struct cell *tape, char leftOrRight) {
    struct cell *newcell = (cell*) malloc(sizeof(cell));
    newcell->value = BLANK;
    switch (leftOrRight) {
      case LEFT:
        tape->L = newcell;
        newcell->R = tape;
        newcell->L = NULL;
        Serial.println("New tape cell added Left");
        return newcell;
      case RIGHT:
        tape->R = newcell;
        newcell->L = tape;
        newcell->R = NULL;
        Serial.println("New tape cell added Right");
        return newcell;
    }
  }
  
  /* printTape (banda)
    * Saída de fita para o Monitor Serial e
    * chama printTapeToDisplay.
    */
  void printTape(struct tape* tape) {
    printTapeToDisplay(tape);
    struct cell *head = tape->head;
    while (head->L != NULL) {
      head = head->L;
    }
    struct cell* next = head;
    while (next != NULL) {
      Serial.print(next->value);
      next = next->R;
    }
    Serial.println("");
  }
  
  /* printTapeToDisplay (banda)
    * Grava o conteúdo da fita no visor.
    * A cabeça está centrada no meio
    * para a direita e esquerda é opcionalmente com
    * Brancos preenchidos.
    */
  void printTapeToDisplay(struct tape* tape) {
    struct cell *head = tape->head;
    int i = 8;
    lcd.clear();
    while (head->L != NULL && i > 0) {
      head = head->L;
      i--;
    }
    int j = 0;
    struct cell* next = head;
    for (; i > 0; i--) {
      lcd.print("_");
      j++;
    }
    while (next != NULL && j < 16) {
      lcd.print(next->value);
      next = next->R;
      j++;
    }
    for (; j < 16; j++) {
      lcd.print("_");
    }
  
    lcd.setCursor(0, 1);
    lcd.print("S:");
    lcd.print(tape->size);
    lcd.setCursor(8, 1);
    lcd.print("^");
  }
  
  /* moveRight (banda)
    * Move a cabeça uma célula para a direita.
    * Se esta célula não existir, será
    * criado. Se a célula anterior tiver o valor em branco
    * e nenhum vizinho esquerdo, será deletado.
    * RETURN banda com posição da cabeça alterada
    */
  struct tape *moveRight(struct tape *tape) {
    Serial.println("Move Right");    
    makeSound(MoveS);
    bool del = (tape->head->value == BLANK && tape->head->L == NULL);
    if (tape->head->R == NULL) {
      tape->head = newCell(tape->head, RIGHT);
      tape->size++;
    } else {
      tape->head = tape->head->R;
    }
    if (del) {
      free(tape->head->L);
      tape->head->L = NULL;
      tape->size--;
    }
    return tape;
  }
  
  /* moveLeft (banda)
    * Move a cabeça uma célula para a esquerda.
    * Se esta célula não existir, será
    * criado. Se a célula anterior tiver o valor em branco
    * e nenhum vizinho certo, será deletado.
    * RETURN banda com posição da cabeça alterada
    */
  struct tape *moveLeft(struct tape *tape) {
    Serial.println("Move Left");    
    makeSound(MoveS);
    bool del = (tape->head->value == BLANK && tape->head->R == NULL);
    if (tape->head->L == NULL) {
      tape->head = newCell(tape->head, LEFT);
      tape->size++;
    } else {
      tape->head = tape->head->L;
    }
    if (del) {
    free(tape->head->R);
      tape->head->R = NULL;
      tape->size--;
    }
    return tape;
  }
  
  /* writeCell (banda, símbolo)
    * Escreve o símbolo passado no
    * célula atual do volume
    */
  void writeCell(struct cell *tape, char toWrite) {
    makeSound(WriteS);
    Serial.print("Writing" );
    Serial.println(toWrite);
    if (toWrite == ZERO || toWrite == ONE || toWrite == BLANK) {
      tape->value = toWrite;
    }
  }
  
  /* manualmenteWriteTape ()
    * Permite a gravação manual de uma fita
    * com a ajuda dos botões
    * RETURN nova banda
    */
  struct tape *manuallyWriteTape() {
    struct tape *tape = newTape();
    printTape(tape);
    while (true) {
      if (digitalRead(PIN_OK) == LOW) {
        //lcd.clear();
        lcd.setCursor(10, 1);
        lcd.print("Inicio");
        delay(400);
        while (digitalRead(PIN_OK) == LOW) {};
        //break;
        run(tape);
      }
      if (digitalRead(PIN_LEFT) == LOW) {
        while (digitalRead(PIN_LEFT) == LOW) {};
        //delay(10);
        tape = moveLeft(tape);
        printTape(tape);
      }
      if (digitalRead(PIN_RIGHT) == LOW) {
        while (digitalRead(PIN_RIGHT) == LOW) {};
        //delay(10);
        tape = moveRight(tape);
        printTape(tape);
      }
      if (digitalRead(PIN_CHOOSE) == LOW) {
        while (digitalRead(PIN_CHOOSE) == LOW) {};
        //delay(10);
        switch (tape->head->value) {
          case ONE:
            writeCell(tape->head, BLANK);
            break;
          case BLANK:
            writeCell(tape->head, ZERO);
            break;
          case ZERO:
            writeCell(tape->head, ONE);
            break;
        }
        printTape(tape);
      }
    }
    return tape;
  }
  
  //- RUN-Methode ----------------------------------------------------------------------------------
  void run(struct tape *tape) {
    Serial.println("Starte Simulation");
    long i = 0;
    while (true) {
      if (digitalRead(PIN_OK) == LOW) {
        lcd.setCursor(12, 1);
        lcd.print("Parar");
        delay(400);
        while (digitalRead(PIN_OK) == LOW) {};
        break;
      }
      printTape(tape);
      Serial.print("Simuliere Schritt ");
      Serial.println(i++);
      char received = NULL;
      while (received == NULL) {
        received = read();
      }
      switch (received) {
        case READ:
          {
            Serial.println("Read");
            send(tape->head->value);
            break;
          }
        case WRITE:
          {
            Serial.println("Write");
            char rec = read();
            writeCell(tape->head, rec);
            break;
          }
        case MOVE:
          {
            Serial.println("Move");
            char rec = read();
            switch (rec) {
              case LEFT:
                tape = moveLeft(tape);
                break;
              case STAY:
                tape = tape;
                break;
              case RIGHT:
                tape = moveRight(tape);
                break;
            }
          }
          break;
        default:
          break;
      }
    }
  }
  
  /*void run(struct tape *tape) {
    Serial.println("Starte Simulation");
    char lastValue = tape->head->value;
    struct tape* lastPlace = tape;
    char changedValue = tape->head->value;
    struct tape* changedPlace = tape;
    int i = 0;
    while (true) {
      printTape(tape);
      Serial.print("Simuliere Schritt ");
      Serial.println(i++);
      char received = NULL;
      while (received == NULL) {
        received = read();
      }
      if (received == RESEND) {
        irsend.sendSony(RESENDACCEPT, 12);
        writeCell(tape->head, lastValue);
        tape = lastPlace;
      } else {
        lastValue = changedValue;
        lastPlace = changedPlace;
      }
      switch (received) {
        case READ:
          {
            Serial.println("Read");
            send(tape->head->value);
            break;
          }
          received = NULL;
          while (received == NULL) {
            received = read();
          }
        case WRITE:
          {
            Serial.println("Write");
            lastValue = tape->head->value;
            char rec = read();
            if (rec == RESEND) {
              break;
              received = RESEND;
            }
            writeCell(tape->head, rec);
            changedValue = tape->head->value;
            break;
          }
        case MOVE:
          {
            Serial.println("Move");
            char rec = read();
            switch (rec) {
              case LEFT:
                tape = moveLeft(tape);
                break;
              case STAY:
                tape = tape;
                break;
              case RIGHT:
                tape = moveRight(tape);
                break;
              default:
                writeCell(tape->head, lastValue);
                break;
            }
            changedPlace = tape;
          }
          break;
        default:
          writeCell(tape->head, lastValue);
          break;
      }
    }
  }
  */
  //- IR Senden & Lesen ----------------------------------------------------------------------------
  /* enviar (Caracter)
    * Envia um caracter através do transmissor de infravermelho
    */
  void send(char a) {
    delay(100);
    irsend.sendSony(a, 12);
    Serial.print("Sent ");
    Serial.println(a);
    // Serial.println((int)a);
    delay(40);
    //delay(100);
  }
  
  char read() {
    irrecv.enableIRIn();
    char received = NULL;
    long start = millis();
    while (received == NULL) {
      delay(5);
      if (irrecv.decode(&results)) {
        received = results.value;
        Serial.print("Received ");
        Serial.println(received);
        irrecv.resume();
        if (received == RESEND) {
          irsend.sendSony(RESENDACCEPT, 12);
          delay(50);
          return (RESEND);
        }
        if (!check (received)) {
          Serial.println("wrong input");
          resetState();
          return RESEND;
        }
      } else {
        //   Serial.println("Waiting for read");
      }
    }
    return (received);
  }
  
  void resetState() {
    Serial.println("Reseting");
    char received = NULL;
    while (received != RESENDACCEPT) {
      Serial.println("Reseting");
      delay(30);
      irsend.sendSony(RESEND, 12);
      delay(20);
      irrecv.enableIRIn();
      for (int i = 0; i < 5; i++) {
        delay(random(100));
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
  
  //- SETUP ----------------------------------------------------------------------------------------
  void welcome() {
    lcd.print("Bem vindo");
    delay(190);
    tone(PIEZO, 440);
    delay(100);
    noTone(PIEZO);
    delay(70);
    tone(PIEZO, 392);
    delay(450);
    noTone(PIEZO);
    delay(190);
  }
  
  void setup() {
    Serial.begin(9600);
    irrecv.enableIRIn();
    Serial.println("~TurINO~ Band");
  
    pinMode(PIEZO, OUTPUT);
  
    pinMode(PIN_RIGHT, INPUT_PULLUP);
    pinMode(PIN_LEFT, INPUT_PULLUP);
    pinMode(PIN_OK, INPUT_PULLUP);
    pinMode(PIN_CHOOSE, INPUT_PULLUP);
  
    pinMode(rsPin, OUTPUT);
    pinMode(ePin,  OUTPUT);
    pinMode(d4Pin, OUTPUT);
    pinMode(d5Pin, OUTPUT);
    pinMode(d6Pin, OUTPUT);
    pinMode(d7Pin, OUTPUT);
  
    lcd.begin(16, 2);
    welcome();
  
    struct tape *tape = manuallyWriteTape();
    run(tape);
  }
  void loop() {
  };
  
  //-
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
      default:
        return false;
    }
  }
