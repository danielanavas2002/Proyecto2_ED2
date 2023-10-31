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
// Distancia medida por Sensor Ultrasonico
float distancia;
float distanciaI;
float alturaRec = 12.60;
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
    distanciaI = lectura(trigger, echo); // Se mide la distancia utilizando la Libreria del Sensor
    distancia = alturaRec - distanciaI; //Se calcula la altura del nivel del agua sabiendo el alto del recipiente
    if (distancia <= 0){ //Si en algun momento se da una medicion erronea y se tiene un numero negativo
      distancia = 0.01; //Enviar un dato mayor a 0 para evitar problemas en la Tiva
    }
    Serial1.println(distancia); // Enviar este dato en el Serial 1 por UART hacia la Tiva
    Serial.print("Dato enviado: "); //Enviar dato en el Serial 0 por UART hacia la computadora
    Serial.print(distancia);
    Serial.println(" cm");
    comando = 0; // Reiniciar comado
  }
}