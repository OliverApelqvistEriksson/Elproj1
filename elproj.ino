
//lcd
#include <DIYables_LCD_I2C.h> // Library for LCD
#include <Stepper.h> // stepper motor
#include <Adafruit_NeoPixel.h> //gissa

//försök 2 rot encoder
#define CLK A1
#define DT A2



#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        A3 // On Trinket or Gemma, suggest changing this to 1
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1 // Popular NeoPixel ring size
// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.

// Servobibblan
#include <Servo.h>

// Servopin
int servoPin = 10;

// Variabel för att förvara servopositionen
int pos;

//Servoobjekt
Servo Servo1;


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

int counter = 0;
int lastStateCLK;




Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);






void setup() {
  //neopixel
  Serial.begin(9600);
  pixels.begin();

  //snurrknapp
  //encoder.setPosition(10 / ROTARYSTEPS); // start with the value of 10.
  //snurrknapp lösning 2 lastStateCLK
  pinMode(CLK, INPUT_PULLUP); // Critical for 3-pin encoders
  pinMode(DT, INPUT_PULLUP);
  Serial.begin(9600);
  lastStateCLK = digitalRead(CLK);

  //lcd
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight
  lcd.setCursor(5, 0);

  Servo1.attach(servoPin);

 //resten av knapparna
  pinMode(buzzer, OUTPUT);
  pinMode(setupknapp, INPUT_PULLUP);
  pinMode(startknapp, INPUT_PULLUP);
  
  // stepper motor
  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);
  myStepper.setSpeed(10);

  //startsignal
  lcd.print("Startar...");
  delay(1000);
  pixels.setPixelColor(0, pixels.Color(50, 50, 50));
  pixels.show();
  spela_truddilutt(1000, 2000);
  lcd.clear();


}

void lcd_pomodoro_setup() {
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("MODE "); //kom ihåg att börja Mode-symbol på 5
  lcd.setCursor(8,1);
  lcd.print("ROUNDS: "); // kom ihåg 15
}

void lcd_tidkvar(int kvar) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Aktiv Pomodoro!");
  lcd.setCursor(0,1);
  lcd.print("Kvar: ");
  lcd.setCursor(6, 1);
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
      lcd.print("dis");}


    lcd.setCursor(15,1);
    //print här
    lcd.print((antal_pomodoro));
}

bool lcd_update_checker() { 
  unsigned long sixseven = millis();
  if ((sixseven % 100) == 67) {
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
void spela_truddilutt(int buzztid, int crescendo) {
  int inc = crescendo / buzztid;
  for (int x; x < buzztid; x++) {
    int ton = x*inc;
    tone(buzzer, ton);
    delay(1);
    noTone(buzzer);
  }
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

    if (itryckt == startknapp && itryckt_tid >= 20) { //startknapp
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
  int max1 = 4;
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
  delay(5000); // Vänta 5 sekunder låst

}

void snurraStepper(int varv){
  analogWrite(ena, 225); 
  analogWrite(enb, 225);
  myStepper.step(varv*stepsPerRev);

  // 2. Sänk till "Hållström" (Låst men svalare)
  // Testa dig fram, t.ex. 80 ger tillräckligt lås men mycket mindre värme
  analogWrite(ena, 150); 
  analogWrite(enb, 150);
}


void pris(bool sista) {
  //sätt in dispenserfunktion och kyllådefunktion.
  dispenser();
  if (sista == true) {
    lock();
  }
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


void pomodorocykel(int length) { //kan ta in en string som enkapsulerar båda istället
  //timerstart
  unsigned long cykelstart = millis();
  //kontrollera mobil
  while (true) {

    unsigned long cykeltid = millis() - cykelstart;
    unsigned long tid_kvar = (cykeltid/1000);
    if (kontrollera_mobil() == false) {
      varning();
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
        if (itryckt == setupknapp && itryckt_tid >= 5000) { //setupknapp
          return;
      }



      }
    if ((cykeltid >= 20000) && (length == 0) ) {
      break;
    }
    if (cykeltid >= 1500000) {
      break;
    }
    if (lcd_update_checker() == true) {
      lcd_tidkvar(tid_kvar);
    }
  }
  bool sista = false;
  pris(sista);
}


void pomodoromaskin() { //skulle eventuellt kunna ha att den tar in en str som typ "mode".
  for (; antal_pomodoro > 0; antal_pomodoro--) {
    pomodorocykel(25);
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

      if (itryckt == setupknapp && itryckt_tid >= 5000) { //setupknapp
        // servo_öppnare() // typ nåt sånt här som låter en ladda upp med dricka.
      }
      
      else if (itryckt == startknapp && itryckt_tid >= 50) { //selectknapp
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
      
      if (itryckt == setupknapp && itryckt_tid >= 50) { //setupknapp
        steg = 1;
        break;
      }
      
      else if (itryckt == startknapp && itryckt_tid >= 50) { //selectknapp
        steg = 3;
        break;
      }
    
    }

    //steg 3: kör programmet enligt instruktioner från 1-2.
    while (steg == 3) {
      if (mode == 1) {
        pomodorocykel(0);
      }
      else if (mode == 4) {
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