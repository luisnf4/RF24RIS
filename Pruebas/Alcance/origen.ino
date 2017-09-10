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


volatile  int i=0;

void setup() 
{
  Serial.begin(57600);
  printf_begin();
  SPI.begin();
  radio.begin();
  network.begin(direccion,ch,miembro);

  radio.setPALevel(0);
  //radio.setDataRate( RF24_250KBPS );
  radio.setDataRate( RF24_1MBPS );
  //radio.setDataRate( RF24_2MBPS );
  
  radio.printDetails();
  attachInterrupt( 0, prueba, LOW);
;
}

void loop() 
{
  
  if(i<200)
  network.write(0,"prueba");
  network.enrutar();
  i++;
  delay(20);  
}

void prueba()
{

  network.update();
}



