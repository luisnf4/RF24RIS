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
int direccion= 4;                   //modificar cuandose usa más de un nodo

//Miembro: en caso de que el nodo no sea cluster head
int miembro=0;

unsigned long start_enru;
unsigned long stop_enru;

volatile bool recibido=false;


volatile long int i=0;

void setup() 
{
  Serial.begin(57600);
  printf_begin();
  SPI.begin();
  radio.begin();
  network.begin(direccion,ch,miembro);
  radio.printDetails();
  attachInterrupt( 0, prueba, LOW);
  start_enru=millis();
}

void loop() 
{
  if(i<910){
    network.write(0,"prueba1");
    network.enrutar();
    i++;
  }
  /*else{
    stop_enru=millis();
    printf("tiempo %ld", stop_enru-start_enru);
    delay(1000000000);
  }*/
  

    
}

void prueba()
{

  network.update();
}

