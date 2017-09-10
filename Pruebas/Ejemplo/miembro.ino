#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include "printf.h"
#include <TimerOne.h>


//nRF24L01(+) radio attached using Getting Started board 
RF24 radio(7,8);

// Network uses that radio
RF24Network network(radio);

/*
//indica si el nodo es el raiz
bool raiz=false;

//Indica si el nodo es un cluster head
bool ch=false;

//Direccion que se le asigna al nodo
int direccion=2;

//Miembro: en caso de que el nodo no sea cluster head
int miembro=1;
*/
char a[10];

int i=0;

void setup() {
  Serial.begin(57600);
  Serial.println("RF24Network/examples/raiz/");
  printf_begin();
  SPI.begin();
  radio.begin();
  //network.begin(direccion,ch,miembro);
  network.beginCM();
  //radio.printDetails();
}

void loop() {
    i++;
    sprintf(a,"prueba%d",i);
    network.write(0,a);
    network.enrutar();
    delay(5000);

}
