#include <ArduinoJson.h>
#include <UIPEthernet.h>

EthernetServer server = EthernetServer(22);

////////////// Funciones  ////////////////////
void ControlPost(int num_Lab, bool subLab, bool iniLab, int variable_0, int variable_1, int variable_2, int variable_3);
void wifi_24_ghz(bool iniLab, int azimut, int elevacion);
void sdr(bool iniLab, int intensidad_max, int intensidad_min, int modulacion, int codificacion);
void valorSalidas(int);
void enciendoled(bool p0,bool p1,bool p2,bool p3);
void prueva_lab(int vueltas, bool Sentido);
void stopMotor(void);

//////////// declaración de salidas ///////////////////
const int Led_0=7;
const int Led_1=8;
const int Led_2=9;
const int Led_3=10;

/////////// salidas para el motor ////////////
#define IN1  3
#define IN2  4
#define IN3  5
#define IN4  6

//secuencia de vueltas
/*  
int paso [8][4] =
  {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
  };
  int vueltas=100;
  */

//  int dir=0;
/////////////////////////////////////
void setup() {
  uint8_t mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  IPAddress myIP(172,20,5,140);
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) continue;
  // Initialize Ethernet libary
  Ethernet.begin(mac,myIP);  
  // Start to listen
  server.begin();
  Serial.println(F("Server is ready."));
  Serial.print(F("Please connect to http://"));
  Serial.println(Ethernet.localIP());
  
  // declaro salidas para leds
  pinMode(Led_0, OUTPUT);
  pinMode(Led_1, OUTPUT);
  pinMode(Led_2, OUTPUT);
  pinMode(Led_3, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(Led_0,LOW);
  digitalWrite(Led_1,LOW);
  digitalWrite(Led_2,LOW);
  digitalWrite(Led_3,LOW);
  // declaro salidas para motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  }

void loop() {
//////// VAriables de json //////////////
// Estado
int num_Lab=0;
bool subLab=true;
bool iniLab=true;
// Analogico
int variable_0=0;
int variable_1=0;
int variable_2=0;
int variable_3=0;
////////////////////

//////////// Strings de comunicación /////////////
char status[150] = {0};
char instrucciones[130] = {0};
char operacion[20] = {0};

  // Wait for an incomming connection
  EthernetClient client = server.available(); 
  // Do we have a client?
  if (!client) return;
  Serial.println();
  Serial.println(F("New client"));
  // Read the request (we ignore the content in this example)
  while (client.available()) {
    client.readBytesUntil('\r', status, sizeof(status));
    Serial.println("status:");
    Serial.println(status);
  // Comparo la cadena recibida con las opcciones
    strncpy(operacion,status,15);
    Serial.println("operacion: ");
    Serial.println(operacion);
//obtengo las instrucciones del formato json
    strncpy(instrucciones,&status[15],(sizeof(status)-15));
    Serial.println("instrucciones:");
    Serial.println(instrucciones);   
    if (strstr(status, "GET / HTTP/1.1") != NULL) {
      
      StaticJsonDocument<128> doc;     
      JsonArray Estado = doc.createNestedArray("Estado");
      Estado.add(num_Lab);
      Estado.add(subLab);
      Estado.add(iniLab);

      JsonArray Analogico = doc.createNestedArray("Analogico");
      Analogico.add(variable_0);
      Analogico.add(variable_1);
      Analogico.add(variable_2);
      Analogico.add(variable_3);

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
 client.stop();

      }
///////////////////////////// POST ///////////////////////////////////
    if (strstr(status, "POST / HTTP/1.1") !=NULL) {
        Serial.println("Solicitud de escritura recibida");
        client.println(F("HTTP/1.1 200 OK"));
        client.println();
        StaticJsonDocument<128> doc;
        // Deserializo
        DeserializationError error = deserializeJson(doc, instrucciones);
        
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
        
        JsonArray Estado = doc["Estado"];
        num_Lab = Estado[0]; // 0 [Sist Dig], 1 [Sist Control], 2[Telecomunicaciones], 3[Fisica]
        subLab = Estado[1]; // true[SubLab 1], false [SubLab 2]
        iniLab = Estado[2]; // true[Inicia Experimento], false[Finaliza Experimento]

        JsonArray Analogico = doc["Analogico"];
        variable_0 = Analogico[0]; // 
        variable_1 = Analogico[1]; // 
        variable_2 = Analogico[2]; // 
        variable_3 = Analogico[3]; // 

        ControlPost(num_Lab ,subLab, iniLab, variable_0, variable_1,variable_2,variable_3);
    }
  }
}

void ControlPost(int num_Lab, bool subLab, bool iniLab, int variable_0, int variable_1, int variable_2, int variable_3){
  if(num_Lab== 2){
    Serial.println("Laboratorio: Telecomunicaciones");
    if(subLab){
      Serial.println("Sub - Laboratorio: Enlace Wifi 2.4GHz");
      wifi_24_ghz(iniLab, variable_0,variable_1);
    }
    else{
      Serial.println("Sub - Laboratorio: Enlace Radio definido por Software");
      sdr(iniLab, variable_0,variable_1,variable_2,variable_3);
    }
  }
  else{
      Serial.println("Error: Laboratorio Incorrecto");
  }
}

void wifi_24_ghz(bool iniLab, int azimut, int elevacion){
  if(iniLab){
    if (0 <= azimut <= 180) {
      digitalWrite(Led_0,true);
      Serial.println(azimut);
    }
    if (0 <= elevacion <= 90) {
      digitalWrite(Led_1,true);
      Serial.println(elevacion);
    }
  }
  else{
    Serial.println("Esperando para iniciar el Laboratorio...");
  }
}

void sdr(bool iniLab, int intensidad_max, int intensidad_min, int modulacion, int codificacion){
  if(iniLab){
    switch (modulacion)
    {
    case 1:
      Serial.println("Modulación: 4-QAM");
      break;
    case 2:
      Serial.println("Modulación: 8-QAM");
      break;
    case 3:
      Serial.println("Modulación: 16-QAM");
      break;
    case 4:
      Serial.println("Modulación: PSK");
      break;
    case 5:
      Serial.println("Modulación: FSK");
      break;
    case 6:
      Serial.println("Modulación: QPSK");
      break;          
    default:
      Serial.println("Modulación: PSK");
      break;
    }
  }
  else{
    Serial.println("Esperando para iniciar el Laboratorio...");
  }
}
