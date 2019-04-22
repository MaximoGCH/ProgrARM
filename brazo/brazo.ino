#include <Wire.h>
#include <Servo.h>
#include <EEPROM.h>
#include <String.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
//Stepper
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *stepper = AFMS.getStepper(400, 1);
//~Stepper

//Servos
Servo servo_pinza;
Servo servo_giropinza;
Servo servo_Codo1R;
Servo servo_Codo1L;
Servo servo_Codo2;
Servo servo_Codo3;
//~Servos
const int idcodo1 = 1;
const int idcodo2 = 2;
const int idcodo3 = 3;
const int idpinza = 4;
const int idgiropinza = 5;
const int idbase = 6;
//Angulo servos
int poscodo1[5];
int poscodo2[5];
int poscodo3[5];
int pospinza[5];
int posgiropinza[5];
int posbase[5];
//~Angulo servos
//Angulo actual
int acodo1=90;
int acodo2=180;
int acodo3=50;
int apinza=90;
int agiropinza=90;
int abase=90;
//~Angulo actual
//Numero de instrucciones
int ncodo1 = -1;
int ncodo2 = -1;
int ncodo3 = -1;
int npinza = -1;
int ngiropinza = -1;
int nbase = -1;
int nInstrucciones = 0;
//~Numero de instrucciones

int *orden = (int *) malloc(nInstrucciones * (sizeof(int)));

//Bourns
const int bourns_pin = A0;
int bourns_min = 220;
int bourns_max = 880;
int bourns_value = 0;
int stepper_punto;
//~Bourns

String prueba ;
char prueba2[1024];
int nBloques = 0;
char **listaBloques = (char**)malloc(nBloques * (sizeof(char*)));
void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(bourns_pin, INPUT);
  servo_Codo1L.attach(7);
  servo_Codo1R.attach(6);
  servo_Codo2.attach(3);
  servo_Codo3.attach(4);
  servo_giropinza.attach(5);
  servo_pinza.attach(2);
  AFMS.begin();
  stepper->setSpeed(1);
  Serial.println("Bienvenido a PorgrARM\n");
  servo_Codo1R.write(90);
  servo_Codo1L.write(70);
  servo_Codo2.write(180);
  servo_Codo3.write(50);
  delay(10);
  getPositionBourn();

  


}

void loop() {
if (Serial1.available() > 0) {
  
prueba= Serial1.readString();
Serial.println(prueba); 
prueba.toCharArray(prueba2, 1024);
DividirBloq(prueba2);
}
if (Serial.available() > 0) {

prueba= Serial.readString();
Serial.println(prueba); 
prueba.toCharArray(prueba2, 1024);
DividirBloq(prueba2);
}
delay(50);
}

void getPositionBourn() {
  bourns_value = analogRead(bourns_pin);
  Serial.println(bourns_value);
  delay(5);
  while ( bourns_value != bourns_min) {
    bourns_value = analogRead(bourns_pin);
    Serial.println(bourns_value);
    if (bourns_value > bourns_min) {
      stepper->onestep(FORWARD, MICROSTEP);
    }
    if (bourns_value < bourns_min) {
      stepper->onestep(BACKWARD, MICROSTEP);
    }
  }
  stepper_punto = 0;
}

void GirarBase(int angulo){
  stepper_punto=map(angulo,0,360,bourns_min,bourns_max);
  Serial.println(stepper_punto);
  Serial.println("________________________________");
  bourns_value = analogRead(bourns_pin);
  while ( bourns_value != stepper_punto) {
    bourns_value = analogRead(bourns_pin);
    Serial.println(bourns_value);
    if (bourns_value >  stepper_punto) {
      stepper->onestep(FORWARD, MICROSTEP);
    }
    if (bourns_value <  stepper_punto) {
      stepper->onestep(BACKWARD, MICROSTEP);
    }
  }
  
  
  }

void DividirBloq(char bloques[]) {
  Serial.println(bloques);
  char* temp = strtok(bloques, "|");
  while (temp != 0) {
    nBloques++;
    listaBloques = realloc(listaBloques, nBloques * (sizeof(String)));
    listaBloques[nBloques - 1] = temp;
    Serial.println(temp);
    temp = strtok(0, "|");
  }
  for (int i = 0; i < nBloques; i++) {
    Serial.print("*****************Bloque ");
    Serial.print(i);
    Serial.println("*****************");
    ParsePos(listaBloques[i]);
    Ejecutar();
    delay(1000);
  }
  nBloques=0;
}
void ParsePos(char posicion[]) {


  
  char* temp = strtok(posicion, ";");
  while (temp != 0) {
    char *separador = strchr(temp, ',');
    if (separador != 0) {
      *separador = 0;
      separador++;
      switch (atoi(temp)) {
        case idcodo1:
          if ((atoi(separador)) != 0) {
            ncodo1++;
            poscodo1[ncodo1] = atoi(separador);
            nInstrucciones++;
            orden = realloc (orden, nInstrucciones * (sizeof(int)));
            orden[nInstrucciones - 1] = idcodo1;
          }
          break;
        case idcodo2:

          if ((atoi(separador)) != 0) {
            ncodo2++;
            poscodo2[ncodo2] = atoi(separador);
            nInstrucciones++;
            orden = realloc (orden, nInstrucciones * (sizeof(int)));
            orden[nInstrucciones - 1] = idcodo2;
          }
          break;
        case idcodo3:
          if ((atoi(separador)) != 0) {
            ncodo3++;
            poscodo3[ncodo3] = atoi(separador);
            nInstrucciones++;
            orden = realloc (orden, nInstrucciones * (sizeof(int)));
            orden[nInstrucciones - 1] = idcodo3;
          }
          break;
        case idpinza:
          if ((atoi(separador)) != 0) {
            npinza++;
            pospinza[npinza] = atoi(separador);
            nInstrucciones++;
            orden = realloc (orden, nInstrucciones * (sizeof(int)));
            orden[nInstrucciones - 1] = idpinza;
          }
          break;
        case idgiropinza:

          if ((atoi(separador)) != 0) {
            ngiropinza++;
            posgiropinza[ngiropinza] = atoi(separador);
            nInstrucciones++;
            orden = realloc (orden, nInstrucciones * (sizeof(int)));
            orden[nInstrucciones - 1] = idgiropinza;
          }
          break;
        case idbase:
          if ((atoi(separador)) != 0) {
            nbase++;
            posbase[nbase] = atoi(separador);
            nInstrucciones++;
            orden = realloc (orden, nInstrucciones * (sizeof(int)));
            orden[nInstrucciones - 1] = idbase;
          }
          break;
      }
    }
    temp = strtok(0, ";");
  }
  Serial.println("*****************Angulos*****************");
  Serial.print("Angulo codo 1 = ");
  for (int n = 0; n <= ncodo1; n++) {
    Serial.print(poscodo1[n]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("Angulo codo 2 = ");
  for (int n = 0; n <= ncodo2; n++) {
    Serial.print(poscodo2[n]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("Angulo codo 3 = ");
  for (int n = 0; n <= ncodo3; n++) {
    Serial.print(poscodo3[n]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("Angulo pinza = ");
  for (int n = 0; n <= npinza; n++) {
    Serial.print(pospinza[n]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("Angulo giro pinza = ");
  for (int n = 0; n <= ngiropinza; n++) {
    Serial.print(posgiropinza[n]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("Angulo base = ");
  for (int n = 0; n <= nbase; n++) {
    Serial.print(posbase[n]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println();
  Serial.println("*****************Instrucciones*****************");
  for (int i = 0; i < nInstrucciones; i++) {
    Serial.print("Instrucion nº");
    Serial.print(i + 1);
    Serial.print(" = ");
    switch (orden[i]) {
      case idcodo1:
        Serial.println("Codo 1");
        break;
      case idcodo2:
        Serial.println("Codo 2");
        break;
      case idcodo3:
        Serial.println("Codo 3");
        break;
      case idpinza:
        Serial.println("Pinza");
        break;
      case idgiropinza:
        Serial.println("Giro Pinza");
        break;
      case idbase:
        Serial.println("Giro Base");
        break;
    }

  }



}
bool Ejecutar(){
  int cont=0;
  int id;
  for(int i ; i<nInstrucciones;i++){
    
    Serial.println(orden[i]);
    }
  while(nInstrucciones!=0){
    id=orden[cont];
      Serial.println();
    switch(id){
    case 1:
      Serial.println(poscodo1[0]);
      while(acodo1!=poscodo1[0]){
        if(acodo1>poscodo1[0])
          acodo1--;
          else
          acodo1++;
      servo_Codo1L.write(160-acodo1);
      servo_Codo1R.write(acodo1);
      delay(30);
      }
      ncodo1--;
      break;
    case 2:
    while(acodo2!=poscodo2[0]){
        if(acodo2>poscodo2[0])
          acodo2--;
          else
          acodo2++;
      Serial.println(acodo2);
      servo_Codo2.write(acodo2);
      
      delay(100);
    }
    ncodo2--;
     break;
    case 3:
    while(acodo3!=poscodo3[0]){
        if(acodo3>poscodo3[0])
          acodo3--;
          else
          acodo3++;
      Serial.println(acodo3);
      servo_Codo3.write(acodo3);
      
      delay(80);
      }
      ncodo3--;
      break;
    case 4:
      Serial.println(pospinza[0]);
      servo_pinza.write(pospinza[0]);
      npinza--;
      break;
    case 5:
    Serial.println(posgiropinza[0]);
    servo_giropinza.write(posgiropinza[0]);
    ngiropinza--;
      break;
    case 6:
    GirarBase(posbase[0]);
    nbase--;
      break;
      }
      nInstrucciones--;
      cont++;
  delay(100);
  }
  delay(600);
  return true;
  
  }
