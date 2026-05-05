
//LCD
#include <DIYables_LCD_I2C.h> // Library for LCD
#include <Stepper.h> // stepper motor
#include <Adafruit_NeoPixel.h> //gissa

//Försök 2 rot encoder
#define CLK A1
#define DT A2


//Neopixel
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define PIN        A3 
#define NUMPIXELS 1 // En pixel

// Servobibblan
#include <Servo.h>

//strömkollare
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

const int outputPin = 2; //Du får bestämma pinsen själv
const int overridePin = 5; 

// Servopin
int servoPin = 10;

// Variabel för att förvara servopositionen
int pos;

//Servoobjekt
Servo Servo1;

//lcd 
DIYables_LCD_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

const int stepsPerRev = 200;
Stepper myStepper(stepsPerRev, 4, 3, 7, 8);

const int ena = 6;
const int enb = 5;

const int buzzer = 12; //vet inte om det är rätt pin men whatevs
const int setupknapp = 9;
const int startknapp = A0;

int setupknapp_state = 0;
int startknapp_state = 0;
int lockinknapp_state = 0;

unsigned long itryckt_tid = 0;
unsigned long sluttid = 0;
unsigned long starttid = 0;

int mode = 0;
int antal_pomodoro = 1;
int steg;
int igang;

int counter = 0;
int lastStateCLK;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);






void setup() {
  Serial.begin(115200);

  //neopixel
  pixels.begin();

  //snurrknapp
  //encoder.setPosition(10 / ROTARYSTEPS); // start with the value of 10.
  //snurrknapp lösning 2 lastStateCLK
  pinMode(CLK, INPUT_PULLUP); // Critical for 3-pin encoders
  pinMode(DT, INPUT_PULLUP);
  lastStateCLK = digitalRead(CLK);

  Servo1.attach(servoPin);

 //resten av knapparna
  pinMode(buzzer, OUTPUT);
  pinMode(setupknapp, INPUT_PULLUP);
  pinMode(startknapp, INPUT_PULLUP);
  
  // stepper motor
  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);
  myStepper.setSpeed(10);
  analogWrite(ena, 5); 
  analogWrite(enb, 5);

  //strömgrej
  ina219.begin();
  
  pinMode(outputPin, OUTPUT);
  pinMode(overridePin, INPUT_PULLUP); 

  //startsignal
  //lcd
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight
  lcd.setCursor(0, 0);
  lcd.print("Perfekt Pomodoro");
  lcd.setCursor(3,1);
  lcd.print("..startar..");
  pixels.setPixelColor(0, pixels.Color(50, 50, 50));
  pixels.show();
  spela_intro();
  lcd.clear();

}


void lcd_pomodoro_setup() {
  lcd.clear();
  
  if (steg == 1) {
    lcd.setCursor(0,0);
    lcd.print("Steg 1: mode");
  }
  else if (steg == 2) {
    lcd.setCursor(0,0);
    lcd.print("Steg 2: antal");
  }

  lcd.setCursor(0,1);
  lcd.print("MODE:"); //kom ihåg att börja Mode-symbol på 5
  lcd.setCursor(8,1);
  lcd.print("RNDS:"); // kom ihåg 15
}

void lcd_tidkvar(int kvar, bool pom) {
  lcd.clear();
  lcd.setCursor(0,0);
  if (pom == true) {
    lcd.print("Aktiv Pomodoro!");
  }
  else {
    lcd.print("Ta en paus :)");
  }
  lcd.setCursor(0,1);
  lcd.print("Sek kvar: ");
  lcd.setCursor(10, 1);
  lcd.print(kvar);

}

void update_lcd() {
    lcd.setCursor(5,1);
    //modes:
    ///0=standard/normal(nor), 1=test (tst), 2=coop (cop) 3= duktig(som normal fast ingen mobil)(duk), 4=display(öppna och snurra utan kortare tid)
    if (mode == 0) {
      lcd.print("nor");}
    if (mode == 1) {
      lcd.print("tst");}
    if (mode == 2) {
      lcd.print("cop");}
    if (mode == 3){
      lcd.print("duk");}
    if (mode == 4){
      lcd.print("dts");}
    if (mode == 5){
      lcd.print("dis");}


    lcd.setCursor(14,1);
    //print här
    lcd.print((antal_pomodoro));
}

bool lcd_update_checker() { 
  int skib;
  int idi;  
    skib = 100;
    idi = 6;

    if ( steg == 3) {
      skib = 500;
    }

  unsigned long sixseven = millis();
  if ((sixseven % skib) == idi) {
    return true;
  }

  return false;
}


void spela_buzzer(int buzztid, int paustid) { // behöver uppdateras pga aktiv buzzer!
  tone(buzzer, 1000);
  delay(buzztid);
  noTone(buzzer);
  delay(paustid);
}

void tone_length(int length, int hz) {
  tone(buzzer, hz);
  delay(length);
  noTone(buzzer);
}

void spela_truddilutt(int buzztid, int crescendo) {
  int inc = crescendo / buzztid;
  for (int x; x < buzztid; x++) {
    int ton = x*inc;
    tone(buzzer, ton);
    delay(1);
    noTone(buzzer);
  }
}

void spela_intro() {
  tone_length(200, 1975);
  tone_length(100, 1396);
  tone_length(100, 1567);
  tone_length(100, 1046);
  tone_length(300, 1318);
}

void lock() {
  for (pos = 90 ;pos >= 0; pos--){
    Servo1.write(pos);
    delay(15);
  }
  bool open = true;
  while (open){
    int itryckt = kolla_knappar();
    // om knapp itryckt: hur länge är den itryckt? 
    if (itryckt != 67) {
      pixels.setPixelColor(0, pixels.Color(150, 0, 0));
      pixels.show();
      itryckt_tid = knapptid(itryckt);
      pixels.setPixelColor(0, pixels.Color(0, 150, 0));
      pixels.show();
      // ger tid i millisekunder som den är itryckt.
    }
    else {itryckt_tid = 0;}

    if (itryckt == startknapp && itryckt_tid >= 10) { //startknapp
      // lock stängs
      for (pos = 0; pos <= 90; pos++) {
        Servo1.write(pos);
        delay(15);
      }
      break;
    }
  }
}


void kontrollera_counter() {
  int max1 = 5;
  int max2 = 9;
  int currentStateCLK = digitalRead(CLK);

  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    if (digitalRead(DT) != currentStateCLK) {
      counter--; // Counter-clockwise
      if (steg == 1) {
        if  (counter < 0) {
          mode = 0;
          counter = 0;
      }
      else {mode = counter;}
      }
      
      else if (steg == 2) {
        if  (counter < 1) {
          antal_pomodoro = 1;
          counter = 1;
        }
        else {antal_pomodoro = counter;}
      }
    } 
    else {
      counter++; // Clockwise
      if (steg == 1) {
         if (counter > max1) {
          mode = max1;
          counter = max1;
        }
        else {mode = counter;}
      }
      else if (steg == 2) {
        if (counter > 9) {
          antal_pomodoro = 9;
          counter = max2;
        }
        else {antal_pomodoro = counter;}
      }
    }
  }
  lastStateCLK = currentStateCLK;
}
int kolla_knappar() {
  setupknapp_state = digitalRead(setupknapp);
  startknapp_state = digitalRead(startknapp);

  if (setupknapp_state == LOW) {
    spela_buzzer(100, 0);
    return setupknapp;
  }
  else if (startknapp_state == LOW) {
    spela_buzzer(100, 0);
    return startknapp;
  }

  kontrollera_counter();
  return 67;
}


unsigned long knapptid(int knapp) {
  unsigned long starttid = millis();
  while (digitalRead(knapp) == LOW) {
  } //pausar här tills den släpps.
  sluttid = millis();

  unsigned long totaltid = sluttid - starttid;
  return totaltid;
}

void dispenser(){
  // 1. Full kraft för att röra på sig (ca 1.5A - 2A beroende på värde)
  int i;
  if (mode == 2) { i = 2;}
  else { i = 1; }
  snurraStepper(i);
  delay(500); // Vänta 5 sekunder låst

}

void snurraStepper(int varv){
  analogWrite(ena, 150); 
  analogWrite(enb, 150);
  myStepper.step(varv*stepsPerRev);

  // 2. Sänk till "Hållström" (Låst men svalare)
  // Testa dig fram, t.ex. 80 ger tillräckligt lås men mycket mindre värme
  analogWrite(ena, 5); 
  analogWrite(enb, 5);
}


void pris(bool sista) {
  //sätt in dispenserfunktion och kyllådefunktion.
  dispenser();
  if (sista == true) {
    lock();
  }
}


bool kontrollera_mobil() {
  float current_mA = ina219.getCurrent_mA();
  bool overrideActive = (digitalRead(overridePin) == LOW);
  bool isRunning = (digitalRead(outputPin) == HIGH);
  
  // Serial Plotter data
  Serial.print(0); Serial.print(" ");
  Serial.print(1000); Serial.print(" ");
  Serial.println(current_mA); 
  
  // Logik för indikering och systemstart
  // Systemet räknas som "aktivt" om strömmen > 200 mA ELLER override är på
  if (current_mA > 200|| overrideActive) { 
    return true;    
  } 
  else if (current_mA >= 150 && isRunning) {
    // Mellanläge så den inte stängs av i förtid
    return true;
  } 
  else {
    return false;
  }


}


bool varning() {
  delay(10);
  if (kontrollera_mobil() == true) {
    return true;
  }
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Stoppa tillbaka");
  lcd.setCursor(0,1);
  lcd.print("mobilen!");
  
  for (int i=0; i<10; i++) {
    spela_buzzer(200, 200);
  }
  
  if (kontrollera_mobil() == true) {
    return true;
  }

  for (int i=0; i<1; i++) {
    spela_buzzer(3000, 0);
  }
  
  if (kontrollera_mobil() == true) {
    return true;
  }
  return false;
}

void pausTimer() {
  unsigned long cykelstart = millis();
  while (true) {

      unsigned long cykeltid = millis() - cykelstart;
      unsigned long tid1 = (cykeltid/1000);
      //den här delen funkar ju inte som den ska .......
      int itryckt = kolla_knappar();
        // om knapp itryckt: hur länge är den itryckt? 
        if (itryckt != 67) {
          pixels.setPixelColor(0, pixels.Color(50, 50, 0));
          pixels.show();
          itryckt_tid = knapptid(itryckt);
          pixels.setPixelColor(0, pixels.Color(0, 50, 50));
          pixels.show();
          if (itryckt == startknapp && itryckt_tid >= 10) { //starta cykel
            return; }
        }
      if (cykeltid >= 225) {
        break;
      }
      int tid_kvar;
      if (lcd_update_checker() == true) {
        tid_kvar = (300)-tid1;
        lcd_tidkvar(tid_kvar, false);
      }
      delay(500);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Tryck på knappen");
      lcd.setCursor(0,1);
      lcd.print("Aktivera cykel");
    }
}

void pomodorocykel(int length) { //kan ta in en string som enkapsulerar båda istället
  //timerstart
  unsigned long cykelstart = millis();
  //kontrollera mobil
  while (true) {

    unsigned long cykeltid = millis() - cykelstart;
    unsigned long tid1 = (cykeltid/1000);
    
    if ((kontrollera_mobil() == false) && (mode != 3) && (mode != 4)) {
      bool tillbaka = false;
      tillbaka = varning();
      if (tillbaka != true) {
        return;
      }
    }

    //den här delen funkar ju inte som den ska .......
    int itryckt = kolla_knappar();
      // om knapp itryckt: hur länge är den itryckt? 
      if (itryckt != 67) {
        pixels.setPixelColor(0, pixels.Color(50, 50, 0));
        pixels.show();
        itryckt_tid = knapptid(itryckt);
        pixels.setPixelColor(0, pixels.Color(0, 50, 50));
        pixels.show();
        if (itryckt == setupknapp && itryckt_tid >= 3000) { //setupknapp
          antal_pomodoro = 0;
          igang = 0;
          steg = 67; 
          return;
      }

      }
    if ((cykeltid >= 10000) && (length == 0) ) {
      break;
    }
    if (cykeltid >= 1500000) {
      break;
    }
    int tid_kvar;
    if (lcd_update_checker() == true) {
      if (length == 0) {
        tid_kvar = (10)-tid1;
      }
      else { tid_kvar = (1500)-tid1; }
      lcd_tidkvar(tid_kvar, true);
    }
  }
  bool sista = false;
  if (antal_pomodoro == 0) {
    sista = true;
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Pumpar ut godis");
  lcd.setCursor(0,1);
  if (sista == true) {
    lcd.print("Och en dricka ;)");
  }
  else {
    lcd.print("                         ");
  }
  pris(sista);
  pausTimer();

}


void pomodoromaskin() { //skulle eventuellt kunna ha att den tar in en str som typ "mode".
  if ((mode == 1) || (mode == 4)) {
    for (; antal_pomodoro >= 1; antal_pomodoro--) {
      pomodorocykel(0);
    }
    return;
  }
  else {
    for (; antal_pomodoro >= 1; antal_pomodoro--) {
      pomodorocykel(25);
    }
    return;
  }
}


void loop() {
  igang = 1;
  steg = 1;
  mode = 0;
  antal_pomodoro = 1;
  //huvudsystemet
  
  while (igang == 1) {
    
    if (steg == 1) {
    counter = mode;
    lastStateCLK = digitalRead(CLK);
    lcd_pomodoro_setup();
    }
    //steg 1: Vilka inställningar?
    while (steg == 1) {

      if (lcd_update_checker()==true) {
      update_lcd();
      }
    
      int itryckt = kolla_knappar();
      // om knapp itryckt: hur länge är den itryckt? 
      if (itryckt != 67) {
        pixels.setPixelColor(0, pixels.Color(100, 10, 50));
        pixels.show();
        itryckt_tid = knapptid(itryckt);
        pixels.setPixelColor(0, pixels.Color(50, 50, 50));
        pixels.show();
        // ger tid i millisekunder som den är itryckt.
      }
      else {itryckt_tid = 0;}

      //behövs kod för rot.encoder här.
      //mode = ""; mode ändras till något här som kommer användas till

      if (itryckt == setupknapp && itryckt_tid >= 3000) { //setupknapp
        lock();
      }
      
      else if (itryckt == startknapp && itryckt_tid >= 10) { //selectknapp
        steg = 2; // den kommer genom kolla_knappar registrera snurrningarna och sedan trycker man select för att gå vidare.
        break;
      }

    }

    if (steg == 2) {
    counter = antal_pomodoro;
    lastStateCLK = digitalRead(CLK);
    lcd_pomodoro_setup();
    }

    //steg 2: hur många cykler? Rot. encoder snurrar för att bestämma antal.
    
    while (steg == 2) {
      
      if (lcd_update_checker()==true) {
        update_lcd();
      }

      int itryckt = kolla_knappar();
      // om knapp itryckt: hur länge är den itryckt? 
      if (itryckt != 67) {
        pixels.setPixelColor(0, pixels.Color(100, 50, 10));
        pixels.show();
        itryckt_tid = knapptid(itryckt);
        pixels.setPixelColor(0, pixels.Color(50, 50, 50));
        pixels.show();
        // ger tid i millisekunder som den är itryckt.
      }
      else {itryckt_tid = 0;}

      //grej för rot. encoder här oxå.
      
      if (itryckt == setupknapp && itryckt_tid >= 10) { //setupknapp
        steg = 1;
        break;
      }
      
      else if (itryckt == startknapp && itryckt_tid >= 10) { //selectknapp
        steg = 3;
        break;
      }
    
    }

    //steg 3: kör programmet enligt instruktioner från 1-2.
    while (steg == 3) {
      if (mode == 5) {
        pris(true);
      }
      // kör pomodoro här.
      else {
        pomodoromaskin(); // lägg in de andra variablarna allt eftersom.
      }
      steg = 67;
      igang = 0;
    }
    
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Programmet");
  lcd.setCursor(0, 1);
  lcd.print("avslutas...");
  delay(3000);

}

 //Algoritm:
  //1. välj antal cykler
  //1. välj om lockin-knapp
  //2. starta system

  //loop här --->
  //timern igång:
  //här buzzer om telefon tas ut (del av telefonkoden)
  //timern avklarad
  //här väljs antingen godisdispenser, drycklock eller båda när tiden är slut.

  //<---- till hit. 

  //coola extrasaker:
  //+ om setup hålls in i 8 sek startas display-mode, som bara gör att timern är mycket kortare (30 sek)
  // duktig-program: tar bort krav på mobilhållare:
  //Co-op: flera godisutmatningar
  


  //avslutningsmeddelande efter och sedan startas programmet om.