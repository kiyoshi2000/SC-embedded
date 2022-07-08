// ESP32cam Test

#include "WiFi.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "esp_camera.h"
//#include "Arduino.h"

#include "soc/soc.h"          // Disable brownout problems
#include "soc/rtc_cntl_reg.h" // Disable brownout problems
#include "driver/rtc_io.h"

#include <SPIFFS.h>
#include <FS.h>

#include <Firebase_ESP_Client.h>
// Provide the token generation process info.}

#include <addons/TokenHelper.h>

// Replace with your network credentials
const char *ssid = "SEMEAR";
const char *password = "SemearEhAmor";

// Insert Firebase project API Key
#define API_KEY "AIzaSyCdOSH0AbxaNYZj4NtK6WuL4PVoKrNnwlU" // WEB API KEY

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "kiyoshi.araki@usp.br"
#define USER_PASSWORD "projetomk2022"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "smartcabinet-a190c.appspot.com"

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/Gaveta1/photoesp.jpeg"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

boolean takeNewPhoto = true;

// Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

bool taskCompleted = false;

// Check if photo capture was successful
bool checkPhoto(fs::FS &fs)
{
    File f_pic = fs.open(FILE_PHOTO);
    unsigned int pic_sz = f_pic.size();
    return (pic_sz > 100);
}

void setup()
{
    // Serial port for debugging purposes
    Serial.begin(115200);
    // LCD

    lcd.begin(16, 2);       // SETA A QUANTIDADE DE COLUNAS(16) E O NÚMERO DE LINHAS(2) DO DISPLAY
    lcd.setBacklight(HIGH); // LIGA O BACKLIGHT (LUZ DE FUNDO)

    // MODULO WIFI
    initWiFi();
    initSPIFFS();
    // Turn-off the 'brownout detector'
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    initCamera();

    // Firebase
    //  Assign the api key
    configF.api_key = API_KEY;
    // Assign the user sign in credentials
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    // Assign the callback function for the long running token generation task
    configF.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&configF, &auth);
    Firebase.reconnectWiFi(true);
}

void loop()
{
    // Inicialização
    lcd.setCursor(0, 0);
    lcd.print("Bem Vindo");
    delay(1500);
    lcd.print("Escolha os");
    lcd.setCursor(0, 1);
    lcd.print("remedios no APP");
    // Recebimento dos dados do firebase
    // Gaveta a ser aberta

    // Central de autentificação
    // Digital

    // Autenticação falhou
    lcd.setCursor(0, 0);
    lcd.print("Acesso negado!");

    // Autenticação sucedida
    lcd.setCursor(0, 0);
    lcd.print("Acesso concedido");
    delay(1500);
    lcd.setCursor(0, 0);
    lcd.print("Abra a gaveta: ");
    lcd.setCursor(16, 0);
    lcd.print(gav_input);

    // Abertura das gavetas
    if (liberado)
    {
        if (gav_input)
        {
            digitalWrite(gaveta_1, HIGH);
        }
        else
        {
            digitalWrite(gaveta_2, HIGH);
        }
    }
    delay(3000);

    // Central de gravação
    if (liberado || gaveta_open_1 || gaveta_open_2)
    {
        if (takeNewPhoto)
        {
            capturePhotoSaveSpiffs();
            takeNewPhoto = false;
        }
        delay(1);
        if (Firebase.ready() && !taskCompleted)
        {
            taskCompleted = true;
            Serial.print("Uploading picture... ");

            // MIME type should be valid to avoid the download problem.
            // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
            if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, FILE_PHOTO /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */))
            {
                Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
            }
            else
            {
                Serial.println(fbdo.errorReason());
            }
        }
    }

    // Retravamento
    do
    {
        lcd.setCursor(0, 0);
        lcd.print("Feche as gavetas");
        delay(1000);
        lcd.setCursor(0, 0);
        lcd.print("                ");
    } while (gav_open_1 && gav_open_2)

        digitalWrite(gaveta_1, LOW);
    digitalWrite(gaveta_2, LOW);
}
