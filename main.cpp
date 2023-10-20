//****************************************************************
// Librerías
//****************************************************************
#include <Arduino.h>
//****************************************************************
// Definición de etiquetas
//****************************************************************
// Sensor Ultrasonico
#define trigger 32
#define echo 35
//****************************************************************
// Prototipos de Funciones
//****************************************************************
void lecturaSensor(void);
//****************************************************************
// Variables Globales
//****************************************************************
// Funcionamiento Sensor Ultrasonico
float tiempoE;
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
    lecturaSensor(); // Realizar una lectura del Sensor 
    Serial1.println(distancia); // Enviar este dato en el Serial 1
    comando = 0; // Reiniciar comado
  }
}
//****************************************************************
// Lectura Sensor
//****************************************************************
void lecturaSensor(void){
  digitalWrite(trigger, LOW); // Apagar el trigger 2 microsegundos
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH); // Encender el trigger 10 microsegundos
  delayMicroseconds(10);
  digitalWrite(trigger, LOW); // Apagar nuevamente el trigger
  tiempoE = pulseIn(echo, HIGH); // Tiempo en microsegundos que el echo esta en HIGH
  distancia = (tiempoE/2)/29.15; // Convertir este tiempo a una medida equivalente en Segundos
  Serial.print("Dato Enviado: "); // Enviar dato en Serial 0 para que sea visible en la Computadora
  Serial.print(distancia); 
  Serial.println(" cm");
}