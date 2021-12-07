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
  pinMode(0, INPUT_PULLUP);        //nivel leve
  pinMode(4, INPUT_PULLUP);        //nivel medio
  pinMode(5, INPUT_PULLUP);        //nivel alto
  /*
  digitalWrite(aire,LOW);
  digitalWrite(CO2,LOW);
  digitalWrite(gas,LOW);*/
  //-------CONFIGURACION DE INTERRUPCIONES
  attachInterrupt(digitalPinToInterrupt(0), ISR_n1, FALLING);      //nivel leve
  attachInterrupt(digitalPinToInterrupt(4), ISR_n2, FALLING);      //nivel medio
  attachInterrupt(digitalPinToInterrupt(5), ISR_n3, FALLING);      //nivel alto
  Serial.begin(115200);
}
/*-----------------------------------------------------------------------------
 -------------------------- M A I N   L O O P ---------------------------------
 -----------------------------------------------------------------------------*/
void loop()
{
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
  if (digitalRead(PD_0)==0 && antirrebote1==1){   //p1 libre
    cuenta_p1=0;
    digitalWrite(PB_0,HIGH);     //verde on
    digitalWrite(PB_0,LOW);     //rojo off
  }
  else{                                           //p1 ocupado
    cuenta_p1=1;
    digitalWrite(PB_0,HIGH);     //verde off
    digitalWrite(PB_0,LOW);     //rojo on
  }
  //-------antirrebote para nivel 2,
  if (digitalRead(PD_1)==0 && antirrebote2==1){   //p2 libre
    cuenta_p2=0;
    digitalWrite(PE_4,HIGH);     //verde on
    digitalWrite(PE_5,LOW);     //rojo off
  }
  else{                                           //p2 ocupado
    cuenta_p2=1;
    digitalWrite(PE_4,HIGH);     //verde off
    digitalWrite(PE_5,LOW);     //rojo on
  }
  //-------antirrebote para parqueo 3
  if (digitalRead(PD_2)==0 && antirrebote3==1){   //p3 libre
    cuenta_p3=0;
    digitalWrite(PB_4,HIGH);     //verde on
    digitalWrite(PA_5,LOW);     //rojo off
  }
  else{                                           //p3 ocupado
    cuenta_p3=1;
    digitalWrite(PB_4,HIGH);     //verde on
    digitalWrite(PA_5,LOW);     //rojo off
  }
  
 }
