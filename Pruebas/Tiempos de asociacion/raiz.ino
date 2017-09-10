#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include "printf.h"
#include <TimerOne.h>


//nRF24L01(+) radio attached using Getting Started board 
RF24 radio(7,8);

// Network uses that radio
RF24Network network(radio);

char a[20];
unsigned long start_int;

void setup() 
{
  Serial.begin(57600);
  printf_begin();
  SPI.begin();
  radio.begin();
  network.beginRaiz();
  radio.printDetails();
  attachInterrupt( 0, prueba, LOW);
  Timer1.initialize(5000000);         // Dispara cada 5 segundos
  Timer1.attachInterrupt(tiempo);     // Activa la interrupcion y la asocia a ISR_Blink
  start_int = millis();
}

void loop() 
{
  network.enrutar();

}

void prueba()
{
  network.update();
}

void tiempo()
{
  network.interrupcion();
}

