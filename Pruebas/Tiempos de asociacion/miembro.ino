#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include "printf.h"
#include <TimerOne.h>


//nRF24L01(+) radio attached using Getting Started board 
RF24 radio(7,8);

// Network uses that radio
RF24Network network(radio);


char a[10];

int i=0;
unsigned long start_w;
unsigned long stop_w;


void setup() {
  Serial.begin(57600);
  Serial.println("RF24Network/examples/raiz/");
  printf_begin();
  SPI.begin();
  radio.begin();
  start_w=millis();
  network.beginCM();
  stop_w=millis();
  printf("tiempo de asociacion= %d",stop_w-start_w);
  delay(1000);
  //radio.printDetails();
}

void loop(){
    /*i++;
    sprintf(a,"prueba%d",i);
    network.write(0,a);
    network.enrutar();
    delay(5000);*/
}
