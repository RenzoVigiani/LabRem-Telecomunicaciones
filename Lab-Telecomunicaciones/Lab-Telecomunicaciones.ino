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
  const int Numero_laboratorio=2;
// Nombres para los pines GPIO
// Leds
  #define Led_aux1 10 // Indicador aux 1
  #define Led_aux2 11 // Indicador aux 2
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
  int num_Lab=2;
  bool subLab=0;
  bool iniLab=0;
// Analogico
  int variable_0=0;
  int variable_1=0;
//--- Variables auxiliares ---//
  int ang_azimut_act; //Angulos actuales de azimut
  int ang_elevacion_act; //Angulos actuales de azimut
  bool bandera_rep=0; // bandera para limitar la cantidad de repeticiones de mensaje de lab incorrecto 

// Constantes auxiliares
  //Azimut entre (-30º y +30º)
  const int limite_min_azimut=-60; // Indica el minimo valor que puede tomar distancia
  const int limite_max_azimut=60; // Indica el minimo valor que puede tomar distancia
  //Elevación entre (-30º y +30º)
  const int limite_min_elevacion=-30; // indica el minimo valor que puede tomar distancia.
  const int limite_max_elevacion=30; // indica el minimo valor que puede tomar distancia.

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
  pinMode(Led_aux1, OUTPUT); // Led Aux 1
  pinMode(Led_aux2, OUTPUT); // Led Aux 2
  pinMode(Led_S1, OUTPUT); // Led Servo 1
  pinMode(Led_S2, OUTPUT); // Led Servo 2
// Servos
  servo_azimut.attach(azimut_pin);
  servo_elevacion.attach(elevacion_pin);
//------ Definir estados iniciales ------//
  digitalWrite(Led_aux1,LOW);
  digitalWrite(Led_aux2,LOW);
  digitalWrite(Led_S1,LOW);
  digitalWrite(Led_S2,LOW);
}

void loop(){
//////////// Strings de comunicación /////////////
char Mensaje_recibido[const_mje] = {0}; // Mensaje recibido por ethernet. (Comando + JSON) 
char valores_recibidos[const_valores] = {0}; // JSON recibido.  // Wait for an incomming connection
EthernetClient client = server.available(); 
  if(client){ // Si tengo un cliente conectado
    while (client.available()){ 
      client.readBytesUntil('\r', Mensaje_recibido, sizeof(Mensaje_recibido)); // Tomo el mensaje recibido.
      strncpy(valores_recibidos,&Mensaje_recibido[15],(sizeof(Mensaje_recibido)-15)); 
      Serial.print("Mensaje Recibido: " + (String)Mensaje_recibido);
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
        doc["Error"] = Errores;

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
      }
      //------- POST -----//      
      if (strstr(Mensaje_recibido, "POST /HTTP/1.1") !=NULL) { // Compruebo si llega un POST y respondo, habilito banderas.
        Errores=0;
        if(bandera_rep==1)bandera_rep=0;//reinicio bandera de repetición cuando tengo un mje nuevo.
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
  if(num_Lab==2 and bandera_rep==0){
    if (subLab and iniLab){ // 2.4 GHz
      Serial.println("Sub - Laboratorio: Wifi 2.4 GHz"); 
      wifi_24_ghz(variable_0, variable_1);
    }
    else{
      if(bandera_rep==0){// Laboratorio parado.
        Serial.println("Laboratorio Parado");
        bandera_rep=1;
      }
    }
  }
  else{
    if(bandera_rep==0){//Laboratorio incorrecto 
      Serial.println("Laboratorio incorrecto");  
      bandera_rep = 1; 
      Errores=3;
    }
  }
}

/**
 * @brief Funcion utilizada para realizar el movimiento de la antena de a cuerdo al angulo solicitado.
 * 
 * @param azimut - Angulo de inclinación Azimut 
 * @param elevacion - Angulo de inclinación Elevación
 */
void wifi_24_ghz(int azimut, int elevacion){
  if (limite_min_azimut <= azimut and azimut <= limite_max_azimut){
    // Serial.println("Azimut:");
    // Serial.println(azimut);
    if(ang_azimut_act<azimut){
      ang_azimut_act++;
      servo_azimut.write(ang_azimut_act);  // Desplazamos a la posición deseada    
    }
    if(ang_azimut_act>azimut){
      ang_azimut_act=ang_azimut_act--;
      servo_azimut.write(ang_azimut_act);  // Desplazamos a la posición deseada    
    }
    if(ang_azimut_act==azimut){
      servo_azimut.write(azimut);  // Desplazamos a la posición deseada
    }
  }
  else{ // Error 1
    Serial.println("El valor de azimut no es correcto. Debe ser entre 0 y 180º");
    Errores=1;
  } 
  if (limite_min_elevacion <= elevacion and elevacion <= limite_max_elevacion){ // Modifico angulo de elevación
    // Serial.println("Elevación:");
    // Serial.println(elevacion);
    if(ang_elevacion_act<elevacion){
      ang_elevacion_act++;
      servo_elevacion.write(ang_elevacion_act);  // Desplazamos a la posición deseada    
    }
    if(ang_elevacion_act>elevacion){
      ang_elevacion_act=ang_elevacion_act--;
      servo_elevacion.write(ang_elevacion_act);  // Desplazamos a la posición deseada    
    }
    if(ang_elevacion_act==elevacion){
      servo_elevacion.write(elevacion);  // Desplazamos a la posición deseada
    }
  }    
  else{ // Error 2
    Serial.println("El valor de elevacion no es correcto. Debe ser entre 0 y 90º");
    Errores=2;
  }
  delay(20);
}