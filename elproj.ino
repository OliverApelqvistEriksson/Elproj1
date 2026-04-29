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

int mode = 0;
int antal_pomodoro = 1;

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        22 // On Trinket or Gemma, suggest changing this to 1
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1 // Popular NeoPixel ring size
// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);






void setup() {
  //neopixel
  pixels.begin();

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

void lcd_pomodoro_setup() {
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("MODE "); //kom ihåg att börja Mode-symbol på 5
  lcd.setCursor(8,1);
  lcd.print("ROUNDS: "); // kom ihåg 15
}

void update_lcd(int sel) {
  if (sel == 1) { //mode
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
      lcd.print("dis");}
  }

  if (sel == 2) { //cykler

    lcd.setCursor(15,1);
    //print här
  }
}

bool lcd_update_checker() {
  unsigned long sex = millis();
  if ((sex % 10) == 7) {
    return true;
  }
  return false;
}


void spela_buzzer(int buzztid, int paustid) { // behöver uppdateras pga aktiv buzzer!
  digitalWrite(buzzer, HIGH);
  delay(buzztid);
  digitalWrite(buzzer, LOW);
  delay(paustid);
}


int kolla_knappar(int steg) {
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
    if (steg==1) {
      if (lastPos > 4) {
        mode = 4;
      }
      else {mode = lastPos;}
    }
    else if (steg == 2) {
      antal_pomodoro = lastPos;
    }
  } // if

  return 67;
}


unsigned long knapptid(int knapp) {
  unsigned long starttid = millis();
  while (digitalRead(knapp) == HIGH) {
  } //pausar här tills den släpps.
  sluttid = millis();

  unsigned long totaltid = sluttid - starttid;
  return totaltid;
}


void pris(bool sista) {
  //sätt in dispenserfunktion och kyllådefunktion.

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


void pomodorocykel(int mode) { //kan ta in en string som enkapsulerar båda istället
  //timerstart
  unsigned long cykelstart = millis();
  //kontrollera mobil
  while (true) {

    unsigned long cykeltid = millis() - cykelstart;
    unsigned long tid_kvar = cykeltid/1000;
    if (kontrollera_mobil() == false) {
      varning();
    }

    int itryckt = kolla_knappar(3);
      // om knapp itryckt: hur länge är den itryckt? 
      if (itryckt != 67) {
        pixels.setPixelColor(0, pixels.Color(150, 0, 0));
        pixels.show();
        itryckt_tid = knapptid(itryckt);
        pixels.setPixelColor(0, pixels.Color(0, 150, 0));
        pixels.show();
        if (itryckt == 12 && itryckt_tid >= 5000) { //setupknapp
          break;
      }

    
    if (cykeltid >= 1500000) {
      break;
    }
    
    lcd.printCenter("Tid kvar:", 0);
    lcd.printCenter(char(tid_kvar), 1);
  }
  bool sista = false;
  pris(sista);
}
}


void pomodoromaskin(int loopar, int mode) { //skulle eventuellt kunna ha att den tar in en str som typ "mode".
  for (loopar > 0; loopar--;) {
    pomodorocykel(mode);
  }
}


void loop() {
  int igang = 1;
  int steg = 1;
  //huvudsystemet
  
  while (igang == 1) {
    
    if (steg == 1) {
    encoder.setPosition(0);
    lastPos = 0;
    }
    //steg 1: Vilka inställningar?
    while (steg == 1) {

      if (lcd_update_checker()==true) {
      update_lcd(steg);
      }
    
      int itryckt = kolla_knappar(steg);
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

      //behövs kod för rot.encoder här.
      //mode = ""; mode ändras till något här som kommer användas till

      if (itryckt == 12 && itryckt_tid >= 5000) { //setupknapp
        // servo_öppnare() // typ nåt sånt här som låter en ladda upp med dricka.
      }
      
      else if (itryckt == 19 && itryckt_tid >= 50) { //selectknapp
        steg = 2; // den kommer genom kolla_knappar registrera snurrningarna och sedan trycker man select för att gå vidare.
      }

    }

    if (steg == 2) {
    encoder.setPosition(0);
    lastPos = 0;
    }

    //steg 2: hur många cykler? Rot. encoder snurrar för att bestämma antal.
    while (steg == 2) {

      if (lcd_update_checker()==true) {
        update_lcd(steg);
      }

      int itryckt = kolla_knappar(steg);
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
      // kör pomodoro här.
      pomodoromaskin(antal_pomodoro, mode); // lägg in de andra variablarna allt eftersom.
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