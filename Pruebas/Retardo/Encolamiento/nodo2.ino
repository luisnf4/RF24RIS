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

unsigned long start_enru;
unsigned long stop_enru;

volatile bool recibido=false;

  int butam=1;		//Cantidad de paquetes que se encuentran en el bufer
  int i=0;

void setup() 
{
  Serial.begin(57600);
  printf_begin();
  SPI.begin();
  radio.begin();
  network.begin(direccion,ch,miembro);
  radio.printDetails();
  attachInterrupt( 0, prueba, LOW);

}

void loop() 
{
  for(i=0;i<butam;i++){
    network.write(0,"prueba");
  }
  while(!recibido);
  start_enru=micros();
  network.enrutar();
  stop_enru=micros();
  Serial.print(F("Retardo: "));
  Serial.print(stop_enru-start_enru);
  Serial.println(F(" microsegundos"));
  recibido=false;
  
}

void prueba()
{
  recibido=true;
  network.update();
}



