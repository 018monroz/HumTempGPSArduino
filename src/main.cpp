#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <TinyGPS.h>
#include "ClosedCube_HDC1080.h"
#include <ESP8266WebServer.h>
#include <DNSServer.h>

const char* ssid = "UPBWiFi";

const char* password = "";

//poner la direccion IP del servidor

const char* server = "54.225.72.244";


ClosedCube_HDC1080 sensor;



//Conteo de iteraciones
int countGPS;
int countT;

//Objeto GPS
TinyGPS gps;
SoftwareSerial ss(2, 0);

//Datos para podado y envío
double latitud;
double longitud;
double temperaturaEnv;
double temperaturaSum;
double humedadEnv;
double humedadSum;


WiFiClient client;

//Vbles lectura gps
static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);

//Envio de datos
static void EnvioHumedadyTemperatura() 
{

  String PostData = "";

  Serial.println("datos para enviar");

  //Formato en JSON por petición del servidor
  PostData = String("{\"id\":\"000420108\", \"temperatura\":\""+String(temperaturaEnv,7)+"\", \"humedad\":\""+String(humedadEnv,7)+"\"}");

  Serial.println(PostData);

  if (client.connect(server,80))

  {

    Serial.println("conectado");

    //Trama HTTP
    client.print("POST /datos HTTP/1.1\n");

    client.print("Host:  54.225.72.244 \n"); //Serv destino

    client.println("User-Agent: Arduino/1.0");

    client.println("Connection: close");

    client.println("Content-Type: application/json;"); //Formato

    client.print("Content-Length: ");

    client.println(PostData.length());

    client.println();

    client.println(PostData);

  } else {

    Serial.println("error de conexion");

  }

}

static void EnvioGPS()
{

  String PostData = "";

  Serial.println("datos para enviar");

PostData = String("{\"id\":\"000420108\", \"latitud\":\""+String(latitud,7)+"\", \"longitud\":\""+String(longitud,7)+"\"}");

  Serial.println(PostData);

  if (client.connect(server,80))

  {

    Serial.println("conectado");

    client.print("POST /datos HTTP/1.1\n");

    // poner la direccion IP del servidor ​

    client.print("Host:  54.225.72.244 \n"); //Serv destino

    client.println("User-Agent: Arduino/1.0");

    client.println("Connection: close");

    client.println("Content-Type: application/json;"); //Formato

    client.print("Content-Length: ");

    client.println(PostData.length());

    client.println();

    client.println(PostData);

  } else {

    Serial.println("error de conexion");

  }
}


void setup()
{ 
  //Arranque dispositivos
  Serial.begin(115200);
  ss.begin(9600);
  sensor.begin(0x40);

  delay(10);
  Serial.println();
  Serial.println();

  Serial.print("Connecting to ");

  Serial.println(ssid);
  
  //Conexión a Red
WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

  }

 Serial.println("");

 Serial.println("WiFi connected");

  //Inicialización de variables
  countGPS = 0;
  countT = 0;
  temperaturaSum = 0;
  temperaturaEnv = 0;
  humedadEnv = 0;
  humedadSum = 0;
}

void loop()
{
  countGPS++; //Cada count significa un lapso de 100ms
  float flat, flon;  //Para la captura de gps desde la librería
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;


  if(countGPS > 100){ //Pasaron 100x100ms. Se envia GPS (10 segundos)

    gps.f_get_position(&flat, &flon, &age);
    latitud = flat;
    longitud = flon;


    //Envío Paquete.
    EnvioGPS();

    //Obtención de datos de humedad y temperatura

    temperaturaSum += sensor.readTemperature();
    humedadSum += sensor.readHumidity();
    countT++;
    countGPS = 0.0;

  }
  
  if(countT == 3){ //30 segundos, se envía H, T

  //Prunning: Se promedian los datos para enviar los correctos.
  temperaturaEnv = temperaturaSum/3.0;
  humedadEnv = humedadSum/3.0;

  EnvioHumedadyTemperatura();

  countT = 0;
  
  //Se actualizan los datos de temperatura y humedad.
  temperaturaSum = 0;
  humedadSum = 0;

  }


  smartdelay(100);
}

//Delay en el que se puede capturar GPS
static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

