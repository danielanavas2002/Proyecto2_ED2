//****************************************************************
// Definición de etiquetas
//****************************************************************
#include <Arduino.h>
#define pinBtnR PF_4 //Boton Recolectar dato del Sensor
#define pinBtnG PF_0 //Boton Guardar 
//****************************************************************
// Variables Globales
//****************************************************************
// Funcionamiento Boton Recolectar dato del Sensor
int btnR_S = LOW; // Estado actual del Boton Recolectar
int btnR_LS = LOW; // Estado anterior del Boton Recolectar
int btnR_R; //Lectura Estado de Boton Recolectar
// Funcionamiento Guardar dato del Sensor
int btnG_S = LOW; // Estado actual del Boton Guardar
int btnG_LS = LOW; // Estado anterior del Boton Guardar
int btnG_R; //Lectura Estado de Boton Guardar
// Antirebote de los Botones
unsigned long lastDebounceTime = 0; // Último momento en que se cambió el estado del botón
unsigned long debounceDelay = 50; // Tiempo de rebote en milisegundos
//*****************************************************************************
// Configuración
//*****************************************************************************
void setup() {
  // Definicion de Entradas
  pinMode(pinBtnR, INPUT_PULLUP);
  pinMode(pinBtnG, INPUT_PULLUP);
  // Monitor Serial 
  Serial.begin(115200); //Con Computadora
  Serial1.begin(115200); //Con ESP32
}
//*****************************************************************************
// Loop Principal
//*****************************************************************************
void loop() {
  // Lectura de Estado de Botones
  btnR_R = digitalRead(pinBtnR); // Lee el estado actual del Boton Recolectar
  btnG_R = digitalRead(pinBtnG); // Lee el estado actual del Boton Guardar
  // Boton Recolectar
  if (btnR_R != btnR_LS) { 
    lastDebounceTime = millis(); // Si el estado del Boton Recolectar ha cambiado, actualiza el tiempo de rebote
  } if ((millis() - lastDebounceTime) > debounceDelay) { // Verifica si ha pasado suficiente tiempo desde el último cambio del Boton Recolectar para evitar el rebote
    if (btnR_R != btnR_S) { // Si es asi, actualiza el estado del Boton Recolectar
      btnR_S = btnR_R;
      if (btnR_S == LOW) {
        Serial.println("Solicitar Dato"); //Mostrar mensaje de Solicitar Dato
        Serial1.println('1'); //Enviar un "1" al ESP32 por medio del Serial 1, para que sepa que debe enviar 1 dato 
      }
    }
  }
  btnR_LS = btnR_R; // Guarda el estado actual del Boton Recolectar
  // Boton Guardar
  if (btnG_R != btnG_LS) { 
    lastDebounceTime = millis(); // Si el estado del Boton Guardar ha cambiado, actualiza el tiempo de rebote
  } if ((millis() - lastDebounceTime) > debounceDelay) { // Verifica si ha pasado suficiente tiempo desde el último cambio del Boton Guardar para evitar el rebote
    if (btnG_R != btnG_S) { // Si es asi, actualiza el estado del Boton Guardar
      btnG_S = btnG_R;
      if (btnG_S == LOW) {
        Serial.println("Guardar Dato"); //Guardar Dato 
      }
    }
  }
  btnG_LS = btnG_R; // Guarda el estado actual del Boton Guardar  
}
