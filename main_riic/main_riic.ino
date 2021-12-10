/*------------------------------------------------------------------------------  
Archivo: prototipo_RIIC
Microcontrolador: ATmega328P
Autor: C2A2E
Compilador: Arduino IDE
Programa: prototipo_RIIC
Hardware: ESP32 8266
    
Creado: 15 de octubre de 2021    
Descripcion: lectura del ADC para sensor de fuerza y se despliegara en un 7seg 
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 ----------------------------L I B R E R I A S---------------------------------
 -----------------------------------------------------------------------------*/
#include <WiFi.h>                 //libreria para conexion a wifi
#include <WebServer.h>            //libreria para peuqeño servidor web
#include <Arduino.h>              //libreria de arduino
#include <Wire.h>                 //libreria de comunicacion i2c
#include <Adafruit_SHT31.h>       //libreria de sensor de temperatura
#include <Adafruit_NeoPixel.h>    //libreria de rueda rgb
#include <SPIFFS.h>
/*-----------------------------------------------------------------------------
 ------------ P R O T O T I P O S   D E   F U N C I O N E S -------------------
 -----------------------------------------------------------------------------*/
void antirrebotes_niveles(void);
void estado_sensores(void);
bool HandleFileRead(String);
 /*-----------------------------------------------------------------------------
 -----------------V A R I A B L E S   A   I M P L E M T E N T A R--------------
 -----------------------------------------------------------------------------*/
//-------DIRECTIVAS DE COMPILADOR
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define PIN        13                           //pin que se hara toggle de color 
#define NUMPIXELS 16                            //numero de leds en aro de luz
#define DELAYVAL 500 //
//-------VARIABLES GLOBALES DEL PROGRAMA
const char* ssid = "Ejemplo RIIC4";              //Enter your SSID here
const char* password = "123456789";        //Enter your Password here
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);                           //HTTP port, 80 is defult)

byte nivel1, nivel2, nivel3, niveles;   //variables para sensor Prox
byte antirrebote1, antirrebote2, antirrebote3; //antirrebotes
byte aire;                              //variables para aire
byte wenas;
float temp,humedad;                                      //variable de temperatura
bool enableHeater = false;                       //inicializador de temp
uint8_t loopCnt = 0;

Adafruit_SHT31 sht31 = Adafruit_SHT31();  
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);  
/*-----------------------------------------------------------------------------
 --------------------- I N T E R R U P C I O N E S ----------------------------
 -----------------------------------------------------------------------------*/  
//-------interrupcion para cambio de nivel leve
void ISR_n1(){
  antirrebote1=1;
}
//-------interrupcion para cambio de nivel medio
void ISR_n2(){
  antirrebote2=1;
}
//-------interrupcion para cambio de nivel alto
void ISR_n3(){
  antirrebote3=1;
}
/*-----------------------------------------------------------------------------
 ------------------------------ S E T   U P -----------------------------------
 -----------------------------------------------------------------------------*/
void setup()
{
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  //-------CONFIGURACION DE ENTRADAS Y SALIDAS
  //si ninguno se activa, es porque esta vacio el contenedor
  pinMode(19, INPUT_PULLUP);        //nivel leve
  pinMode(18, INPUT_PULLUP);        //nivel medio
  pinMode(5, INPUT_PULLUP);         //nivel alto
  pinMode(36, INPUT);               //entrada para sensor de co2
  pinMode(12, OUTPUT);              //salida del buzzer
  
  //-------CONFIGURACION DE INTERRUPCIONES
  attachInterrupt(digitalPinToInterrupt(19), ISR_n1, FALLING);      //nivel leve
  attachInterrupt(digitalPinToInterrupt(18), ISR_n2, FALLING);      //nivel medio
  attachInterrupt(digitalPinToInterrupt(5), ISR_n3, FALLING);      //nivel alto
  Serial.begin(9600);
  //-------INICIALIZACION DE SERVIDOR WIFI
  Serial.begin(115200);

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("password: ");
  Serial.println(password);

  server.on("/", handle_OnConnect); // página de inicio
  server.onNotFound([]() {                  // si el cliente solicita una uri desconocida
    if (!HandleFileRead(server.uri()))      // enviar archivo desde SPIFF, si existe
      handleNotFound();             // sino responder con un error 404 (no existe)
  });
  //server.onNotFound(handle_NotFound); //si no se encuentra el uri, responder con error 404 (no existe)

  server.begin(); // iniciar servidor
  Serial.println("HTTP server started");
  delay(100);
  //-------INICIALIZACION DE SENSOR 12C
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
    delay(100);
  //-------INCIALIZACION DE ARO DE LUZ
  pixels.begin();  
  pixels.clear();
}
/*-----------------------------------------------------------------------------
 -------------------------- M A I N   L O O P ---------------------------------
 -----------------------------------------------------------------------------*/
void loop()
{
  //-------inicializacion de lecturas
  aire = analogRead(36);              //sensor de co2
  temp = sht31.readTemperature();     //sensor de temperatura
  humedad = sht31.readHumidity();     //sensor de humedad
  niveles=nivel1+nivel2+nivel3;       
  //-------se inicia el servidor web
  server.handleClient();
  //-------antirrebotes de cambios en niveles de basura
  antirrebotes_niveles();
  //-------actualizacion de sensores
  estado_sensores();
  //-------ejecucion de pequeño servidor
  //server.send(200, "text/html", SendHTML(aire,temp,niveles));
  server.send(200, "text/html", SendHTML(aire,temp,niveles,humedad)); //responde con un OK (200) y envía HTML
  wenas++;
  if (wenas>15){
    wenas=0;
  }
  
  
}
/*-----------------------------------------------------------------------------
 ------------------------- F U N C I O N E S ----------------------------------
 -----------------------------------------------------------------------------*/
//-------FUNCION PARA ANTIRREBOTES DE SENSORES DE PROXIMIDAD
void antirrebotes_niveles(void){
  //-------antirrebote para nivel 1, leve
  if (digitalRead(19)==0 && antirrebote1==1){   
    nivel1=1;       //nivel leve leve on
  }
  else{                                           
    nivel1=0;       //nivel leve leve off
  }
  //-------antirrebote para nivel 2, medio
  if (digitalRead(18)==0 && antirrebote2==1){   
    nivel2=1;       //nivel medio on
  }
  else{                                           
    nivel2=0;       //nivel medio off
  }
  //-------antirrebote para parqueo 3
  if (digitalRead(5)==0 && antirrebote3==1){  
    nivel3=1;       //nivel alto on
  }
  else{                                           
    nivel3=0;         //nivel alto off
  }
}
//-------FUNCION DE ACTUALIZACION DE SENSORES
void estado_sensores(void){
  //-------actualizacion nivel 1
  if(nivel1==1 && nivel2==0 && nivel3==0){
    Serial.print(aire);
    Serial.print(" | ");
    Serial.print("bajo nivel de basura");
    Serial.print(" | ");
    Serial.println(temp);
    pixels.setPixelColor(wenas,pixels.Color(0, 150, 0));  //verde
    pixels.show();
  }
  
  //-------actualizacion nivel 2
  else if(nivel1==1 && nivel2==1 && nivel3==0){
    Serial.print(aire);
    Serial.print(" | ");
    Serial.print("nivel medio de basura");
    Serial.print(" | ");
    Serial.println(temp);
    pixels.setPixelColor(wenas,pixels.Color(255, 233, 0)); //amarillo
    pixels.show();
  }
  //-------actualizacion nivel 3
  else if(nivel1==1 && nivel2==1 && nivel3==1){
    Serial.print(aire);
    Serial.print(" | ");
    Serial.print("nivel alto de basura");
    Serial.print(" | ");
    Serial.println(temp);
    pixels.setPixelColor(wenas,pixels.Color(255, 0, 0)); //rojo
    pixels.show();
    //digitalWrite(12,HIGH);
  }
  else{
    Serial.print(aire);
    Serial.print(" | ");
    Serial.print("no hay basura");
    Serial.print(" | ");
    Serial.println(temp);
    pixels.setPixelColor(wenas,pixels.Color(0 , 0, 0));
    pixels.show();
  }
 }

//-------HANDLER DE INICIO DE PAGINA
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(aire,temp, niveles,humedad));
}

//-------funcion para ejecucion de pequeño servidor web
String SendHTML(byte aire, float tempt, byte level, float humedad) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += " <html>\n";
  ptr += " <head>\n";
  ptr += " <meta charset=\"utf-8\">\n";
  ptr += " <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  ptr += " <meta http-equiv=\"refresh\" content=\"20\" />";
  ptr += " <title> YAIMA, RIIC 4.0 val </title>\n";
  ptr += " <link rel=\"stylesheet\" href=\"index.css\">\n";
  ptr += " </head>\n";

  ptr += " <body onload=\"gettime()\"><center>";
  ptr += "     <h1>YAIMA PROJECT</h1>";
  ptr += " <table style=\"margin-top: 5%;\"><tr>";
  ptr += "     <td>";
  ptr += "         <div class=\"content-all\">";
  ptr += " <div class=\"content-img\">";

  ptr += "             <div class=\"content-txt\">";
  ptr += "                 <h2> Humedad <br> ";
  ptr += String(humedad);
  ptr += "% </h2>";
  ptr += "            </div>";
  ptr += "             <div class=\"content-1\"></div>";
  ptr += "             <div class=\"content-2\"></div>";
  ptr += "             <div class=\"content-3\"></div>";
  ptr += "             <div class=\"content-4\"></div>";
  ptr += "             <img src=\"humedad.jpg \">";
  ptr += "         </div>";
  ptr += " </div";
  ptr += "     </td>";
  ptr += " <td> <div class=\"content-all\">";
  ptr += " <div class=\"content-img\">";

  ptr += "             <div class=\"content-txt\">";
  ptr += "                 <h2> Temperatura <br>";
  ptr += String(tempt);
  ptr += "°</h2>";
  ptr += "             </div>";
  ptr += "             <div class=\"content-1\"></div>";
  ptr += "             <div class=\"content-2\"></div>";
  ptr += "             <div class=\"content-3\"></div>";
  ptr += "             <div class=\"content-4\"></div>";
  ptr += "             <img src=\"temperatura.jpg \">";
  ptr += "         </div> ";
  ptr += "        </td>";
  ptr += "        <td> <div class=\"content-all\">";
  ptr += " <div class=\"content-img\">";

  ptr += "             <div class=\"content-txt\">";
  ptr += "                 <h2> Nivel de CO<sub>2</sub> <br> ";
  ptr += String(aire);
  ptr += "</h2>";

  ptr += "             </div>";

  ptr += "             <div class=\"content-1\"></div>";
  ptr += "             <div class=\"content-2\"></div>";
  ptr += "             <div class=\"content-3\"></div>";
  ptr += "             <div class=\"content-4\"></div>";
  ptr += "             <img src=\"co4.jpg \">";

  ptr += "         </div> </td> ";
  ptr += " </tr></table>";
  ptr += " <table>";
  ptr += " <tr>";
  ptr += " <td>";
  ptr += " <div class=\"content-all\">";
  ptr += " <div class=\"content-img\">";
  ptr += "             <div class=\"content-txt\">";
  ptr += "                 <h2> Mantenimiento<br> </h2>";
  ptr += "             </div>";
  ptr += "             <div class=\"content-1\"></div>";
  ptr += "             <div class=\"content-2\"></div>";
  ptr += "             <div class=\"content-3\"></div>";
  ptr += "             <div class=\"content-4\"></div>";
  ptr += "             <img src=\"settings.png \">";
  ptr += "         </div></td>";
  ptr += "         <td><div class=\"content-all\">";
  ptr += " <div class=\"content-img\">";
  ptr += "             <div class=\"content-txt\">";
  ptr += "                 <h2> Capacidad<br>";
  ptr += String(level);
  ptr += "</h2>";
  ptr += "             </div>";
  ptr += "             <div class=\"content-1\"></div>";
  ptr += "             <div class=\"content-2\"></div>";
  ptr += "             <div class=\"content-3\"></div>";
  ptr += "             <div class=\"content-4\"></div>";
  ptr += "             <img src=\"bat.jpg \">";
  ptr += " </div></td>";
  ptr += " </table>";
  ptr += " </center>";
  ptr += " <script language=\"JavaScript\">";
  ptr += " function gettime() {";
  ptr += "     var date= new Date();";
  ptr += "      var s = date.getSeconds();";
  ptr += "         if(s < 10){";
  ptr += "         s = \"0\" + s}";
  ptr += "     document.clockform.clock.value =  s;";
  ptr += "     setTimeout(\"gettime()\",100)}";
  ptr += " </script>";
  ptr += " <form name=\"clockform\">";
  ptr += " <input type=\"text\" name=\"clock\">";
  ptr += " </form>";
  ptr += " </body>";
  ptr += " </html>";
  
  return ptr;
}
//************************************************************************************************
// Handler de not found
//************************************************************************************************
void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}
//************************************************************************************************
// Obtener el tipo de contenido del archivo
//************************************************************************************************
String GetContentType(String filename)
{
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
//************************************************************************************************
// Enviar al servidor el archivo desde SPIFFS
//************************************************************************************************
void ServeFile(String path)
{
  File file = SPIFFS.open(path, "r");
  size_t sent = server.streamFile(file, GetContentType(path));
  file.close();
}
//************************************************************************************************
// Enviar al servidor el archivo desde SPIFFS
//************************************************************************************************
void ServeFile(String path, String contentType)
{
  File file = SPIFFS.open(path, "r");
  size_t sent = server.streamFile(file, contentType);
  file.close();
}
//************************************************************************************************
// Handler de not found
//************************************************************************************************
bool HandleFileRead(String path)
{
  if (path.endsWith("/")) path += "index.html";
  Serial.println("handleFileRead: " + path);

  if (SPIFFS.exists(path))
  {
    ServeFile(path);
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;
}
