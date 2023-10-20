//****************************************************************
// Librerías
//****************************************************************
#include <Arduino.h>
//****************************************************************
// Definición de etiquetas
//****************************************************************
// Sensor Ultrasonido  
#define trigger 32
#define echo 35
//****************************************************************
// Prototipos de Funciones
//****************************************************************
void lecturaSensor(void);
//****************************************************************
// Variables Globales
//****************************************************************
float tiempoE;
float distancia;

int comando;
//****************************************************************
// Configuración
//****************************************************************
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
}
//****************************************************************
// Loop Principal
//****************************************************************
void loop() {
  if(Serial1.available() > 0){
    comando = Serial1.read();
  }

  if(comando == '1'){
    Serial.println("SE DEBE ENVIAR DATO");
    lecturaSensor();
    comando == 0;
  }
}
//****************************************************************
// Lectura Sensor
//****************************************************************
void lecturaSensor(void){
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  tiempoE = pulseIn(echo, HIGH);

  distancia =(tiempoE/2)/29.15;
  Serial.print(distancia);
  Serial.println(" cm");
}