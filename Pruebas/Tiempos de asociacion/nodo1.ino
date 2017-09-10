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
unsigned long stop_w;



void setup() {
  Serial.begin(57600);
  Serial.println("RF24Network/examples/raiz/");
  printf_begin();
  SPI.begin();
  radio.begin();
  start_w=millis();
  network.beginCH();
  stop_w=millis();
  attachInterrupt( 0, prueba, LOW);
  radio.printDetails();
  Timer1.initialize(5000000);         // Dispara cada 5 segundos
  Timer1.attachInterrupt(tiempo);     // Activa la interrupcion y la asocia a ISR_Blink
  printf("tiempo de asociacion= %d",stop_w-start_w);
  delay(1000);
}

void loop() 
{
   
   network.enrutar();
   if(millis() - start_w > 1000){
        i++;
        sprintf(a,"prueba%d",i);
        network.write(0,a);
        start_w=millis();
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


