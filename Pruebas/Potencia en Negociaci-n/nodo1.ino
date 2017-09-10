#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include "printf.h"
#include <TimerOne.h>


//nRF24L01(+) radio attached using Getting Started board 
RF24 radio(7,8);

// Network uses that radio
RF24Network network(radio);


uint8_t i=0;
char a[10]="prueba";
unsigned long start_w;

bool hola=false;

void setup() {
  Serial.begin(57600);
  Serial.println("RF24Network/examples/raiz/");
  printf_begin();
  SPI.begin();
  radio.begin();
  network.beginCH();
  attachInterrupt( 0, prueba, LOW);
  radio.printDetails();
  Timer1.initialize(2000000);         // Dispara cada 5 segundos
  Timer1.attachInterrupt(tiempo);     // Activa la interrupcion y la asocia a ISR_Blink
  start_w = millis();
  delay(1000);
}

void loop() 
{
   
   if(!hola){
    printf("Nivel de potencia quedo en %d", radio.getPALevel());
    hola=true;
   }

    
}

void prueba()
{
    network.update();
}


void tiempo()
{
    network.interrupcion();
}


