#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include "printf.h"
#include <TimerOne.h>


//nRF24L01(+) radio attached using Getting Started board 
RF24 radio(7,8);

// Network uses that radio
RF24Network network(radio);


//indica si el nodo es el raiz
bool raiz=false;

//Indica si el nodo es un cluster head
bool ch=true;

//Direccion que se le asigna al nodo
int direccion=1;

//Miembro: en caso de que el nodo no sea cluster head
int miembro=0;

int i=0;

unsigned long start_int;

void setup() 
{
  Serial.begin(57600);
  printf_begin();
  SPI.begin();
  radio.begin();
  network.begin(direccion,ch,miembro);
  radio.printDetails();
 
}

void loop() 
{
  
  while(i<10){
    network.write(0,"prueba");
    network.enrutar();
    i++;
    delay(2000);
  }
  

}


