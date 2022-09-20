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
  #define Azimut_Led 2  // Indicador Servo 1
  #define Elevacion_Led 3 // Indicador Servo 2
// Servos
  #define Azimut_pin 4  // Servo azimut
  #define Elevacion_pin 5  // Servo elevación
// Declaramos la variable para controlar el servo
  Servo servo_azimut;
  Servo servo_elevacion;
//////// VAriables de json //////////////
// Estado
  int num_Lab=2;
  bool subLab=0;
  bool iniLab=0;
// Analogico
  int Azimut_recibido=0;
  int Elevacion_recibido=0;
//--- Variables auxiliares ---//
  int Azimut_ang_actual=90; //Angulos actuales de azimut
  int Elevacion_ang_actual; //Angulos actuales de azimut
  bool bandera_rep=0; // bandera para limitar la cantidad de repeticiones de mensaje de lab incorrecto 

// Constantes auxiliares
  //Azimut entre (-30º y +30º)
  const int limite_min_azimut=0; // Indica el minimo valor que puede tomar distancia
  const int limite_max_azimut=180; // Indica el minimo valor que puede tomar distancia
  //Elevación entre (-30º y +30º)
  const int limite_min_elevacion=0; // indica el minimo valor que puede tomar distancia.
  const int limite_max_elevacion=180; // indica el minimo valor que puede tomar distancia.

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
  pinMode(Azimut_Led, OUTPUT); // Led Servo 1
  pinMode(Elevacion_Led, OUTPUT); // Led Servo 2
// Servos
  servo_azimut.attach(Azimut_pin);
  servo_elevacion.attach(Elevacion_pin);
//------ Definir estados iniciales ------//
  digitalWrite(Azimut_Led,LOW);
  digitalWrite(Elevacion_Led,LOW);
  // Pongo los servos en posición inicial.
  mover_servo(servo_azimut,90);
  mover_servo(servo_elevacion,90);
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
      //------ GET ----- //
      if (strstr(Mensaje_recibido, "GET /HTTP/1.1") != NULL) { // Compruebo si llega un GET, respondo valores actuales
        StaticJsonDocument<256> doc;     
        JsonArray Estado = doc.createNestedArray("Estado");
          Estado.add(num_Lab);
          Estado.add(subLab);
          Estado.add(iniLab);
        JsonArray Analogicos = doc.createNestedArray("Analogicos");
          Analogicos.add(Azimut_ang_actual);
          Analogicos.add(Elevacion_ang_actual);
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
          if(subLab==1){
          Serial.println("Sub - Laboratorio: Wifi 2.4 GHz"); 
          JsonArray Analogico = doc["Analogico"];
          Azimut_recibido = Analogico[0];
          Elevacion_recibido = Analogico[1];
          }
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
    if (subLab and iniLab){ wifi_24_ghz(Azimut_recibido, Elevacion_recibido);}
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
  //Control de limites.
  if ((limite_min_azimut <= azimut) and (azimut <= limite_max_azimut)){
    Azimut_ang_actual = mover_servo(servo_azimut,azimut);// Desplazamos a la posición deseada    
  } else{ Errores=1;} // Error 1 : El valor de azimut no es correcto. Debe ser entre 0 y 180º
  if((limite_min_elevacion<=elevacion) and (elevacion <= limite_max_azimut)){
    Elevacion_ang_actual = mover_servo(servo_elevacion,elevacion);
  } else{ Errores=1;} // Error 1 : El valor de elevacion no es correcto. Debe ser entre 0 y 180º
  delay(20);
}

/**
 * @brief Funcion utilizada para mover el servo de forma lenta
 * 
 * @param servo_sel Es el servo seleccionado.
 * @param ang_sel Es el angulo requerido a mover.
*/
int mover_servo(Servo servo_sel,int ang_sel){
  int ang_actual = servo_sel.read(); // angulo actual 
  if(ang_actual < ang_sel){ ang_actual = ang_actual +1; servo_sel.write(ang_actual); }
  if(ang_actual > ang_sel){ ang_actual = ang_actual -1; servo_sel.write(ang_actual); }
  delay(10);
  return ang_actual;
}