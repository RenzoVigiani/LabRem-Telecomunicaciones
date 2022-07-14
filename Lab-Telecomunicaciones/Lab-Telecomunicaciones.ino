// Defino las librerias a utilizar
#include <ArduinoJson.h> // Para el manejo y conversión de JSON a vartiables
#include <UIPEthernet.h> // Para el manejo del Shield Ethernet
#include <Servo.h> // Para el manejo de los Servos.
//----------------------------------------------//
// Defino el servidor y el Puerto
#define server_port 22 // 80
EthernetServer server = EthernetServer(server_port);
//----------------------------------------------//
// Defino variables para Json
#define const_mje 256
#define const_valores 236
//----------------------------------------------//
//------- Defino mensajes de error predeterminado
// una variable error int
uint8_t Errores = 0;
// 0 - Sin errores
// 1 - Error limites de angulo de azimut.
// 2 - Error limites de angulo de elevación.
// 3 - Laboratorio incorrecto.
//----------------------------------------------//
//------- Defino variables globales
// Nombres para los pines GPIO
// Leds
  #define Led_aux1 10 // Indicador aux 1
  #define Led_aux2 11 // Indicador aux 2
  #define Led_aux3 12 // Indicador aux 3
  #define Led_aux4 13 // Indicador aux 4
  #define Led_S1 2  // Indicador Servo 1
  #define Led_S2 3 // Indicador Servo 2
// Servos
  #define azimut_pin 4  // Servo azimut
  #define elevacion_pin 5  // Servo elevación
// Declaramos la variable para controlar el servo
  Servo servo_azimut;
  Servo servo_elevacion;


//////// VAriables de json //////////////
// Estado
  int num_Lab=0;
  bool subLab=true;
  bool iniLab=true;
// Analogico
  int variable_0=0;
  int variable_1=0;
//--- Variables auxiliares ---//
  int ang_azimut_act; //Angulos actuales de cada servo
  int ang_elevacion_act;
  bool bandera_rep=0; // bandera para limitar la cantidad de repeticiones de mensaje de lab incorrecto 

// Constantes auxiliares
  const int limite_min_azimut=0; // Indica el minimo valor que puede tomar distancia
  const int limite_max_azimut=180; // Indica el minimo valor que puede tomar distancia
  const int limite_min_elevacion=0; // indica el minimo valor que puede tomar distancia.
  const int limite_max_elevacion=90; // indica el minimo valor que puede tomar distancia.

void setup(){
  uint8_t myMAC[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // defino mac del dispositivo.
  IPAddress myIP(192,168,1,108); // 172.20.5.140 Defino IP Address del dispositivo. 
  Serial.begin(115200); // Inicializo Puerto serial 0 
  while (!Serial) continue; 
  Ethernet.begin(myMAC,myIP);  // Inicializo libreria Ethernet
  server.begin(); // Start to listen
  Serial.println(F("Server is ready."));
  Serial.print(F("Please connect to http://"));
  Serial.println(Ethernet.localIP());
  Serial.println("Port:" + (String)server_port);
// ------- Defino GPIO MODE (INPUT/OUTPUT)--------  //  
// Leds
  pinMode(Led_aux1, OUTPUT); // Led Motor 1
  pinMode(Led_aux2, OUTPUT); // Led Motor 2
  pinMode(Led_aux3, OUTPUT); // Led Motor 3
  pinMode(Led_aux4, OUTPUT);// Led Aux
  pinMode(Led_S1, OUTPUT); // Led Servo 1
  pinMode(Led_S2, OUTPUT); // Led Servo 2
// Servos
  servo_azimut.attach(azimut_pin);
  servo_elevacion.attach(elevacion_pin);
//------ Definir estados iniciales ------//
}

void loop(){
//////////// Strings de comunicación /////////////
char Mensaje_recibido[const_mje] = {0}; // Mensaje recibido por ethernet. (Comando + JSON) 
char valores_recibidos[const_valores] = {0}; // JSON recibido.  // Wait for an incomming connection
EthernetClient client = server.available(); 
  if(client){ // Si tengo un cliente conectado
    while (client.available()){ 
      if(bandera_rep==1)bandera_rep=0; //reinicio bandera de repetición cuando tengo un mje nuevo.
      Serial.println("New Command");
      client.readBytesUntil('\r', Mensaje_recibido, sizeof(Mensaje_recibido)); // Tomo el mensaje recibido.
      strncpy(valores_recibidos,&Mensaje_recibido[15],(sizeof(Mensaje_recibido)-15)); 
      Serial.print("Mensaje Recibido: ");
      Serial.println(Mensaje_recibido);   
  //      Serial.print("Json_Recibido: ");
  //      Serial.println(valores_recibidos);   
      //------ GET ----- //
      if (strstr(Mensaje_recibido, "GET /HTTP/1.1") != NULL) { // Compruebo si llega un GET, respondo valores actuales
        StaticJsonDocument<256> doc;     
        JsonArray Estado = doc.createNestedArray("Estado");
        Estado.add(num_Lab);
        Estado.add(subLab);
        Estado.add(iniLab);

        JsonArray Analogicos = doc.createNestedArray("Analogicos");
        Analogicos.add(ang_azimut_act);
        Analogicos.add(ang_elevacion_act);

        JsonArray Error = doc.createNestedArray("Error");  
        Error.add(Errores);

        Serial.print(F("Sending: "));
        serializeJson(doc, Serial);
        Serial.println();
    // Write response headers
        client.println(F("HTTP/1.0 200 OK"));
        client.println(F("Content-Type: application/json"));
    // client.println(F("Connection: close"));
        client.print(F("Content-Length: "));
        client.println(measureJsonPretty(doc));
        client.println();
    // Write JSON document
        serializeJsonPretty(doc, client);
    // Disconnect
        client.println(F("Get terminado"));
    //      client.stop();
      }
      //------- POST -----//      
      if (strstr(Mensaje_recibido, "POST /HTTP/1.1") !=NULL) { // Compruebo si llega un POST y respondo, habilito banderas.
        Serial.println("Solicitud de escritura recibida");        
        client.println();
        client.println(F("HTTP/1.1 200 OK"));
        client.println();
        StaticJsonDocument<256> doc; // Creo un doc de json
        DeserializationError error = deserializeJson(doc, valores_recibidos); // Deserializo
        if (error) // Analizo posibles errores.
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }

        JsonArray Estado = doc["Estado"];
        num_Lab = Estado[0]; // 0 [Sist Dig], 1 [Sist Control], 2[Telecomunicaciones], 3[Fisica]
        subLab = Estado[1]; // 1 [SubLab 1], 0 [SubLab 2]
        iniLab = Estado[2]; // 1 [Inicia Experimento], 0 [Finaliza Experimento]

        if(num_Lab==2) // Control de numero de lab.
        {
          JsonArray Analogico = doc["Analogico"];
          variable_0 = Analogico[0];
          variable_1 = Analogico[1];
        //client.stop();
        }
      }
    }
  }
  else{ // Si no está el cliente enviando algo, sigo haciendo lo que corresponde.
      Control();
  }
}

/**
 * @brief Funcion utilizada para el control de laboratorios. "Se analiza las variables de entrada y se ejecuta lo necesario"
 * 
 */
void Control(){
  if(num_Lab==2){
    if (subLab and iniLab){
      Serial.println("Sub - Laboratorio: Wifi 2.4 GHz"); 
      wifi_24_ghz(variable_0, variable_1);
    }
    else{
      if(bandera_rep==0){
        Serial.println("Laboratorio Parado");
        bandera_rep=1;
      }
    }
  }
  else{
    if(bandera_rep==0){
      Serial.println("Laboratorio incorrecto");  
      bandera_rep = 1; 
      Errores=3;
    }
  }
}

/**
 * @brief Funcion utilizada para realizar el movimiento de la antena de a cuerdo al angulo solicitado.
 * 
 * @param azimut 
 * @param elevacion 
 */
void wifi_24_ghz(int azimut, int elevacion){
if (limite_min_azimut <= azimut and azimut <= limite_max_azimut){
  Serial.println("Azimut:");
  Serial.println(azimut);
  servo_azimut.write(azimut);  // Desplazamos a la posición deseada
  ang_azimut_act = azimut;
  delay(1000);
}
else{
  Serial.println("El valor de azimut no es correcto. Debe ser entre 0 y 180º");
  Errores=1;
} 
if (limite_min_elevacion <= elevacion and elevacion <= limite_max_elevacion){
  Serial.println("Elevación:");
  Serial.println(elevacion);
  servo_elevacion.write(elevacion);  // Desplazamos a la posición deseada
  ang_elevacion_act = elevacion;
  delay(1000);
}    
else{
  Serial.println("El valor de elevacion no es correcto. Debe ser entre 0 y 90º");
  Errores=2;
}
}