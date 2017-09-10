#ifndef __RF24NETWORK4_H__
#define __RF24NETWORK4_H__

#include "RF24.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


//Estructura de los paquetes que viajan a traves de la red
typedef struct paquete_p
{
	uint16_t origen;         //Nodo Origen
	uint16_t destino;        //Nodo Destino
 	uint8_t miembro_origen;  //Miembro origen o destino de la información
	uint8_t opciones;        //Campo de opciones
	char mensaje[26];        //Carga útil del paquete
}paquete;

//Estructura de los paquetes para negociación de direcciones
typedef struct dar
{
  uint16_t address;        //Inicialmente direccion de CH, luego se usa para asignar 
  uint8_t miembro;         //Número de miembro que se asigna 
  uint16_t id;             //Identificador aleatorio
  uint8_t level;           //Nivel en el árbol
  uint8_t costo;           //Costo del Enlace
  bool hijos;              //¿Direcciones (CH) disponibles?
  bool miembros;           //¿irecciones de miembros disponibles?
  char men[10];            //Carga útil
  
}darDir;


//Estructura del búfer de recepción y transmisión
typedef struct buffer_t
{
  paquete paq;            //Paquete regular de la red
  darDir  des;            //Paquete de descubrimiento
  bool espacio;           //¿La información del bloque fue leída?
  bool descubrimiento;    //¿El paquete es de descubrimiento?
}buffer_r;


class RF24Network
{

public:
  /* Constructor
   *  @param _radio Objeto constructor de la clase RF24
   */
  RF24Network(RF24& _radio);

  /* Comienzo de la operación de la red
   *  Se usa para la configuración estática de direcciones
   *  Parámetros:
   *  direccion Dirección del nodo
   *  _ch       ¿El nodo es cabeza de grupo?
   *  _miembro  Si no es cabeza de grupo, ¿Número de miembro?
   */
  void begin(int direccion, bool _ch, int _miembro);

  /* Comienzo de la operación del nodo raíz cuando 
   * las asignación de direcciones es dinámica
   */
  void beginRaiz();

  /* Comienzo de la operación de un nodo CH cuando 
   * las asignación de direcciones es dinámica
   */
  void beginCH();

  /* Comienzo de la operación de un nodo miembro cuando 
   * las asignación de direcciones es dinámica
   */
  void beginCM();  

  /* Almacena los paquetes recibidos en el búfer
   */
  void update();

  /* Función para el procesamiento de los paquetes
   * Selecciona el destino adecuado para los paquetes en el bufer, o los procesa
   * según sea el caso.
   * Si los paquetes recibidos son para solicitar dirección, se asigna o no
   * dependiendo de si hay direcciones disponibles
   */
  bool enrutar();

  /* Envía un paquete por el canal lógico especificado
   * Parámetros:
   * paq Paquete a transmitir
   * pipe Número de canal lógico (0-5)
   */
  bool sendtoPipe(paquete paq, uint8_t pipe);

  /* Calcula la dirección de un nodo a partir de un número decimal
   * Parámetros:
   * Num Número decimal a convertir
   * opc Opción: 1 Establece la dirección resultante como la dirección del nodo
   *             0 Solo calcula la dirección
   * Salida: Dirección convertida
   * Ejemplo: Entrada -> Num=123 opc=0
   *          Salida -> 10624 (decimal) = 0010100110000000 (binario)
   */
  uint16_t set_node(int Num, uint8_t opc);

  /* Añade un paquete al búfer para su transmisión
   * Parámetros:
   * dest       Dirección destino
   * Men        Mensaje añadido en carga útil
   * opc        Opción del paquete
   * miembro_t  Miembro destino del paquete
   */
  void write(uint16_t dest, char* men, uint8_t opc, uint8_t miembro_t);

  /* Añade un paquete al búfer para su transmisión
   * Parámetros:
   * dest       Dirección destino
   * Men        Mensaje añadido en carga útil
   */
  void write(uint16_t dest, char* men);

  /* Calcula la dirección de un canal lógico a partir de una dirección lógica
   * Parámetros:
   * node       Dirección lógica
   * pipe       Número de canal lógico
   * level      Nivel de la dirección lógica
   * Salida:    Dirección de canal lógico
   */
  uint64_t direccion_pipe(uint16_t node, uint8_t pipe, uint8_t level);

  /* Calcula la dirección de un canal lógico a partir de una dirección lógica
   * Se usa el nivel actual del nodo para el cálculo
   * Parámetros:
   * node       Dirección lógica
   * pipe       Número de canal lógico
   * Salida:    Dirección del canal lógico
   */
  uint64_t direccion_pipe(uint16_t node, uint8_t pipe);

  /* Empieza procedimiento de solicitud de dirección
   * Llama a las funciones conocer y comparar
   */
  bool solicitar();

  /* El nodo que recibe una solicitud de dirección asigna
   * la primera dirección disponible que posea. Si no hay direcciones
   * disponibles, no responde la solicitud
   */
  void asignar(darDir aux);

  /* Envia el mensaje de aviso con los datos correspondientes cada
   * 5 segundos. Para ello se debe llamar en una interrupción por tiempo
   */
  void interrupcion();

  /* Empieza procedimiento de solicitud de dirección
   * Llama a las funciones conocer y comparar
   */
  void solicitar();

  /* Recibe los mensajes de aviso de los nodos adyacentes
   * Parámetros:
   * aux[5]     Arreglo donde se almacenan los mensajes de aviso
   * i          Variable donde se almacena el número de vecinos conocidos (máximo 5)
   * Salida:    ¿Se conoció algún vecino? Si=true No=false
   */
  bool conocer(darDir aux[5], int &i);

  /* Compara el costo de todos los vecinos recibidos para seleccionar el de menor costo
   * hacia el sumidero. Llama a la función negociar() tras seleccionarlo
   * Parámetros:
   * aux[5]     Arreglo donde se almacenan los mensajes de aviso
   * i          Variable donde se almacena el número de vecinos conocidos (máximo 5)
   * Salida:    ¿Se conoció algún vecino? Si=true No=false
   */
  bool comparar(int &i, darDir aux[5]);

  /* Solicita dirección al vecino seleccionado en comparar()
   * Parámetros:
   * aux        Paquete que contiene información del vecino seleccionado
   * Salida:    ¿Se obtuvo dirección dentro de la red? Si=true No=false
   */
  bool negociar(darDir aux);

  /* Modifica el bufer de paquetes para establecer un espacio
   * de memoria como vacío
   */
  void sacar();

  /* Modifica el bufer de paquetes para establecer un espacio
   * de memoria como ocupado
   */
  void incluir();

  /* Envía un paquete de negociación de dirección a la dirección física 0xF0F0F0F0FE
   * Parámetros:
   * aux            Paquete con los datos del nodo vecino
   * mensaje        Mensaje que se envía en la carga útil
   */
  void enviarDarDir(darDir aux, char *mensaje);

  /* Envía mensajes de prueba para comprobar si un nodo perdió la conexíon con un vecino
   * Parámetros:
   * nodo           Número de canal lógico de conexión que se desea probar
   * 
   */
  bool probar(uint8_t nodo);

  /* Configura el tiempo entre retransmisiones para el nodo a partir 
   * de su dirección lógica
   */
  void retransmision();

  /* Realiza la solicitud de dirección móvil de un nodo miembro
   * al nodo raíz
   */
  void solMiembro();

  /* Similar a la función enrutar pero para los nodos especializados
   * que siempre deben transmitir al mismo nodo
   */
  void enrutarMiembro();
  
  //volatile int paqRecibidos;   //Variable opcional que permite conocer cuantos paquetes destinados al nodo actual fueron recibidos

  //volatile int paqEnviados;    //Variable opcional que permite conocer cuantos paquetes fueron añadidos correcamente al bufer del
                                 //del nodo actual
  
private:
  
  RF24& radio;                  //Objeto de la clase RF24 para la configuración y uso del nRF24l01+
  uint16_t my_address;          //Dirección lógica del nodo
  uint8_t level;                //Nivel en el arbol
  uint8_t randomid;             //Id aleatorio para el autodescubrimiento
  uint8_t mi_costo;             //Costo hasta el sumidero
  uint16_t my_masc;             //Mascara del nodo
  darDir inte;                  //paquete que se envia para que otros nodos puedan descubrir
  bool ch;                      //Cluster head
  uint8_t miembro;              //nro de miembro en caso de !ch
  uint8_t direccion_movil;      //Dirección para movilidad de nodo especializado
  buffer_r buffer[10];          //bufer para almacenar paquetes
  volatile uint8_t lleno;       //siguiente posicion del bufer para añadir paquete
  volatile uint8_t vacio;       //siguiente posicion del bufer para leer paquete
  paquete paq_basura;           //paquetes que no pueden ser almacenados en el buffer
};

#endif
