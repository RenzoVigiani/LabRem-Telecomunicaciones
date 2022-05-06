#include <ArduinoJson.h>
#include <UIPEthernet.h>
#include <Servo.h>

#define const_status 150
#define const_instruc 130

EthernetServer server = EthernetServer(22);
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

////////////// Funciones  ////////////////////
void wifi_24_ghz(int azimut, int elevacion);
void rad_def_soft(bool iniLab, int intensidad_max, int intensidad_min, int modulacion, int codificacion);
//-------------------//
void get_json(EthernetClient client);
void post_json(char instrucciones[const_instruc], EthernetClient client);

// Declaramos la variable para controlar el servo
Servo servo_azimut;
Servo servo_elevacion;

void setup() 
{
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

// Iniciamos el pin de cada servo
  servo_azimut.attach(9);
  servo_elevacion.attach(3);
  }

void loop() 
{

//////////// Strings de comunicación /////////////
char status[const_status] = {0};
char instrucciones[const_instruc] = {0};

  // Wait for an incomming connection
  EthernetClient client = server.available(); 
  // Do we have a client?
  if (!client) return;
  Serial.println();
  Serial.println(F("New client"));
  // Read the request (we ignore the content in this example)
  while (client.available()) 
  {
    client.readBytesUntil('\r', status, sizeof(status));
    Serial.println("status:");
    Serial.println(status);
//obtengo las instrucciones del formato json
    strncpy(instrucciones,&status[15],(sizeof(status)-15));
  //------ GET ----- //
    if (strstr(status, "GET / HTTP/1.1") != NULL) 
    {
      get_json(client);
    }
  //------- POST -----//      
    if (strstr(status, "POST / HTTP/1.1") !=NULL) 
    {
      post_json(instrucciones, client);
    }
  }
}

void get_json(EthernetClient client)
{
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
// client.stop();  
}

void post_json(char instrucciones[const_instruc], EthernetClient client)
{
    Serial.println("Solicitud de escritura recibida");
    client.println(F("HTTP/1.1 200 OK"));
    client.println();
    StaticJsonDocument<128> doc;
    // Deserializo
    DeserializationError error = deserializeJson(doc, instrucciones);
    
    if (error) 
    {
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

  if(num_Lab==2)
  {
    if (subLab and iniLab)
    {
      Serial.println("Sub - Laboratorio: Wifi 2.4 GHz"); 
      wifi_24_ghz(variable_0, variable_1);
    }
    else if (!subLab and iniLab)
    {
      Serial.println("Sub - Laboratorio: Radio definida por Software");  
      rad_def_soft(variable_0, variable_1, variable_2, variable_3);
    }
  }
  else
  {
    Serial.println("Laboratorio incorrecto");    
  }
//  get_json(client);
}

void wifi_24_ghz(int azimut, int elevacion){
    if (0 <= azimut and azimut <= 180) 
    {
      Serial.println("Azimut:");
      Serial.println(azimut);
      servo_azimut.write(azimut);  // Desplazamos a la posición deseada
      delay(1000);
    }
    else
      Serial.println("El valor de azimut no es correcto. Debe ser entre 0 y 180º");
    if (0 <= elevacion and elevacion <= 90) 
    {
      Serial.println("Elevación:");
      Serial.println(elevacion);
      servo_elevacion.write(elevacion);  // Desplazamos a la posición deseada
      delay(1000);
    }    
    else
      Serial.println("El valor de elevacion no es correcto. Debe ser entre 0 y 90º");
}

void rad_def_soft(int intensidad_max, int intensidad_min, int modulacion, int codificacion){
/*  if(iniLab){
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
  }*/
}
