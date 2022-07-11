//MAIN

//Include
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <esp_camera.h>
//#include <Arduino.h>
//#include <soc/soc.h>           // Disable brownout problems
//#include <soc/rtc_cntl_reg.h>  // Disable brownout problems
//#include <driver/rtc_io.h>
//#include <SPIFFS.h>
//#include <FS.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>//Provide the token generation process info.}
#include <Adafruit_Fingerprint.h> //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library

// Local include
#include "checkFingerprint.cpp"
#include "initWiFi.cpp"
#include "setupFingerprintSensor.cpp"

//Configurações do WIFI
const char* ssid = "SEMEAR";
const char* password = "SemearEhAmor";


//Firebase
//Chave do Firebase
#define API_KEY "AIzaSyCdOSH0AbxaNYZj4NtK6WuL4PVoKrNnwlU"  //WEB API KEY

//Email e senha do Firebase
#define USER_EMAIL "kiyoshi.araki@usp.br"
#define USER_PASSWORD "projetomk2022"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "smartcabinet-a190c.appspot.com"

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/Gaveta1/photoesp.jpeg"

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

//LCD
#define BACKLIGHT_PIN     13
LiquidCrystal_I2C lcd(0x38,16,2);  // Set the LCD I2C address
//LiquidCrystal_I2C lcd(0x38, BACKLIGHT_PIN, POSITIVE);  // Set the LCD I2C address


//Digital
SoftwareSerial mySerial(2, 3);


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


//Variaveis
//global de erro da digital
int erro = 0;
String Operador;

//Variaveis globais do loop
int gav_input = 0;
int sai = 1;
int gaveta_open_1 = 0;
int gaveta_open_2 = 0;
int Contador = 0;
int Contador_erro = 0;
String LINK;
String LINK_erro;
String Endereco = "Gaveta/";
String Endereco_erro = "Erro/";
float tempo = 0;
String Horario = "00:00:00";
int Segundo = 0; 
int Minuto = 0; 
int Hora = 0;
//PORTAS
int gaveta_1 = 0;            
int gaveta_2 = 0;
int gav_open_1 = 0; 
int gav_open_2 = 0;
int arrombamento = 0;
int grava = 0; 

void setup() {
  Serial.begin(115200);
  
  //Inicializa o sensor de digitais
  setupFingerprintSensor();
  
  //LCD
  lcd.begin (16,2); //SETA A QUANTIDADE DE COLUNAS(16) E O NÚMERO DE LINHAS(2) DO DISPLAY
  lcd.setBacklight(HIGH); //LIGA O BACKLIGHT (LUZ DE FUNDO)
  
  //MODULO WIFI
  initWiFi();
  initSPIFFS();
  
  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();

  //Firebase
  // Assign the api key
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);

  //Portas                  //definir as portas corretas da esp32cam
  pinMode(gaveta_1, OUTPUT); //Reles
  pinMode(gaveta_2, OUTPUT);
  pinMode(gav_open_1, INPUT); //Sensores
  pinMode(gav_open_2, INPUT);
}


void loop() {
  //Inicialização
    lcd.setCursor(0,0);
    lcd.print("Bem Vindo");
    delay(1500);
    
  //Recebimento dos dados do firebase
       //Gaveta a ser aberta
      sai = 1;
      do{
       Segundo = Segundo + (millis() - tempo)/1000;
       tempo = millis();
       if(Segundo > 60){
        Segundo = Segundo - 60;
        Minuto++;
       }
       if(Minuto > 60){
        Minuto = Minuto - 60;
        Hora++;
       }
       if(Hora==25){Hora = 0;}
       
       Horario = String(String(Hora) + ":" + String(Minuto) + ":" + String(Segundo));
       lcd.setCursor(0,0);
       lcd.print("Escolha os");
       lcd.setCursor(0,1);
       lcd.print("remedios no APP");
       
       if(Firebase.getInt("Auxiliary/Acess_Request/Atualizador") == true){
             gav_input = Firebase.getInt("Auxiliary/Acess_Request/Gaveta");
             Hora = Firebase.getInt("Auxiliary/Acess_Request/Hora");
             Minuto = Firebase.getInt("Auxiliary/Acess_Request/Minuto");
             Segundo = Firebase.getInt("Auxiliary/Acess_Request/Segundo");
             tempo = millis();
             sai = 0;
       }
       
      
      }while(sai);
       Firebase.setInt("Auxiliary/Acess_Request/Atualizador", false);
       
  //Central de autentificação
    //Digital
      do{
          erro = 0;
          Serial.println(F("Encoste o dedo no sensor"));
          lcd.print("Insira a digital");
          
          //Espera até pegar uma imagem válida da digital
          while (fingerprintSensor.getImage() != FINGERPRINT_OK);
        
          //Converte a imagem para o padrão que será utilizado para verificar com o banco de digitais
          if (fingerprintSensor.image2Tz() != FINGERPRINT_OK)
          {
            //Se chegou aqui deu erro, então abortamos os próximos passos
            Serial.println(F("Erro image2Tz"));
            erro = 1;
            lcd.setCursor(0,0);
            lcd.print("ERRO");
          }
          
          //Procura por este padrão no banco de digitais
          if (fingerprintSensor.fingerFastSearch() != FINGERPRINT_OK)
          {
            //Se chegou aqui significa que a digital não foi encontrada
            Serial.println(F("Digital não encontrada"));
            erro = 1;
            lcd.setCursor(0,0);
            lcd.print("Digital nao");
            lcd.setCursor(0,1);
            lcd.print("encontrada");
          }
        if(erro == 0){
          //Se chegou aqui a digital foi encontrada
          //Mostramos a posição onde a digital estava salva e a confiança
          //Quanto mais alta a confiança melhor
          Serial.print(F("Digital encontrada com confiança de "));
          Serial.print(fingerprintSensor.confidence);
          Serial.print(F(" na posição "));
          Serial.println(fingerprintSensor.fingerID);
            Operador = fingerprintSensor.fingerID;
            erro = 0;
            lcd.setCursor(0,0);
            lcd.print("Acesso");
            lcd.setCursor(0,1);
            lcd.print("Concedido");
            digitalWrite(arrombamento,LOW);
            digitalWrite(grava,HIGH);
            
        }
         if(gav_open_1 == 0||gav_open_2 == 0){
            //Avisa a espcam
            erro = 1;
            digitalWrite(arrombamento,HIGH);
            digitalWrite(grava,HIGH);
            
            Endereco_erro = String(Endereco_erro + String(Contador_erro));
              
              //Envio de dados Firebase  
              Firebase.pushString("Erro", String(Contador_erro));
              Firebase.pushString(Endereco_erro, "Horario");
              Firebase.setString(String(Endereco_erro + "/Horario"), Horario);

              Contador_erro++;
              Endereco_erro = "Erro/"
              
            //Alerta
            while(gav_open_1 == 0||gav_open_2 == 0){
            lcd.setCursor(0,0);
            lcd.print("ARROMBAMENTO");
            delay(500);
            lcd.print("             ");
            delay(500);
            }
            digitalWrite(grava,LOW);
            }
        } 
      }while(erro);
      
      delay(1500);
      lcd.setCursor(0,0);
      lcd.print("Abra a/s gaveta/s: ");
      
      if(gav_input = 0){
        lcd.setCursor(16,0);
        lcd.print("1");
      }
       if(gav_input = 1){
        lcd.setCursor(16,0);
        lcd.print("2");
      }
       if(gav_input = 2){
        lcd.setCursor(0,1);
        lcd.print("1 e 2");
      }
      
  //Abertura das gavetas
      if(erro == 0){
        if(gav_input == 0){
          digitalWrite(gaveta_1,HIGH);
          }
        if(gav_input == 1){
          digitalWrite(gaveta_2,HIGH);
          }
        if(gav_input == 2){
          digitalWrite(gaveta_2,HIGH);
          digitalWrite(gaveta_1,HIGH)
          }
      }

     
    delay(3000);
    
  //Retravamento
    do{
        lcd.setCursor(0,0);
        lcd.print("Feche as gavetas");
        delay(1000);
        lcd.setCursor(0,0);
        lcd.print("                ");
    }while(gav_open_1 == 0 || gav_open_2 == 0)
  
    digitalWrite(gaveta_1,LOW);
    digitalWrite(gaveta_2,LOW);
    digitalWrite(grava,LOW);
    
    Endereco = String(Endereco + String(Contador));
  
  //Envio de dados Firebase  
    Firebase.pushString("Gaveta", String(Contador));
    Firebase.pushString(Endereco, "Horario");
    Firebase.setString(String(Endereco + "/Horario"), Horario);
    Firebase.pushString(Endereco, "Medicamento");
    Firebase.setString(String(Endereco + "/Medicamento"), Firebase.getString("Auxiliary/Acess_Request/Medicamento"));
    Firebase.pushString(Endereco, "Operador");
    Firebase.setString(String(Endereco + "/Operador"), Operador);
    Firebase.pushString(Endereco, "Gif");
    Firebase.setString(String(Endereco + "/Gif"), LINK);
    
    Contador++;
    Endereco = "Gaveta/"
}
