/***********************************************************************************************/
/*
*  Sketch per la gestione del controllo ambientale tramite sensore PIR e webcam.
*  Raspberry è gestito da RandA, quindi SW2 di RandA deve essere posizionato su Arduino ON.
*  Ha una modalità test in cui Raspberry è sempre acceso. Ma è sempre gestito da RandA.
*  Controlla:
*  - un interruttore con pin D7
*  - un sensore PIR con pin D8
*  - il servo di brandeggio con pin D10
*  Inoltre:
*  - accende/spegne Raspberry
*  - riceve comando di spostamento servo (angolo)
*  - o riceve il comando per il reset dell'allarme 
*  Formato dei comandi:
*  - chiede modalità:   M risposta TEST/CTRL
*  - chiede se allarme: Q risposta YAL/NAL
*  - posiziona webcam:  Annn (nnn: angolo 0-180)
*  - auto scanning:     S
*  - reset allarme:     R
*  Per i comandi senza risposta risponde sempre OK/NOK
*/


#include <Servo.h> 

#define TSTPIN 12  // pin per debug. Input pullup; se chiuso su GND attiva modalità test 

#define SWCPIN 7   // pin collegato all'interruttore
#define PIRPIN 8   // pin collegato al sensore PIR
#define SRVPIN 10  // pin collegato al servo

#define RASPON 4   // pin ON/OFF Raspberry  (RandA)
#define RASPCT 2   // pin Raspberry controll(RandA)

#define RSISON 6   // pin per rilevare se Raspberry acceso

#define TIMEON 5   // tempo di attesa prima di attivare il controllo (in minuti)

Servo swcam;                                 // carica la libreria servo

char EL='\n';                                // carattere di fine record
char buffer[16];                             // buffer per ricezione comando servo

boolean statoON=false;                       // flag di stato attivo/disattivo
boolean statoAlarm=false;                    // flag di stato allarme/no-allarme
boolean statoRaspOn=false;                   // flag di stato Raspberry acceso
boolean fscan=false;                         // flag auto scanning
boolean ftest=false;                         // flag per debug

int dgstep=6;   // gradi di spostamento ad ogni step in caso di scanning automatico


void setup() 
{
  pinMode(RSISON,INPUT);
  pinMode(TSTPIN,INPUT_PULLUP);
  pinMode(SWCPIN,INPUT_PULLUP);
  pinMode(PIRPIN,INPUT);
  pinMode(SRVPIN,OUTPUT);
  pinMode(RASPON,OUTPUT);
  pinMode(RASPCT,OUTPUT);
  swcam.attach(SRVPIN);   // inizializza servo collegandolo al pin
  swcam.write(90);
  Serial.begin(9600);                        // inizializza seriale
  delay(200);
  checkOn();                                 // controlla se Raspberry è già ON e inizializza flag di stato
  if (digitalRead(TSTPIN)==LOW) ftest=true;  // verifica se in modalità test
  if (!ftest) switchRaspOFF();               // se in modalità normale spegne Raspberry
}

void loop() 
{
  if (ftest) modeTestWebCam();               // modalità test webcam 
  else modeAlarm();                          // modalità normale 
  if (fscan) autorot();                      // nel caso di richiesta di scanning ruota di dgstep gradi
  delay(200);
}

void modeTestWebCam()
{
  if (digitalRead(SWCPIN)==LOW)  {switchRaspON();}  // se interruttore ON accende Raspberry 
  if (digitalRead(SWCPIN)==HIGH) {switchRaspOFF();} // altrimenti lo spegne
  if (digitalRead(PIRPIN)==HIGH) statoAlarm=true;   // controlla se PIR attivato
  gestioneWebCam();                                 // permette il controllo da Web
}

void modeAlarm()
{
  testSwitch();                                // verifica se deve attivare o disattivare il controllo
  if (statoON) testPir();                      // se attivo ma non ancora in allarme controlla PIR
  if (statoRaspOn) gestioneWebCam();           // se attivo ed in allarme si predispone al collegamento Web   
}

/********************** Routine per attività controllo *****************************/

void testSwitch()                              // cambia il flag di stato attivo/disattivo
{
  int sw=digitalRead(SWCPIN);                  // legge l'interruttore
  switch (sw)                                  // se LOW (0) attiva se HIGH (1) disattiva
  {                                            // in ogni caso se attivato o disattivato spegne Rasberry
    case 0: if (!statoON) {statoON=true;statoAlarm=false;switchRaspOFF();
                           // Raspberry sarà acceso solo dall'allarme
                              delay(TIMEON*60000);
                           }; break;
    case 1: statoON=false;statoAlarm=false;switchRaspOFF();break; 
  } 
}

void testPir()                                 // cambia il flag di stato no-allarme/allarme
{
  if (statoAlarm) return;                      // se già in allarme non controlla
  if (digitalRead(PIRPIN)==HIGH)               // perché rimane allarmato fino al reset da web
  {
    statoAlarm=true;
    switchRaspON();                            // se allarme allora accende Raspberry 
  }
}

/*********************** Routine comuni *******************************/

void gestioneWebCam()
{
   if (Serial.available()==0) return;
   int n=Serial.readBytesUntil(EL,buffer,16);  // legge da seriale e decodifica i comandi
   if(n>0) {buffer[n]='\0';decode();Serial.flush();}
}

void checkOn()
{
  if (digitalRead(RSISON)==HIGH) statoRaspOn=true;
  else statoRaspOn=false;
}

void switchRaspON()           // se spento, accende Raspberry
{ 
  checkOn();
  if (!statoRaspOn) 
     {
      Serial.end(); 
      switchRasp();
      int i;
      for(i=0;i<10;i++){delay(100);checkOn();if (statoRaspOn) break;}
      delay(10000);
      Serial.begin(9600);
     }
}

void switchRaspOFF()          // se acceso, spegne Raspberry
{                             // lo spegnimento ordinato impiega qualche secondo
  checkOn();
  if (statoRaspOn)  
     {
      switchRasp();
      int i;
      for(i=0;i<100;i++){delay(200);checkOn();if (!statoRaspOn) break;}
     }
}

void switchRasp()              // switch ON/OFF Raspberry (impulso)
{
  digitalWrite(RASPON,HIGH); 
  delay(300);
  digitalWrite(RASPON,LOW);
}

void autorot()
{
  int val=swcam.read();
  int newval=val+dgstep;
  if ((newval > 180)|(newval < 0)) dgstep=-dgstep;
  swcam.write(val+dgstep);
}

/************************** Routine per decodifica comandi Web *************************/

void decode()
{
  char c=toupper(buffer[0]);
  if (c=='M') {cmdMode();return;}
  if (c=='Q') {cmdQuery();return;}
  if (c=='A') {cmdSetAngle();return;}
  if (c=='S') {cmdScan();return;}
  if (c=='R') {cmdReset();return;}
  Serial.println("NOK");
}

void cmdMode()
{
  if (ftest) Serial.println("TEST");
  else Serial.println("CTRL");
}

void cmdQuery()
{
  if (statoAlarm) Serial.println("YAL");
  else Serial.println("NAL");
}

void cmdSetAngle()
{
  if (strlen(buffer)<2) {Serial.println("NOK");return;}  
  int val=atoi(&buffer[1]); 
  swcam.write(val); 
  Serial.println("OK"); 
}

void cmdScan()
{
  if (strlen(buffer)<2) {Serial.println("NOK");return;} 
  if (buffer[1]=='Y'){fscan=true;}
  if (buffer[1]=='N'){fscan=false;}
  Serial.println("OK");
}

void cmdReset()
{
  statoAlarm=false;
  Serial.println("OK");
  if (!ftest) switchRaspOFF();
}


