//ESP32cam Test

#include "WiFi.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "esp_camera.h"
//#include "Arduino.h"

#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"

#include <SPIFFS.h>
#include <FS.h>

#include <Firebase_ESP_Client.h>
//Provide the token generation process info.}

#include <addons/TokenHelper.h>


//Replace with your network credentials
const char* ssid = "SEMEAR";
const char* password = "SemearEhAmor";

// Insert Firebase project API Key
#define API_KEY "AIzaSyCdOSH0AbxaNYZj4NtK6WuL4PVoKrNnwlU"  //WEB API KEY

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "kiyoshi.araki@usp.br"
#define USER_PASSWORD "projetomk2022"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "smartcabinet-a190c.appspot.com"

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/Gaveta1/photoesp.jpeg"

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

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

bool taskCompleted = false;

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

void setupFingerprintSensor(){
  //Inicializa o sensor
  fingerprintSensor.begin(57600);

  //Verifica se a senha está correta
  if(!fingerprintSensor.verifyPassword())
  {
    //Se chegou aqui significa que a senha está errada ou o sensor está problemas de conexão
    Serial.println(F("Não foi possível conectar ao sensor. Verifique a senha ou a conexão"));
    while(true);
  }
}

//Digital
#include <Adafruit_Fingerprint.h> //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&Serial2, password);

//Senha padrão do sensor de digitais
const uint32_t password = 0x0;

//global de erro da digital
int erro = 0;
String Operador;

//Verifica se a digital está cadastrada
void checkFingerprint()
{
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
    return;
  }
  
  //Procura por este padrão no banco de digitais
  if (fingerprintSensor.fingerFastSearch() != FINGERPRINT_OK)
  {
    //Se chegou aqui significa que a digital não foi encontrada
    Serial.println(F("Digital não encontrada"));
    erro = 0;
    lcd.setCursor(0,0);
    lcd.print("Digital nao");
    lcd.setCursor(0,1);
    lcd.print("encontrada");
    return;
  }

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
}

void setup() {
  // Serial port for debugging purposes
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

  //Portas                  //definir as portas corretas...
  pinMode(gaveta_1, OUTPUT); //Reles
  pinMode(gaveta_2, OUTPUT);
  pinMode(gav_open_1, INPUT); //Sensores
  pinMode(gav_open_2, INPUT);
}

//Variaveis globais do void
int gav_input = 0;
int sai = 1;
int gaveta_open = 0;
int gaveta_open = 0;
int Contador = 0;
String LINK;

void loop() {
  //Inicialização
  lcd.setCursor(0,0);
  lcd.print("Bem Vindo");
  delay(1500);
  //Recebimento dos dados do firebase
       //Gaveta a ser aberta
      sai = 1;
      do{
       lcd.setCursor(0,0);
       lcd.print("Escolha os");
       lcd.setCursor(0,1);
       lcd.print("remedios no APP");
       if(Firebase.getInt("Auxiliary/Acess_Request/Atualizador") == true){
       gav_input = Firebase.getInt("Auxiliary/Acess_Request/Gaveta");
       sai = 0;
       if(gav_open_1 || gav_open_2){  //Arombamento
          //ENVIA SINAL DE ARROMBAMENTO NO FIREBASE!!!!
        }
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
    else{
      digitalWrite(gaveta_2,HIGH);
      }
  }
  delay(3000);
 
  //Central de gravação
  if(erro == 0||gaveta_open_1||gaveta_open_2){
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
  }

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

  //Envio de dados Firebase  //Corrigir como implementar a arvore mudando o nome do contador
  Firebase.pushInt("Gaveta", Contador);
  Firebase.pushInt("Gaveta/Contador", "Horario"); 
  Firebase.pushInt("Gaveta/Contador", Horario); // Horario 
  Firebase.pushString("Gaveta/Contador", "Medicamento");
  Firebase.setString("Gaveta/Contador", Firebase.getString("Auxiliary/Acess_Request/Medicamento"));
  Firebase.pushString("Gaveta/Contador", "Operador");
  Firebase.setString("Gaveta/Contador", Operador);
  Firebase.setString("Gaveta/Contador", "Gif");
  Firebase.setString("Gaveta/Contador", LINK);
  
  Contador++;
}
