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

//const char* server = "18.212.146.249";

/* This sample code demonstrates the normal use of a TinyGPS object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/

ClosedCube_HDC1080 sensor;
double i;
double sumTemp;
double sumHum;
int countGPS;
int countH;
int countT;

TinyGPS gps;
SoftwareSerial ss(2, 0);

//Datos a enviar
double latitud;
double longitud;
double temperaturaEnv;
double temperaturaSum;
double humedadEnv;
double humedadSum;

int id = 1001;

WiFiClient client;

//Vbles lectura gps
static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

/*static void sendthingspeak()
{

  String PostData = "";

  Serial.println("datos para enviar");

  PostData = String("{\"id\":"+String(id)+", \"temperatura\":"+String(temperatura,7)+", \"longitud\":"+String(longitud,7)+", \"latitud\":"+String(latitud,7)+"}");

  Serial.println(PostData);

  if ( client.connect(server,80))

  {

    Serial.println("conectado");

    client.print("POST /sensor HTTP/1.1\n");

    // poner la direccion IP del servidor ​

    client.print("Host: 18.212.146.249 \n"); //el de ander

    //client.println("User-Agent: Arduino/1.0");

    //client.println("Connection: close");

    client.println("Content-Type: application/json;"); //Cambiar a JSON

    client.print("Content-Length: ");

    client.println(PostData.length());

    client.println();

    client.println(PostData);

  } else {

    Serial.println("error de conexion");

  }

}
*/
void setup()
{ 
  Serial.begin(115200);
  ss.begin(9600);
  sensor.begin(0x40);

  delay(10);

  Serial.println();

  Serial.println();

  Serial.print("Connecting to ");

  Serial.println(ssid);

//WiFi.begin(ssid, password);

/*while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

  }*/

 Serial.println("");

 Serial.println("WiFi connected");

  //vbles temperatura pruning
  i = 0.0;
  sumTemp = 0.0;
  sumHum = 0.0;
  countGPS = 0;
  countH = 0;
  countT = 0;
  temperaturaSum = 0;
  temperaturaEnv = 0;
  humedadEnv = 0;
  humedadSum = 0;
}

void loop()
{
  countGPS++;
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;


  if(countGPS > 100){ //Se envia GPS
    gps.f_get_position(&flat, &flon, &age);
    //Serial.println("\nLATITUD:");
    //print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
    latitud = flat;
    //Serial.println("\nLONGITUD:");
    //print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
    longitud = flon;
    //Serial.println("\nFECHA:");
    //print_date(gps);
    //Serial.println("\nALTITUD:");
    //print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
    //Serial.println();
    //Serial.println();
    //Serial.println();

    //Envío Paquete. Se envían los datos vigentes de temperatura y humedad.

    Serial.println("Longitud");
    Serial.println(longitud);
    Serial.println("Latitud");
    Serial.println(latitud);
    print_date(gps);
    Serial.println();
    Serial.println();
    Serial.println();

    //Obtención de datos

    temperaturaSum += sensor.readTemperature();
    humedadSum += sensor.readHumidity();
    countT++;
    countGPS = 0.0;

  }
  
  if(countT == 3){ //30 segundos, se envía H, T y GPS.



    /*Serial.println("\nHUMEDAD A ENVIAR:\n");
    Serial.println(sensor.readHumidity());
    Serial.println("\nTEMPERATURA:\n");
    Serial.println();
    temperaturaEnv = sensor.readTemperature();

    countT = 0;
    countH = 0;
    countGPS = 0;
    //sendthingspeak();
    */

  //Prunning:
  temperaturaEnv = temperaturaSum/3.0;
  humedadEnv = humedadSum/3.0;

  Serial.println("\nHumedad:");
  Serial.println(humedadEnv);
  Serial.println("Temperatura:");
  Serial.println(temperaturaEnv);

  countT = 0;
  
  //Se actualizan los datos de temperatura y humedad.


  }


  smartdelay(100);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartdelay(0);
}

