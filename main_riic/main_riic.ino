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
/*-----------------------------------------------------------------------------
 ------------ P R O T O T I P O S   D E   F U N C I O N E S -------------------
 -----------------------------------------------------------------------------*/
void antirrebotes_niveles(void);
void estado_sensores(void);
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
const char* ssid = "Casa Bonilla";              //Enter your SSID here
const char* password = "losbonilla2021";        //Enter your Password here
WebServer server(80);                           //HTTP port, 80 is defult)

unsigned char nivel1, nivel2, nivel3, niveles;   //variables para sensor Prox
unsigned char antirrebote1, antirrebote2, antirrebote3; //antirrebotes
unsigned char aire;                              //variables para aire
int wenas;
float temp;                                      //variable de temperatura
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
  WiFi.begin(ssid, password);   //se inicia la conexion wifi
  // Check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());  //Show ESP32 IP on serial

  server.on("/", handle_OnConnect); // Directamente desde e.g. 192.168.0.8
  server.onNotFound(handle_NotFound);

  server.begin();
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
  niveles=nivel1+nivel2+nivel3;       
  //-------se inicia el servidor web
  server.handleClient();
  //-------antirrebotes de cambios en niveles de basura
  antirrebotes_niveles();
  //-------actualizacion de sensores
  estado_sensores();
  //-------ejecucion de pequeño servidor
  server.send(200, "text/html", SendHTML(aire,temp,niveles));
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
    pixels.setPixelColor(wenas,pixels.Color(255, 255, 255)); //rojo
    pixels.show();
  }
 }

//-------HANDLER DE INICIO DE PAGINA
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(aire,temp, niveles));
}

//-------funcion para ejecucion de pequeño servidor web
String SendHTML(unsigned char air, float tempt, unsigned char level) {
  String ptr = "<!DOCTYPE html> <html>\n";

  ptr += "  <head>\n";
  ptr += "    <meta chatset=\"utf_8\">\n";
  ptr += "    <title> YAIMA, RIIC 4.0 </title>\n";
  ptr += "    <script>\n";
  ptr += "  <!--\n";
  ptr += "  function timedRefresh(timeoutPeriod) {\n";
  ptr += "  \tsetTimeout(\"location.reload(true);\",timeoutPeriod);\n";
  ptr += "  }\n";
  ptr += "\n";
  ptr += "  window.onload = timedRefresh(15000);\n";
  ptr += "\n";
  ptr += "  //   -->\n";
  ptr += "  </script>\n";
  ptr += "      <h1 align='center'>Proyecto Yaima</h1>\n";
  ptr += "      <h2 align='center'> C2A2E </h2></th>\n";
  ptr += "      <h2 align='center'> Cyntia Matus, Crithofer Patzán, Alvaro Sosa, Andy Bonilla, Emmanuel Sandoval</h2></th>\n";
  //-------prueba de aire
  if (aire>0){
    ptr += "<h2 align='center'>hay aire xd</h2>";
  }
  else{
    ptr += "<h2 align='center'>no hay aire xd</h2>";
  }
  //-------prueba de temperatura
  if (temp>0){
    ptr += "<h2 align='center'>hay temperatura :o </h2>";
  }
  else {
    ptr += "<h2 align='center'>no hay temperatura :o </h2>";
  }
  //-------prueba de niveles
  switch(niveles){
    case 1: ptr += "<h2 align='center'> nivel 1 </h2>"; break;
    case 2: ptr += "<h2 align='center'> nivel 2 </h2>"; break;
    case 3: ptr += "<h2 align='center'> nivel 3 </h2>"; break;
    default: ptr += "<h2 align='center'> no ta conectado :( </h2>"; break;
  }
  /*switch(wenas){
    case 0:
      ptr += "<h2 align='center'>Cantidad de Parqueos disponibles: 0</h2>";
      break;
    case 1:
      ptr += "<h2 align='center'>Cantidad de Parqueos disponibles: 1</h2>";
      break;
    case 2:
      ptr += "<h2 align='center'>Cantidad de Parqueos disponibles: 2</h2>";
      break;
    case 3:
      ptr += "<h2 align='center'>Cantidad de Parqueos disponibles: 3</h2>";
      break;
    case 4:
      ptr += "<h2 align='center'>Cantidad de Parqueos disponibles: 4</h2>";
      break;
    case 5:
      ptr += "<h2 align='center'>Cantidad de Parqueos disponibles: 5</h2>";
      break;
    case 6:
      ptr += "<h2 align='center'>Cantidad de Parqueos disponibles: 6</h2>";
      break;
    case 7:
      ptr += "<h2 align='center'>Cantidad de Parqueos disponibles: 7</h2>";
      break;
    case 8:
      ptr += "<h2 align='center'>Cantidad de Parqueos disponibles: 8</h2>";
      break;
  }*/
  ptr += "<img src='https://cdn.pixabay.com/photo/2016/12/31/01/43/auto-1941988_960_720.png'>";
  ptr += "  </table>\n";
  ptr += "  </html>\n";
  ptr += "";
  return ptr;
}
//-------HANDLER DE NOT FOUND
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}
