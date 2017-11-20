#include <LiquidCrystal.h>
#include "max6675.h"

//Pinout
#define HEATER1     3
#define HEATER2     6
#define TSCK        13
#define TMISO       12
#define TCS         10
#define RS          A5
#define EN          A4
#define D4          A3
#define D5          A2
#define D6          A1
#define D7          A0
#define ENCA        0
#define ENCB        1
#define ENCBTN      2
//Temperaturas
#define TEST        0
#define TEMP_TEST   100
#define PLA         1
#define TEMP_PLA    200
#define PET         2
#define TEMP_PET    255
#define PP          3
#define TEMP_PP     170
//PID
#define SAMPLETIME  250
#define OUTMAX      255
#define OUTMIN      0
#define UMBRALINT   5
#define Kp          35.0
#define Kd          49.0
#define Ki          2.0
//Otros
#define AUTOMATIC   0
#define MANUAL      1

unsigned int tempMaterial=0;
int material=0;
byte modo=MANUAL;
char menuOpcionSelect=0;
byte menuEstado=0;

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
MAX6675 termo(TSCK, TCS, TMISO);

void setup() {
  pinMode(HEATER1,OUTPUT);
  digitalWrite(HEATER1,0);
  //pinMode(HEATER2,OUTPUT);
  //digitalWrite(HEATER2,0);
  pinMode(ENCA,INPUT);
  pinMode(ENCB,INPUT);
  pinMode(ENCBTN,INPUT);
  lcd.begin(16, 2);
  cambiarMaterial(TEST);
  /*lcd.print("  PlastiCiudad");
  delay(1000);*/
}

void loop() {
  if(boton())digitalWrite(HEATER1,0);
  //menu();
  encoder();
  //calcularPID(getTemp(),(float)tempMaterial,HEATER1);
  deadTime((float)tempMaterial,1);
}

void deadTime(float setpoint,unsigned int timeOut){
  static unsigned long timeAnt=millis();
  if(millis()-timeAnt > timeOut){
    if(getTemp()<setpoint)digitalWrite(HEATER1,0);
    else digitalWrite(HEATER1,1);
  }
}

void menu(void){
  static byte menuOpcion=0;
  static byte menuOpcionSelectAnt=0;
  static byte btnAnt=0;
  if(!menuEstado && boton() && !btnAnt){//Entra al menú
    menuEstado=1;       //Menú activo
    menuOpcionSelect=0; //Resetea la opcion
    btnAnt=1;           //Se presionó el botón
    
    lcd.setCursor(0,0);
    lcd.print("|_____MENU_____|");
    lcd.setCursor(0,1);
    lcd.print("<-            ");
    lcd.setCursor(14,1);
    lcd.print("->");
    lcd.setCursor(5,1);
    lcd.print(" Modo ");//Modo automático/manual
  }
  else if(menuEstado && boton() && !btnAnt){//Selecciona la opción
    menuOpcion=menuOpcionSelect;
    btnAnt=1;
    
    lcd.setCursor(0,0);
    switch(menuOpcion){
      case 0:
      break;
      case 1:
      break;
      case 2: menuEstado=0;
              lcd.clear();
              cambiarMaterial(material);
      break;
    }
  }
  if(menuOpcionSelect != menuOpcionSelectAnt && menuEstado){
    menuOpcionSelectAnt=menuOpcionSelect;
    switch(menuOpcionSelect){
      case 0: lcd.setCursor(5,1);
              lcd.print(" Modo ");//Modo automático/manual
      break;
      case 1: lcd.setCursor(5,1);
              lcd.print("Config");//Configuración de Kp, Ki, Kd
      break;
      case 2: lcd.setCursor(5,1);
              lcd.print("Volver");//Salir del menú
      break;
    }
  }
  if(btnAnt && !boton())btnAnt=0;
}

byte boton(void){
  static byte estado=0;
  static unsigned long tiempoAnt=0;
  if(!digitalRead(ENCBTN)){//Si se presionó
    if((millis()-tiempoAnt) > 20){
      tiempoAnt=millis();
      if(!digitalRead(ENCBTN))estado=1;
      else estado=0;
    }
  }
  else estado=0;
  return estado;
}
void encoder(void){
  static byte dataAnt=1;
  byte data=digitalRead(ENCA);
  
  if(!data){//Antirebote
    delayMicroseconds(5);
    data = digitalRead(ENCA);
  }
  if(dataAnt && !data){
    if(digitalRead(ENCB)){//CW
      if(!menuEstado){ //No está en el menú
        if(modo == AUTOMATIC){
          material++;
          if(material>3)material=0;
          cambiarMaterial(material);
        }
        else{
          if(tempMaterial<300){
            tempMaterial++;
            lcd.setCursor(11,0);
            lcd.print(tempMaterial);
            lcd.print((char)223);
            lcd.print("C ");
          }
        }
      }
      else{               //Está en el menú
        if(menuEstado==1){//Menú principal
          menuOpcionSelect++;
          if(menuOpcionSelect>2)menuOpcionSelect=0;
        }
      }
    }
    else{//CCW
      if(!menuEstado){ //No está en el menú
        if(modo == AUTOMATIC){
          material--;
          if(material<0)material=3;
          cambiarMaterial(material);
        }
        else{
          if(tempMaterial>0){
            tempMaterial--;
            lcd.setCursor(11,0);
            lcd.print(tempMaterial);
            lcd.print((char)223);
            lcd.print("C ");
          }
        }
      }
      else{            //Está en el menú
        if(menuEstado==1){//Menú principal
          menuOpcionSelect--;
          if(menuOpcionSelect<0)menuOpcionSelect=2;
        }
      }
    }
  }
  dataAnt = data;
}
float getTemp(void){
  static unsigned int tempActual;
  static unsigned long tiempoAnterior=0;
  if((millis()-tiempoAnterior)>250){
    tiempoAnterior=millis();
    tempActual=termo.readCelsius();
    if(!menuEstado){
      lcd.setCursor(0,1);
      lcd.print("Temp: ");
      lcd.print(tempActual);
      lcd.print((char)223);
      lcd.print("C");
      lcd.print("  ");
    }
  }
  return tempActual;
}
void cambiarMaterial(byte material){
  if(!menuEstado){
    lcd.setCursor(0,0);
    lcd.print("Mat: ");
    switch(material){
      case TEST: lcd.print("TEST");
                tempMaterial=TEMP_TEST;
      break;
      case PLA: lcd.print("PLA ");
                tempMaterial=TEMP_PLA;
      break;
      case PET: lcd.print("PET ");
                tempMaterial=TEMP_PET;
      break;
      case PP: lcd.print("PP  ");
                tempMaterial=TEMP_PP;
      break;
    }
    lcd.setCursor(11,0);
    lcd.print(tempMaterial);
    lcd.print((char)223);
    lcd.print("C ");
  }
}
void calcularPID(float input, float setpoint, byte outputPin){
  static float error=0, errorAnterior=0, inputAnterior=0;
  static unsigned long tiempoAnterior=0;
  static float P=0, I=0, D=0;
  static int output=0;
  
  if((millis()-tiempoAnterior)>=SAMPLETIME){
    tiempoAnterior=millis();

    errorAnterior=error;
    error=setpoint-input;
    
    //Proporcional
    P= (float)Kp*error;

    //Integral
    if(abs(error)<=UMBRALINT){  //Cuando se encuentre cerca del setpoint
      I+=(float)Ki*error;       //Empieza a sumar los puntos
      if(I>OUTMAX)I=OUTMAX;     //Mantiene los valores en rango
      else if(I<OUTMIN)I=OUTMIN;
    }
    else{     //Cuando salió del rango
      I=0.0;  //Reinicia el término integral
    }

    //Derivativa
    //D=Kd*(error-errorAnterior); //<-- Produce derivative kick
    D=(float)Kd*(input-inputAnterior); //<-- Derivada de la medición
    inputAnterior=input;

    //Salida
    output = (int)P + (int)I + (int)D;
    
    if(output>OUTMAX)output=OUTMAX; //Mantiene los valores en rango
    else if(output<OUTMIN)output=OUTMIN;

    /*lcd.setCursor(0,0);
    lcd.print("PWM: ");
    lcd.print(output);
    lcd.print("    ");*/
    
    analogWrite(outputPin,output);
  }
}

