//Från här -->
#include <Arduino.h>
#include <RotaryEncoder.h>

#define PIN_IN1 20
#define PIN_IN2 21

#define ROTARYSTEPS 2
#define ROTARYMIN 0
#define ROTARYMAX 16

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);
int lastPos = -1;
//<-- till här, delvis kopierad från https://github.com/mathertel/RotaryEncoder/blob/master/examples/LimitedRotator/LimitedRotator.ino

//lcd
#include <DIYables_LCD_I2C.h> // Library for LCD
DIYables_LCD_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows



const int buzzer = 26; //vet inte om det är rätt pin men whatevs
const int setupknapp = 12;
const int startknapp = 19;
const int lockinknapp = 1; //är kanske inte här

int setupknapp_state = 0;
int startknapp_state = 0;
int lockinknapp_state = 0;

unsigned long itryckt_tid = 0;
unsigned long sluttid = 0;
unsigned long starttid = 0;

String mode = "";


void setup() {
  //snurrknapp
  encoder.setPosition(10 / ROTARYSTEPS); // start with the value of 10.

  //lcd
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight
  lcd.setCursor(5, 0);


 //resten av knapparna
  pinMode(buzzer, OUTPUT);
  pinMode(setupknapp, INPUT);
  pinMode(startknapp, INPUT);
  pinMode(lockinknapp, INPUT);

  //startsignal
  lcd.print("Startar...");
  delay(1000);
  spela_buzzer(100, 200);
  spela_buzzer(100, 500);
  lcd.clear();
}


void spela_buzzer(int buzztid, int paustid) {
  digitalWrite(buzzer, HIGH);
  delay(buzztid);
  digitalWrite(buzzer, LOW);
  delay(paustid);
}


int kolla_knappar() {
  setupknapp_state = digitalRead(setupknapp);
  startknapp_state = digitalRead(startknapp);
  lockinknapp_state = digitalRead(lockinknapp);

  if (setupknapp_state == HIGH) {
    return setupknapp;
  }
  else if (startknapp_state == HIGH) {
    return startknapp;
  }
  else if (lockinknapp_state == HIGH) {
    return lockinknapp;
  }

  encoder.tick();

  // get the current physical position and calc the logical position
  int newPos = encoder.getPosition() * ROTARYSTEPS;

  if (newPos < ROTARYMIN) {
    encoder.setPosition(ROTARYMIN / ROTARYSTEPS);
    newPos = ROTARYMIN;

  } else if (newPos > ROTARYMAX) { 
    encoder.setPosition(ROTARYMAX / ROTARYSTEPS);
    newPos = ROTARYMAX;
  } // if

  if (lastPos != newPos) {
    lastPos = newPos;
  } // if

  return 67;
}


unsigned long knapptid(int knapp, unsigned long sys_tid) {
  unsigned long starttid = sys_tid;
  int knapp_state = digitalRead(knapp);
  while (knapp_state == HIGH) {
    sluttid = sys_tid;
  }
  unsigned long totaltid = sluttid - starttid;
  return totaltid;
}


void pris(int antal) {
  //sätt in dispenserfunktion

}


bool kontrollera_mobil() {
  // ersätt den här
  return true;
}


void varning() {
  delay(1000);
  if (kontrollera_mobil() == false) {
    return;
  }

  for (int i=0; i<10; i++) {
    spela_buzzer(200, 200);
  }
  
  if (kontrollera_mobil() == false) {
    return;
  }

  for (int i=0; i<1; i++) {
    spela_buzzer(3000, 0);
  }
  
  if (kontrollera_mobil() == false) {
    return;
  }
}


void pomodorocykel(bool duktig, int priser) { //kan ta in en string som enkapsulerar båda istället
  //timerstart
  unsigned long cykelstart = millis();
  //kontrollera mobil
  while (true) {
    unsigned long cykeltid = millis() - cykelstart;
    
    if (kontrollera_mobil() == false) {
      varning();
    }
    
    if (cykeltid <= 1500000) {
      break;
    }
  }
  pris(priser);
}


void pomodoromaskin(int loopar, String mode) { //skulle eventuellt kunna ha att den tar in en str som typ "mode".
  for (loopar > 0; loopar--;) {
    pomodorocykel(false, 1);
  }

}


void loop() {
  unsigned long sys_tid = millis();
  int igang = 1;
  int steg = 1;
  //huvudsystemet
  
  while (igang == 1) {

    //steg 1: Vilka inställningar?
    while (steg == 1) {
    
      int itryckt = kolla_knappar();
      // om knapp itryckt: hur länge är den itryckt? 
      if (itryckt =! 67) {
        unsigned long itryckt_tid = knapptid(itryckt, sys_tid);
        // ger tid i millisekunder som den är itryckt.
      }

      //behövs kod för rot.encoder här.
      //mode = ""; mode ändras till något här som kommer användas till

      if (itryckt == 12 && itryckt_tid >= 5000) { //setupknapp
        // servo_öppnare() // typ nåt sånt här som låter en ladda upp med dricka.
      }
      
      else if (itryckt == 19 && itryckt_tid >= 50) { //selectknapp
        steg = 2; // den kommer genom kolla_knappar registrera snurrningarna och sedan trycker man select för att gå vidare.
      }

    }

    //steg 2: hur många cykler? Rot. encoder snurrar för att bestämma antal.
    while (steg == 2) {

      int itryckt = kolla_knappar();
      // om knapp itryckt: hur länge är den itryckt? 
      if (itryckt != 67) {
        unsigned long itryckt_tid = knapptid(itryckt, sys_tid);
        // ger tid i millisekunder som den är itryckt.
      }

      //grej för rot. encoder här oxå.
      
      if (itryckt == 12 && itryckt_tid >= 50) { //setupknapp
        steg = 1;
      }
      
      else if (itryckt == 19 && itryckt_tid >= 50) { //selectknapp
        steg = 3;
      }
    
    }

    //steg 3: kör programmet enligt instruktioner från 1-2.
    while (steg == 3) {
      int loopar = 5; // ska kunna ändras i steg 2
      //int antal_pomodorare = 1; //ska kunna ändras i steg 1
      //bool duktig = false; //ska kunna ändras i steg 1
      // kör pomodoro här.
      pomodoromaskin(loopar, mode); // lägg in de andra variablarna allt eftersom.
      steg = 67;
      igang = 0;
    }
  }
 
 
  //Algoritm:
  //1. välj antal cykler
  //1. välj om lockin-knapp
  
  //2. starta system


  //+ om setup hålls in i 8 sek startas display-mode, som bara gör att timern är mycket kortare (30 sek)
 
  //loop här --->
  //timern igång:
  //här buzzer om telefon tas ut (del av telefonkoden)

  //timern avklarad
  //här väljs antingen godisdispenser, drycklock eller båda när tiden är slut.

  //<---- till hit. 

  //coola extrasaker:
  // duktig-program: tar bort krav på mobilhållare:
  //Co-op: flera godisutmatningar
  


  //avslutningsmeddelande efter och sedan startas programmet om.

}
