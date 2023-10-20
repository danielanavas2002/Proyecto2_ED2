//********************************************************************************
// Librerias
//********************************************************************************
#include <SPI.h>
#include <SD.h>
//****************************************************************
// Definición de etiquetas
//****************************************************************
// Botones
#define pinBtnR PF_4 //Boton Recolectar dato del Sensor
#define pinBtnG PF_0 //Boton Guardar 
// TFT
#define CS_PIN PA_3
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
// Varibles del Sensor
float dist = 0.00;
float distA = 0.00;
// Datos de Tiempo para Data Log
int segundo;
int minuto;
int hora;
int dia;
// Funcionamiento de Registro de Tiempo
unsigned long previousMillis = 0;  // Variable para almacenar el tiempo anterior
const unsigned long interval = 1000; // Intervalo de 1 segundo en milisegundos
//********************************************************************************
// Escritura SD
//********************************************************************************
File myFile;
//****************************************************************
// Prototipos de Funciones
//****************************************************************
void datalogger(void);
//*****************************************************************************
// Configuración
//*****************************************************************************
void setup() {
  // Definicion de Entradas
  pinMode(pinBtnR, INPUT_PULLUP);
  pinMode(pinBtnG, INPUT_PULLUP);
  // Monitor Serial 
  Serial.begin(115200); // Con Computadora
  Serial1.begin(115200); // Con ESP32
  // Tarjeta SD
  SPI.setModule(0); // Indicar que el módulo SPI se utilizará
  Serial.print("Inicializando Tarjeta... "); // Inicializar Tarjeta
  if (!SD.begin(CS_PIN)) {
    Serial.println("Inicializacion fallida!"); // Indicar si la inicialización es fallida
    return;
  }
  Serial.println("Inicializacion completa."); // Indicar si la inicialización se completo
  // Crea un nuevo archivo de datos (Si no Existe)
  myFile = SD.open("datalog.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(" "); // Cada que se reinicie la Tiva colocar que es un Nuevo Registro
    myFile.println("**********************************"); 
    myFile.println("          NUEVO REGISTRO          ");
    myFile.println("**********************************");
    myFile.println("DIA | HORA | MIN | SEG | DISTANCIA");
    myFile.close();
  } else {
    Serial.println("Error al abrir el archivo de datos."); // Indicar si no se puede abrir el archivo
  }
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
        Serial1.println('1'); //Enviar un "1" al ESP32 por medio del Serial 1, para que sepa que debe enviar un dato de distancia
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
        datalogger(); // Llamar función data logger para registrar los datos en un archivo de texto en la SD
      }
    }
  }
  btnG_LS = btnG_R; // Guarda el estado actual del Boton Guardar
  // Lectura del Sensor enviada por el ESP32 enviada por Comunicacion Serial
  if (Serial1.available() > 0) {
    distA = Serial1.parseFloat();  // Lee el nuevo valor de distancia desde la comunicación serial
    if (distA != 0.00 && distA != dist) {
      dist = distA; // Actualiza la distancia anterior cuando esta cambia
      Serial.print("Distancia: ");
      Serial.println(dist);
    }
  }
  //Contador para Registro de Tiempo
  unsigned long currentMillis = millis(); // Obtiene el tiempo actual
  if (currentMillis - previousMillis >= interval) { // Comprueba si ha pasado el intervalo de 1 segundo
    previousMillis = currentMillis; // Guarda el tiempo actual como el tiempo anterior
    segundo++; //Aumentar el Tiempo 1 Segundo
  }
  if (segundo >= 60){ //Cuando Segundos llegue a 60 
    segundo = 0; // Reiniciar Segundos
    minuto++; //Aumentar un minuto
    if(minuto >= 60){ //Cuando Minutos llegue a 60
      minuto = 0; // Reiniciar Minutos
      hora++; //Aumentar una hora
      if(hora >= 24){ //Cuando Horas llegue a 24
        hora = 0; // Reiniciar Horas
        dia++; //Aumentar una dia 
      }
    } 
  }
}
//****************************************************************
// Data Logger
//****************************************************************
void datalogger(void){
  myFile = SD.open("datalog.txt", FILE_WRITE); // Abrir Archivo de datos como Escritura
  if (myFile) {
    // Escribe tiempo y distancia en el archivo de datos
    myFile.print(dia);
    myFile.print("   | ");
    myFile.print(hora);
    myFile.print("    | ");
    myFile.print(minuto);
    myFile.print("   | ");
    myFile.print(segundo);
    myFile.print("  | ");
    myFile.print(dist);
    myFile.println(" cm");
    myFile.close();
    Serial.println("Datos registrados en la tarjeta SD."); // Indicar que se guardaron los datos
  } else {
    Serial.println("Error al abrir el archivo de datos."); // Indicar si hay un error con el archivo
  } 
 }
