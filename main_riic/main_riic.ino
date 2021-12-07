/*------------------------------------------------------------------------------  
Archivo: prototipo_RIIC
Microcontrolador: ATmega328P
Autor: Cyntia Matus y Andy Bonilla
Compilador: Arduino IDE
Programa: prototipo_RIIC
Hardware: ESP32 8266
    
Creado: 15 de octubre de 2021    
Descripcion: lectura del ADC para sensor de fuerza y se despliegara en un 7seg 
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 ----------------------------L I B R E R I A S---------------------------------
 -----------------------------------------------------------------------------*/
#include <WiFi.h>
#include <WebServer.h>
/*-----------------------------------------------------------------------------
 ------------ P R O T O T I P O S   D E   F U N C I O N E S -------------------
 -----------------------------------------------------------------------------*/
void antirrebotes_niveles(void);
void estado_sensores(void);
 /*-----------------------------------------------------------------------------
 -----------------V A R I A B L E S   A   I M P L E M T E N T A R--------------
 -----------------------------------------------------------------------------*/
const char* ssid = "Casa Bonilla";          // Enter your SSID here
const char* password = "losbonilla2021";    //Enter your Password here
WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)

unsigned char nivel1, nivel2, nivel3;   //variables para sensores de proximidad
unsigned char antirrebote1, antirrebote2, antirrebote3; //antirrebotes
unsigned char temp, aire;               //variables para temperatura y aire
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
  //-------CONFIGURACION DE ENTRADAS Y SALIDAS
  //entradas
  //si ninguno se activa, es porque esta vacio el contenedor
  pinMode(19, INPUT_PULLUP);        //nivel leve
  pinMode(18, INPUT_PULLUP);        //nivel medio
  pinMode(5, INPUT_PULLUP);         //nivel alto
  pinMode(36, INPUT);               //entrada para sensor de co2
  
  //-------CONFIGURACION DE INTERRUPCIONES
  attachInterrupt(digitalPinToInterrupt(19), ISR_n1, FALLING);      //nivel leve
  attachInterrupt(digitalPinToInterrupt(18), ISR_n2, FALLING);      //nivel medio
  attachInterrupt(digitalPinToInterrupt(5), ISR_n3, FALLING);      //nivel alto
  Serial.begin(9600);
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
}
/*-----------------------------------------------------------------------------
 -------------------------- M A I N   L O O P ---------------------------------
 -----------------------------------------------------------------------------*/
void loop()
{
  //-------inicializacion de lectura del adc
  temp = analogRead(36);
  //-------se inicia el servidor web
  server.handleClient();
  //-------antirrebotes de cambios en niveles de basura
  antirrebotes_niveles();
  //-------actualizacion de sensores
  estado_sensores();
  //-------ejecucion de pequeño servidor
  server.send(200, "text/html", SendHTML());
  
  
}
/*-----------------------------------------------------------------------------
 ------------------------- F U N C I O N E S ----------------------------------
 -----------------------------------------------------------------------------*/
 void antirrebotes_niveles(void){
  //-------antirrebote para nivel 1, leve
  if (digitalRead(19)==0 && antirrebote1==1){   
    nivel1=1;       //nivel leve leve on
    //Serial.println("nivel leve de basura");
    
  }
  else{                                           
    nivel1=0;       //nivel leve leve off
  
  }
  //-------antirrebote para nivel 2, medio
  if (digitalRead(18)==0 && antirrebote2==1){   
    nivel2=1;       //nivel medio on
    //Serial1.println("nivel medio de basura");
  }
  else{                                           
    nivel2=0;       //nivel medio off
  
  }
  //-------antirrebote para parqueo 3
  if (digitalRead(5)==0 && antirrebote3==1){  
      nivel3=1;       //nivel alto on
      //Serial1.println("nivel alto de basura");
  }
  else{                                           
    nivel3=0;         //nivel alto off
  
  }
 }
//-------funcion de actualizacion de sensores
void estado_sensores(void){
  if(nivel1==1 && temp>80){
    Serial.print("alto co2");
    Serial.print(" | ");
    Serial.println("bajo nivel de basura");
  }
  else if (nivel1==1 && temp<80){
    Serial.print("bajo co2");
    Serial.print(" | ");
    Serial.println("bajo nivel de basura");
  }
  if(nivel2==1 && temp>80){
    Serial.print("alto co2");
    Serial.print(" | ");
    Serial.println("nivel medio de basura");
  }
  else if(nivel2==1 && temp<80){
    Serial.print("bajo co2");
    Serial.print(" | ");
    Serial.println("nivel medio de basura");
  }
  if(nivel3==1 && temp>80){
    Serial.print("alto co2");
    Serial.print(" | ");
    Serial.println("nivel alto de basura");
  }
  else if(nivel3==1 && temp<80){
    Serial.print("bajo co2");
    Serial.print(" | ");
    Serial.println("nivel alto de basura");
  }
  
 }

//-------Handler de Inicio página
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML());
}

//-------funcion para ejecucion de pequeño servidor web
String SendHTML() {
  String ptr = "<!DOCTYPE html> <html>\n";

  ptr += "  <head>\n";
  ptr += "    <meta chatset=\"utf_8\">\n";
  ptr += "    <title>PROYECTO 3</title>\n";
  ptr += "    <script>\n";
  ptr += "  <!--\n";
  ptr += "  function timedRefresh(timeoutPeriod) {\n";
  ptr += "  \tsetTimeout(\"location.reload(true);\",timeoutPeriod);\n";
  ptr += "  }\n";
  ptr += "\n";
  ptr += "  window.onload = timedRefresh(5000);\n";
  ptr += "\n";
  ptr += "  //   -->\n";
  ptr += "  </script>\n";
 
  ptr += "      <h1 align='center'>PARQUEO CIT</h1>\n";
  ptr += "      <h2 align='center'>Proyecto 3, Electronica Digital 2 </h2></th>\n";
  ptr += "      <h2 align='center'>Julio Avila, Andy Bonilla, Pablo Herrarte </h2></th>\n";
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
//-------Handler de not found
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}
