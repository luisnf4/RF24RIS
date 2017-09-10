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


uint8_t i=0;
char a[10]="prueba";
unsigned long start_w;



void setup() {
  Serial.begin(57600);
  Serial.println("RF24Network/examples/raiz/");
  printf_begin();
  SPI.begin();
  radio.begin();
  //network.begin(direccion,ch,miembro);
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


