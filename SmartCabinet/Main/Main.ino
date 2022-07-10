//MAIN

//Include
#include "WiFi.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>//Provide the token generation process info.}
#include <Adafruit_Fingerprint.h> //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library


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


//Camera
// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

boolean takeNewPhoto = true;

bool taskCompleted = false;

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}


//Digital
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&Serial2, password);

//Senha padrão do sensor de digitais
const uint32_t password = 0x0;


//Variaveis
//global de erro da digital
int erro = 0;
String Operador;

//Variaveis globais do loop
int gav_input = 0;
int sai = 1;
int gaveta_open = 0;
int gaveta_open = 0;
int Contador = 0;
int Contador_erro = 0;
String LINK;
String LINK_erro;
String Endereco = "Gaveta/";
String Endereco_erro = "Erro/";
float tempo = 0;
String Horario = "00:00:00";
int gaveta_1 = 0;            //PORTAS
int gaveta_2 = 0;
int gav_open_1 = 0; 
int gav_open_2 = 0;


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
       
       if(gav_open_1 || gav_open_2){  //Arombamento
              if (takeNewPhoto) {
              capturePhotoSaveSpiffs();
              takeNewPhoto = false;
            }
            delay(1);
            if (Firebase.ready() && !taskCompleted){
              taskCompleted = true;
              Serial.print("Uploading picture... ");
          
              //MIME type should be valid to avoid the download problem.
              //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
              if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, FILE_PHOTO /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */)){
                Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
                LINK_erro = fbdo.downloadURL().c_str();
              }
              else{
                Serial.println(fbdo.errorReason());
                LINK_erro = "ERRO";
              }
            }
              
              Endereco_erro = String(Endereco_erro + String(Contador_erro));
              
              //Envio de dados Firebase  
              Firebase.pushString("Erro", String(Contador_erro));
              Firebase.pushString(Endereco_erro, "Horario");
              Firebase.setString(String(Endereco_erro + "/Horario"), Horario);
              Firebase.pushString(Endereco_erro, "Gif");
              Firebase.setString(String(Endereco_erro + "/Gif"), LINK_erro);

              Contador_erro++;
              Endereco_erro = "Erro/"
        }
      }while(sai);
       Firebase.setInt("Auxiliary/Acess_Request/Atualizador", false);
       
  //Central de autentificação
    //Digital
      do{
      checkFingerprint();
      }while(erro);
      
      delay(1500);
      lcd.setCursor(0,0);
      lcd.print("Abra a gaveta: ");
      lcd.setCursor(16,0);
      lcd.print(gav_input);
    
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

     
  //Central de gravação
          if (takeNewPhoto) {
            capturePhotoSaveSpiffs();
            takeNewPhoto = false;
          }
          delay(1);
          if (Firebase.ready() && !taskCompleted){
            taskCompleted = true;
            Serial.print("Uploading picture... ");
        
            //MIME type should be valid to avoid the download problem.
            //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
            if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, FILE_PHOTO /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */)){
              Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
              LINK = fbdo.downloadURL().c_str();
            }
            else{
              Serial.println(fbdo.errorReason());
              LINK = "ERRO";
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
    }while(gav_open_1 || gav_open_2)
  
    digitalWrite(gaveta_1,LOW);
    digitalWrite(gaveta_2,LOW);

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
