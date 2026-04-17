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
  lcd.print("Startar!");
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
  unsigned long sluttid = 0;
  unsigned long starttid = sys_tid;
  int knapp_state = digitalRead(knapp)
  while (knapp_state == HIGH) {
    sluttid = sys_tid;
  }
  unsigned long totaltid = sluttid - starttid;
  return totaltid;
}

void pomodorocykel() {


}


int steg = 1;
void loop() {
  unsigned long sys_tid = millis();
  
  //huvudsystemet
  //1. välj antal cykler
  //1. välj om lockin-knapp
  while (steg == 1) {
    int itryckt kolla_knappar();
    // om knapp itryckt: hur länge är den itryckt? 
    
    if (itryckt =! 67) {
      unsigned long itryckt_tid = knapptid(itryckt)
      // ger tid i millisekunder som den är itryckt.
    }
    // annars om roterare snurrad: byter värde automatiskt.

  }
  //2. starta system


  //+ om setup hålls in i 8 sek startas display-mode, som bara gör att timern är mycket kortare (30 sek)
 
  //loop här --->
  //timern igång:
  //här buzzer om telefon tas ut (del av telefonkoden)

  //timern avklarad
  //här väljs antingen godisdispenser, drycklock eller båda när tiden är slut.

  //<---- till hit. 


  //avslutningsmeddelande efter och sedan startas programmet om.

}
