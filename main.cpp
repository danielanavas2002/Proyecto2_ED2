//****************************************************************
// Librerías
//****************************************************************
#include <Arduino.h>
#include <lecturaSensor.h>
//****************************************************************
// Definición de etiquetas
//****************************************************************
// Sensor Ultrasonico
#define trigger 32
#define echo 35
//****************************************************************
// Variables Globales
//****************************************************************
// Distancia en Sensor Ultrasonico
float distancia;
// Comado cuando se deba enviar un Dato a la Tiva
int comando;
//****************************************************************
// Configuración
//****************************************************************
void setup() {
  // Monitor Serial
  Serial.begin(115200); // Con Computadora
  Serial1.begin(115200); // Con Tiva
  // Definicion de Salidas
  pinMode(trigger, OUTPUT);
  // Definicion de Entradas
  pinMode(echo, INPUT);
}
//****************************************************************
// Loop Principal
//****************************************************************
void loop() {
  if(Serial1.available() > 0){ //Cuando se reciba un dato en el Serial 1
    comando = Serial1.read(); // Leer este valor y almacenar en variable comando
  }
  if(comando == '1'){ // Si comando es igual a 1
    distancia = lectura(trigger, echo); // Se mide la distancia utilizando la Libreria del Sensor
    Serial1.println(distancia); // Enviar este dato en el Serial 1 por UART hacia la Tiva
    Serial.print("Dato enviado: "); //Enviar dato en el Serial 0 por UART hacia la computadora
    Serial.print(distancia);
    Serial.println(" cm");
    comando = 0; // Reiniciar comado
  }
}
