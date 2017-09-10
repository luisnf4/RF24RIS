#include "RF24Network_config.h"
#include "RF24.h"
#include "RF24Network.h"


bool hijos[3]={false,false,false};
bool miembros[3]={false,false,false};


RF24Network::RF24Network( RF24& _radio ): radio(_radio)
{
}


void RF24Network::begin(int direccion, bool _ch, int _miembro){
	paqRecibidos=0;
	set_node(direccion,1);
	my_masc= 0xfffe << (5-level)*3;
	mi_costo=0;
  	radio.setChannel(90);
  	radio.setDataRate(RF24_1MBPS);
  	radio.setCRCLength(RF24_CRC_16);
  	radio.setPALevel(1);
  	radio.setAddressWidth(5);
  	radio.maskIRQ(1,1,0);
  	ch=_ch;
  	if (ch)
  	{
  		int i;
  		miembro=0;
  		for (i= 0; i < 6; i++){
  			uint64_t dirpipe=direccion_pipe(my_address,i);
  			radio.openReadingPipe(i,dirpipe);
  		}	
  	}
  	else{
  		miembro=_miembro;
  		uint64_t dirpipe=direccion_pipe(my_address,4);
  		radio.openReadingPipe(0,dirpipe);
  	}
  	retransmision();
    lleno=0;
    vacio=0;
    int i;
    for (i = 0; i < 10; i++){
    	buffer[i].espacio=true;
    	buffer[i].descubrimiento=false;
    }
   	radio.startListening();
}


void RF24Network::beginRaiz(){

	set_node(0,1);
	mi_costo=0;
	my_masc= 0xfffe << (5-level)*3;
  	radio.setChannel(90);
  	radio.setDataRate(RF24_1MBPS);
  	radio.setCRCLength(RF24_CRC_16);
  	radio.setAddressWidth(5);
  	radio.setPALevel(1);
  	radio.maskIRQ(1,1,0);
  	//Se definen los campos que se enviaran para el descubrimiento
  	strcpy(inte.men,"saludo");
  	inte.address=my_address;
  	inte.level=level;
  	inte.costo=mi_costo;
  	inte.hijos=1;
  	inte.miembros=1;
  	direccion_movil=1;
  	int i;
  	for (i= 0; i < 6; i++){
  		uint64_t dirpipe=direccion_pipe(my_address,i);
  		radio.openReadingPipe(i,dirpipe);
  	}

    lleno=0;
    vacio=0;
    for (i = 0; i < 10; i++){
    	buffer[i].espacio=true;
    	buffer[i].descubrimiento=false;
    }
  	radio.startListening();
}

void RF24Network::beginCH(){

	my_address = 0;
	ch=true;

	randomSeed(analogRead(1));
	randomid=random(10,30);

  	radio.setChannel(90);
  	radio.setDataRate(RF24_1MBPS);
  	radio.setCRCLength(RF24_CRC_16);
  	radio.setAddressWidth(5);
  	radio.openReadingPipe(0,0xf0f0f0f0fe);
  	radio.maskIRQ(1,1,0);
  	while(my_address==0)
  		solicitar();

  	//Se definen los campos que se enviaran para el descubrimiento
   	strcpy(inte.men,"saludo");
   	inte.address=my_address;
  	inte.level=level;
  	inte.costo=mi_costo;
  	inte.hijos=true;
  	inte.miembros=true;
  	int i;
  	for (i= 0; i < 6; i++){
  		uint64_t dirpipe=direccion_pipe(my_address,i);

  		radio.openReadingPipe(i,dirpipe);
  	} 
  	retransmision();
    lleno=0;
    vacio=0;
    for (i = 0; i < 10; i++){
    	buffer[i].espacio=true;
    	buffer[i].descubrimiento=false;
    }
  	radio.startListening();
	
}


void RF24Network::beginCM(){

	my_address = 0;
	miembro=0;

	randomSeed(analogRead(1));
	randomid=random(10,30);

	ch=false;
  	radio.setChannel(90);
  	radio.setDataRate(RF24_1MBPS);
  	radio.setCRCLength(RF24_CRC_16);
  	radio.setAddressWidth(5);
  	radio.maskIRQ(1,1,0);
  	while(miembro==0)
  		solicitar();
  	//Solo se necesita el pipe 4
  	printf("soy el miembro %d del nodo %x\n", miembro, my_address );
  	uint64_t dirpipe=direccion_pipe(my_address,4);
  	radio.openReadingPipe(0,dirpipe);
  	while(direccion_movil==0){
  		solMiembro();
  	}

  	retransmision();
    lleno=0;
    vacio=0;
    int i;
    for (i = 0; i < 10; i++){
    	buffer[i].espacio=true;
    	buffer[i].descubrimiento=false;
    }
  	radio.startListening();
}

bool RF24Network::solicitar(){
	darDir aux[5];
    int i=0;
    radio.openReadingPipe(0,0xf0f0f0f0fe);
    bool ok=false;
    if (conocer(aux,i)){
    	ok=comparar(i,aux);
    }
    return ok;
}


bool RF24Network::conocer(darDir aux[5], int &i){
	radio.startListening();
	bool conocido=false;
	unsigned long tiempo=millis();
	bool timeout=false;
	i=0;
	while(!timeout){
		if(millis()-tiempo > 5000 || i > 4)       //Tiempo de espera de 5 segundos
			timeout=true;
		while(radio.available()){
			radio.read( &aux[i], sizeof(aux[i]) );		
			if(strcmp(aux[i].men,"saludo")==0){
				strcpy(aux[i].men,"potencia");
				aux[i].id=randomid;
				if( (ch && aux[i].hijos) || (!ch && aux[i].miembros) ){
					i++;
					conocido = true;
				}
				
			}
		}
	}
	return conocido;
}


bool RF24Network::comparar(int &i, darDir aux[5]){
	darDir aux2;
	bool ok=false;
	uint8_t j=0,k=0, menor;
	uint8_t costos[5];
	unsigned long started_waiting_at;
	
	//Ciclo que verifica el costo de enlace con cada vecino
	for (j = 0; j < i ; j++)
	{
		uint64_t dirpip=direccion_pipe(aux[j].address,5,aux[j].level);
		for(k=0; k < 4; k++)
		{
			radio.setPALevel(k);
			radio.stopListening();
			radio.openWritingPipe(dirpip);
			radio.write(&aux[j], sizeof(aux[j]),1);
			radio.startListening();

			started_waiting_at = millis();
    		while (millis() - started_waiting_at < 50 )
    		{
				if(radio.available())
				{
					radio.read(&aux2,sizeof(aux2));
					if(!strcmp(aux2.men,"potencia") && aux2.id==randomid)
					{
						ok=true;
						break;
					}	
				}
    		}
			if(ok) break;
		}
		if(k==4)
			costos[j]=0;
		else
		{
			costos[j]=aux[j].costo+k+1;
			aux[j].costo = costos[j];
		}
	}
	while(1)
	{
		k=6;
		menor=26;
		for(j=0;j < i; j++)
			if (menor > costos[j] && costos[j]!=0)
			{
				menor=costos[j];
				k=j;
			}
		if(menor==26 || k == 6)
			break;
		if(ok=negociar(aux[k]))
			break;
		else
			costos[k]=0;
	}
	return ok;

}


bool RF24Network::negociar(darDir aux){
	bool logro=false;
	bool timeout = false;
	char a[]="null";
 	
 	//Se construye el paquete para la solicitud
 	if(ch)
 		strcpy(aux.men,"descubrir");
 	else
 		strcpy(aux.men,"descubris");
 	aux.id=randomid;
 	
 	//Se envia el paquete y se espera respuesta
 	level=aux.level;
 	uint64_t dirpipe=direccion_pipe(aux.address,5,aux.level);
	uint16_t server_address=aux.address;
	radio.openWritingPipe(dirpipe);
	radio.stopListening();
	radio.write( &aux , sizeof(aux), 1);
	radio.startListening();

	//Se espera respuesta, en caso de no recibir ninguna, se vence el tiempo
	unsigned long started_waiting_at = millis();
    while (! timeout )
    {
	    while(radio.available())
	    {
	    	radio.read( &aux, sizeof(aux) );
			if(!strcmp(aux.men,"asignar") && aux.id==randomid )
			{	   
			    uint8_t intentos=3;
			    paquete paq;
			    strcpy(paq.mensaje,a);
			    paq.origen=aux.address;
			    paq.destino=server_address;
			    if (ch)
			    {
				    paq.opciones= (aux.address^server_address) >> (5-aux.level)*3+1;
				    dirpipe=direccion_pipe(aux.address,0,aux.level);
			    }
			    else
			    {
			    	paq.opciones=aux.miembro;
			    	dirpipe=direccion_pipe(aux.address,4,aux.level);
			    }
			    radio.stopListening();
			    
			    radio.openWritingPipe(dirpipe);

		    	while(intentos--){
		    		logro=radio.write(&paq,sizeof(paq));
			    	if(logro)
			    	{
				    	my_address=aux.address;
				    	level=aux.level;
				    	my_masc= 0xfffe << (5-level)*3;
				    	mi_costo=aux.costo;
				    	if(!ch)
				    		miembro=aux.miembro;
				    	break;	
			    	}
			    }
			}
		   	if(logro) break;   	
		}
	    if(logro) break;
		if (millis() - started_waiting_at > 500 )  //Se esperan 500ms para realizar la negociación. Se puede reducir
        	timeout = true;
	}
	return logro;
}


void RF24Network::asignar(darDir aux){

	uint8_t i;
	if(!strcmp(aux.men,"descubrir")){
		for(i=1 ; i<4 ; i++)
			if(hijos[i-1]==false)
				break;
		if(i==4){
			inte.hijos=false;
			return;
		}
		aux.address = my_address | i << ((5-level)*3-2); 
		aux.level=level+1;
	}
	else
	{
		for(i=1 ; i<4 ; i++){
			if(miembros[i-1]==false){
				break;
			}
		}
		if(i==4){
			inte.miembros=false;
			return;
		}
		aux.miembro = i;
		aux.level=level;                  
	}
	enviarDarDir(aux, "asignar");
}

void RF24Network::interrupcion(){
  	radio.stopListening();
  	radio.openWritingPipe(0xf0f0f0f0fe);
  	radio.write(&inte, sizeof(inte),1);
  	radio.startListening();
}

void RF24Network::enviarDarDir(darDir aux, char *mensaje){
	strcpy(aux.men,mensaje);
	delay(20);
	noInterrupts();
  	radio.stopListening();
  	radio.openWritingPipe(0xf0f0f0f0fe);
  	radio.write(&aux, sizeof(darDir),1);
  	radio.startListening();
  	interrupts();
}


void RF24Network::update(){
	uint8_t pipe_num;
	while( radio.available(&pipe_num)){
		if (buffer[lleno].espacio)
		{
			if (pipe_num==5)
			{
				radio.read( &buffer[lleno].des , sizeof(darDir) );
				buffer[lleno].descubrimiento=true;
			}
			else
				radio.read( &buffer[lleno].paq , sizeof(paquete) );
			incluir();
		}
		else{
			radio.read( &paq_basura , sizeof(paquete) );
		}
	}
}

void RF24Network::solMiembro(){
	paquete paq;
	paq.origen=my_address;
	paq.miembro_origen=miembro;
	strcpy(paq.mensaje,"direccion");
	paq.destino=0;
	uint16_t espera=(level+1)*1000;
  	radio.stopListening();
  	radio.openWritingPipe(direccion_pipe(my_address,4));
  	radio.write(&paq, sizeof(paq));
  	radio.startListening();
  	unsigned long started_waiting_at=millis();
 	while (millis() - started_waiting_at < espera ){
		if(radio.available())
		{
			radio.read(&paq,sizeof(paq));
			if(!strcmp(paq.mensaje,"direccion") && paq.miembro_origen==miembro){
				direccion_movil=paq.opciones;
				break;
			}
		}
 	}
}


void RF24Network::enrutarMiembro(){

	while(buffer[vacio].espacio==false)
	{
		if(my_address==buffer[vacio].paq.destino && miembro==buffer[vacio].paq.miembro_origen)
		{
			printf("me llego un mensaje\n");  //Añadir procesamiento que se desee realizar
		}
		else
			sendtoPipe(buffer[vacio].paq,0);  //Cualquier otro paquete, enviar al padre
		sacar();
	}
}


bool RF24Network::enrutar(){
	uint16_t nodo;
	uint8_t a= (4-level)*3+1;
	uint16_t b=0xe000;
	bool ok=false;

	while(buffer[vacio].espacio==false)
	{	

		//Cuando el paquete es de solicitud de dirección
		if (buffer[vacio].descubrimiento==true)
		{
			if (!strcmp(buffer[vacio].des.men,"descubrir") || !strcmp(buffer[vacio].des.men,"descubris") )
				asignar(buffer[vacio].des);
			else if(!strcmp(buffer[vacio].des.men,"potencia"))
  				enviarDarDir(buffer[vacio].des,"potencia");
			buffer[vacio].descubrimiento=false;
		}

		//Si el paquete recibido tiene como dirección de destino el nodo actual
		else if(my_address==buffer[vacio].paq.destino)
		{
			if (my_address==buffer[vacio].paq.origen)                      
			{
				if(!strcmp(buffer[vacio].paq.mensaje,"null")){			   //Confirmación de dirección de nodo espezializado
					miembros[buffer[vacio].paq.opciones-1]=true;
				}
				else if (!strcmp(buffer[vacio].paq.mensaje,"direccion"))   //Asignación de dirección móvil para nodo esp.
				{														   //hijo del nodo raíz				
					buffer[vacio].paq.opciones=direccion_movil;
					sendtoPipe(buffer[vacio].paq,4);
					direccion_movil++;
				}
			}
			else if(!strcmp(buffer[vacio].paq.mensaje,"null")){			    //Confirmación de dirección de cabeza de grupo
				hijos[buffer[vacio].paq.opciones-1]=true;
			}

			else if(!strcmp(buffer[vacio].paq.mensaje,"direccion") && my_address==0){   //Asignación de dirección móvil para nodo esp.
				write(buffer[vacio].paq.origen,"direccion", direccion_movil, buffer[vacio].paq.miembro_origen);
				direccion_movil++;
			}
			else if (!strcmp(buffer[vacio].paq.mensaje,"direccion"))
				sendtoPipe(buffer[vacio].paq,4);
			
			else
			{ 
				//Serial.println(buffer[vacio].paq.mensaje);          //Muestra en pantalla mensaje recibido (opcional)
				//paqRecibidos++;                                     //opcional
				//Añadir procesamiento que se desee con el paquete. En este caso, se reenvía el mensaje al nodo orígen
				//si el nodo actual es la raíz
				if(my_address==0)
					write(buffer[vacio].paq.origen,buffer[vacio].paq.mensaje);
				
				ok=true;
				radio.startListening();
			}
		}
		else if(my_address == (buffer[vacio].paq.destino & my_masc))	//Redirige a nodo hijo
		{
			nodo = (buffer[vacio].paq.destino & (b >> level*3)) >> a ;
			sendtoPipe(buffer[vacio].paq,nodo);
		}
		else{
			sendtoPipe(buffer[vacio].paq,0);							//Redirige a nodo padre
		}

		sacar();
	}
	return ok;
}

void RF24Network::retransmision(){
	uint8_t i;
	uint8_t suma=0;
	uint16_t aux=my_address;
	for (i = 0; i < 5; i++)
	{
		suma= suma + (aux & 0xe000);
		aux<<3;
	}
	if (miembro!=0)
	{
		suma=suma+3+miembro;
	}
	if (suma > 7)
	{
		suma=suma-7;
	}
	radio.setRetries(suma,15);
}

bool RF24Network::probar(uint8_t nodo){

	paquete paq;
	paq.origen=0;
	paq.destino=0;
	strcpy(paq.mensaje,"nulo");
	bool ok = false;
	uint64_t out_pipe = direccion_pipe( my_address, nodo );
	radio.stopListening();
	radio.openWritingPipe(out_pipe);
	short attempts = 5;
	do
	{
		ok = radio.write( &paq , sizeof(paq) );
		delayMicroseconds(16000);
	}
	while ( !ok && --attempts );
	radio.startListening();
	return ok;

}


void RF24Network::write(uint16_t dest, char* men, uint8_t opc, uint8_t miembro_t)
{
	noInterrupts();
	if(buffer[lleno].espacio==true)
	{
		buffer[lleno].paq.origen=my_address;
		buffer[lleno].paq.destino=dest;
		buffer[lleno].paq.opciones=opc;
		buffer[lleno].paq.miembro_origen=miembro_t;
		strcpy(buffer[lleno].paq.mensaje,men);
		incluir();
		//paqEnviados++; //opcional
	}
	interrupts();
}

void RF24Network::write(uint16_t dest, char* men)
{
	noInterrupts();
	if(buffer[lleno].espacio==true)
	{
		buffer[lleno].paq.origen=my_address;
		buffer[lleno].paq.destino=dest;
		if (!ch)
			buffer[lleno].paq.miembro_origen=miembro;
		strcpy(buffer[lleno].paq.mensaje,men);
		incluir();
		//paqEnviados++;	//opcional
	}
	interrupts();
}

void RF24Network::sacar()
{
	buffer[vacio].espacio=true;
	vacio++;
	if (vacio==10)
		vacio=0;
}

void RF24Network::incluir()
{
	buffer[lleno].espacio=false;
	lleno++;
	if(lleno==10) 
		lleno=0;
}

bool RF24Network::sendtoPipe(paquete paq, uint8_t pipe){
	bool ok = false;
	uint64_t out_pipe = direccion_pipe( my_address, pipe ); 
	noInterrupts();
	radio.stopListening();
	radio.openWritingPipe(out_pipe);
	short attempts = 1;
	do
	{
		ok = radio.write( &paq , sizeof(paq) );
	}
	while ( !ok && --attempts );
	radio.startListening();
	interrupts();
	return ok;
}


uint16_t RF24Network::set_node(int Num, uint8_t opc){
	uint16_t node=0;
	int shift = 13;
	uint16_t a[5]={};
	int Contador=0;
    while(Num != 0){
        a[Contador] = Num%10;
        Contador++;
        Num=(Num / 10);
    }
    Num=Contador;
    int i=4;
	while(i>=0){
		if(a[i] != 0){
			if(a[i]>5 || a[i]<0){
				node=1;
				break;
			}
			node = node | a[i] << shift;
			shift-=3;
		}
		i--;
	}
	if(opc==1){
		level=Contador;
		my_address=node;
	}
	return node;
}


uint64_t RF24Network::direccion_pipe(uint16_t node, uint8_t pipe, uint8_t level){
	static uint8_t pipe_segment[] = { 0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5 };
	uint64_t result=0;
	uint8_t* out = reinterpret_cast<uint8_t*>(&result);
	int i=1,j=4;
	uint16_t masc=0xE000;
	uint16_t aux;
	if (pipe==0)
		i--;
	for (i; i < 5-level ; i++)
	{
		out[j]=0xF0;
		j--;
	}
	j++;
	while(j>0)
	{
		aux=(node & masc)>>13;
		aux= (out[j] & 0x0F) + aux;
		if(aux>5)
			aux=aux-6;
		out[j-1]=pipe_segment[aux];	
		node=node<<3;
		j--;
	}
	if (pipe)
	{
		aux= (out[1] & 0xF) + pipe;
		if(aux>5)
			aux=aux-6;	
		out[0]=pipe_segment[aux];	
	}
	return result;	
}


uint64_t RF24Network::direccion_pipe(uint16_t node, uint8_t pipe){
	static uint8_t pipe_segment[] = { 0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5 };
	uint64_t result=0;
	uint8_t* out = reinterpret_cast<uint8_t*>(&result);
	int i=1,j=4;
	uint16_t masc=0xE000;
	uint16_t aux;
	if (pipe==0)
		i--;
	for (i; i < 5-level ; i++)
	{
		out[j]=0xF0;
		j--;
	}
	j++;
	while(j>0)
	{
		aux=(node & masc)>>13;
		aux= (out[j] & 0x0F) + aux;
		if(aux>5)
			aux=aux-6;
		out[j-1]=pipe_segment[aux];	
		node=node<<3;
		j--;
	}
	if (pipe)
	{
		aux= (out[1] & 0xF) + pipe;
		if(aux>5)
			aux=aux-6;	
		out[0]=pipe_segment[aux];	
	}
	return result;
}

