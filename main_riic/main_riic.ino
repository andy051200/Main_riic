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
 
/*-----------------------------------------------------------------------------
 ------------ P R O T O T I P O S   D E   F U N C I O N E S -------------------
 -----------------------------------------------------------------------------*/
void antirrebotes_niveles(void);
 /*-----------------------------------------------------------------------------
 -----------------V A R I A B L E S   A   I M P L E M T E N T A R--------------
 -----------------------------------------------------------------------------*/
int s_analogica_mq135=0;
/*int aire =12;
int CO2=11;
int gas=10;*/
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
  pinMode(5, INPUT_PULLUP);        //nivel alto
  pinMode(36, INPUT);
  /*
  digitalWrite(aire,LOW);
  digitalWrite(CO2,LOW);
  digitalWrite(gas,LOW);*/
  //-------CONFIGURACION DE INTERRUPCIONES
  attachInterrupt(digitalPinToInterrupt(19), ISR_n1, FALLING);      //nivel leve
  attachInterrupt(digitalPinToInterrupt(18), ISR_n2, FALLING);      //nivel medio
  attachInterrupt(digitalPinToInterrupt(5), ISR_n3, FALLING);      //nivel alto
  Serial.begin(9600);
}
/*-----------------------------------------------------------------------------
 -------------------------- M A I N   L O O P ---------------------------------
 -----------------------------------------------------------------------------*/
void loop()
{
  antirrebotes_niveles();
  temp = analogRead(36);
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
  /*
  s_analogica_mq135 = analogRead(36);       
  Serial.println(s_analogica_mq135, DEC);.
  Serial.println(" ppm");
  delay(500);*/
  //-------funciones para no saturar el loop
  
  //-------indicador en comunicacion serial
  
  
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
