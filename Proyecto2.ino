 //********************************************************************************
// Librerias
//********************************************************************************
#include <SPI.h>
#include <SD.h>

#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"
//****************************************************************
// Definición de etiquetas
//****************************************************************
// Botones
#define pinBtnR PF_4 //Boton Recolectar dato del Sensor
#define pinBtnG PF_0 //Boton Guardar 
// SD
#define CS_PIN PA_3
// Buzzer Pasivo
#define pinBuzzer PF_2
// Led Rojo
#define pinLedR PF_1
// TFT
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1 
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7}; 
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
// Textos TFT
String diaString;
String horaString;
String minString;
String segString;
String distString;
String dospuntos = ":";
String text1 = " ALTURA ";
String text2 = "DEL AGUA";
String text3 = " ULTIMO ";
String text4 = "REGISTRO";
String cmtxt = "cm";
String errortxt = "ERROR";
String dospuntostxt = ":";
String diastxt = "DIAS:";
int distDecimal;
int distEntero;

int porLlenado;
//********************************************************************************
// Escritura SD
//********************************************************************************
File myFile;
//****************************************************************
// Prototipos de Funciones
//****************************************************************
void datalogger(void);
// Pantalla TFT
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);


extern uint8_t fondo[];

//*****************************************************************************
// Configuración
//*****************************************************************************
void setup() {
  // Definicion de Entradas
  pinMode(pinBtnR, INPUT_PULLUP);
  pinMode(pinBtnG, INPUT_PULLUP);
  // Definicion de Salidas
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinLedR, OUTPUT);
  // Monitor Serial 
  Serial.begin(115200); // Con Computadora
  Serial2.begin(115200); // Con ESP32
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
  // Pantalla TFT
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  LCD_Init();
  LCD_Clear(0x00);
  FillRect(0, 0, 320, 240, 0xA71F);
  LCD_Print(diastxt, 195, 10, 2, 0x00, 0xA71F);
  FillRect(0, 30, 320, 3, 0x19EB);
  LCD_Print(text1, 20, 45, 2, 0x00, 0xA71F);
  LCD_Print(text2, 20, 65, 2, 0x00, 0xA71F);
  FillRect(15, 85, 135, 45, 0x03F9);
  LCD_Print(text3, 20, 140, 2, 0x00, 0xA71F);
  LCD_Print(text4, 20, 160, 2, 0x00, 0xA71F);
  FillRect(15, 180, 135, 55, 0x022B);
  FillRect(165, 215, 140, 20, 0x00);
  LCD_Bitmap(185, 55, 100, 145, gota);    
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
        Serial2.println('1'); //Enviar un "1" al ESP32 por medio del Serial 1, para que sepa que debe enviar un dato de distancia
        tone(pinBuzzer, 2093, 200); // Indicador auditivo por medio de Buzzer Pasivo por 200 ms
        digitalWrite(pinLedR, HIGH);
        delay(200);
        digitalWrite(pinLedR, LOW);
        noTone(pinBuzzer);
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
        FillRect(15, 180, 135, 55, 0x022B);
        LCD_Print(horaString, 20, 190, 2, 0xffff, 0x022B);
        LCD_Print(dospuntostxt, 50, 190, 2, 0xffff, 0x022B);
        LCD_Print(minString, 65, 190, 2, 0xffff, 0x022B);
        LCD_Print(dospuntostxt, 95, 190, 2, 0xffff, 0x022B);
        LCD_Print(segString, 110, 190, 2, 0xffff, 0x022B);
        LCD_Print(diastxt, 30, 210, 2, 0xffff, 0x022B);
        LCD_Print(diaString, 110, 210, 2, 0xffff, 0x022B);
        tone(pinBuzzer, 3136, 200); // Indicador auditivo por medio de Buzzer Pasivo por 200 ms
        delay(200);
        noTone(pinBuzzer);
      }
    }
  }
  btnG_LS = btnG_R; // Guarda el estado actual del Boton Guardar
  // Lectura del Sensor enviada por el ESP32 enviada por Comunicacion Serial
  if (Serial2.available() > 0) {
    distA = Serial2.parseFloat();  // Lee el nuevo valor de distancia desde la comunicación serial
    if (distA != 0.00 && distA != dist) {
      dist = distA; // Actualiza la distancia anterior cuando esta cambia
       distEntero = int(dist);  // Obtiene la parte entera
       distDecimal = (dist - distEntero)*100;  // Multiplica por 100 para obtener 2 decimales
       if(distEntero < 10){
        distString = "0" + String(distEntero) + "." + String(distDecimal); //Arreglar a String
       } else{
        distString = String(distEntero) + "." + String(distDecimal); //Arreglar a String
       }
       FillRect(15, 85, 135, 45, 0x03F9);
       LCD_Print(distString, 20, 100, 2, 0xffff, 0x03F9);
       LCD_Print(cmtxt, 110, 100, 2, 0xffff, 0x03F9);
       porLlenado = map(dist, 0.00, 11.50, 10, 130);
       FillRect(165, 215, 140, 20, 0x00);
       FillRect(170, 220, porLlenado, 10, 0x13F2);
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
  if(segundo < 10){
    segString = "0" + String(segundo); //Arreglar a String
  } else{
    segString = String(segundo);
  }
  if(minuto < 10){
    minString = "0" + String(minuto); //Arreglar a String
  } else{
    minString = String(minuto);
  }
  if(hora < 10){
    horaString = "0" + String(hora); //Arreglar a String
  } else{
    horaString = String(hora);
  }
  if(dia < 10){
    diaString = "0" + String(dia); //Arreglar a String
  } else{
    diaString = String(dia);
  }
  LCD_Print(horaString, 10, 10, 2, 0x00, 0xA71F);
  LCD_Print(dospuntostxt, 40, 10, 2, 0x00, 0xA71F);
  LCD_Print(minString, 55, 10, 2, 0x00, 0xA71F);
  LCD_Print(dospuntostxt, 85, 10, 2, 0x00, 0xA71F);
  LCD_Print(segString, 100, 10, 2, 0x00, 0xA71F);
  LCD_Print(diaString, 275, 10, 2, 0x00, 0xA71F);
  
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
    Serial.println("Dato registrados en la tarjeta SD."); // Indicar que se guardaron los datos
  } else {
    Serial.println("Error al abrir el archivo de datos."); // Indicar si hay un error con el archivo
    FillRect(15, 180, 135, 55, 0xD7C444);
    LCD_Print(errortxt, 20, 180, 2, 0xffff, 0xD7C444);
  } 
 }
 //***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
/*void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
*/

void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+w;
  y2 = y+h;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = w*h*2-1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
      
      //LCD_DATA(bitmap[k]);    
      k = k - 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    
    
    }
  digitalWrite(LCD_CS, HIGH);
}
