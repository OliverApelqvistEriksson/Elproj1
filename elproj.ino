const int buzzer = 26; //vet inte om det är rätt pin men whatevs
const int setupknapp = 12;
const int startknapp = 19;
const int lockinknapp = 67; //är kanske inte här

int setupknapp_state = 0;
int startknapp_state = 0;
int lockinknapp_state = 0;

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(setupknapp, INPUT);
  pinMode(startknapp, INPUT);
  pinMode(lockinknapp, INPUT);

  //startsignal
  delay(1000);
  buzzer(100, 200);
  buzzer(100, 500);
}

void buzzer(int buzztid, int paustid) {
  digitalWrite(buzzer, HIGH);
  delay(buzztid);
  digitalWrite(buzzer, LOW);
  delay(paustid);
}

void kolla_knappar() {
  setupknapp_state = digitalRead(setupknapp);
  startknapp_state = digitalRead(startknapp);
  lockinknapp_state = digitalRead(lockinknapp);

  if (setupknapp_state = HIGH) {

  }
  if (startknapp_state) = HIGH {

  }
  if (lockinknapp_state = HIGH) {

  }
}



void loop() {
  
  //huvudsystemet
  //1. välj antal cykler
  //2. välj om lockin-knapp
  //3. starta system

  //+ om setup hålls in i 8 sek startas display-mode, som bara gör att timern är mycket kortare (30 sek)
 
  //loop här --->
  //timern igång:
  //här buzzer om telefon tas ut (del av telefonkoden)

  //timern avklarad
  //här väljs antingen godisdispenser, drycklock eller båda när tiden är slut.

  //<---- till hit. 


  //avslutningsmeddelande efter och sedan startas programmet om.

}
