// ESP-CAM firmware

// Include
#include <WiFi.h>
#include <esp_camera.h>
#include <soc/soc.h>          // Disable brownout problems
#include <soc/rtc_cntl_reg.h> // Disable brownout problems
#include <driver/rtc_io.h>
#include <SPIFFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>   //Provide the token generation process info.}

// Local includes
#include "utils.hpp"

// I/O Pins
#define RX_PIN 5

// Wifi setup
const char *ssid = "SEMEAR";
const char *password = "SemearEhAmor";

// Firebase
// Chave do Firebase
#define API_KEY "AIzaSyCdOSH0AbxaNYZj4NtK6WuL4PVoKrNnwlU" // WEB API KEY
// Email e senha do Firebase
#define USER_EMAIL "kiyoshi.araki@usp.br"
#define USER_PASSWORD "projetomk2022"
// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "smartcabinet-a190c.appspot.com"



// Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

// Variaveis globais do loop
String download_link = "";
// Photo File Name to save in SPIFFS and at Firebase Storage
String spiffs_file = "temp.jpeg";
String remote_path_base = "/Access/access/";
String remote_filename = "";
String remote_path = "";
int photo_counter = 0;
bool take_photo = false;

void setup()
{
    // Inicialização
    delay(1500);
    
    Serial.begin(9600);

    // Turn-off the 'brownout detector'
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    initCamera();

    // MODULO WIFI
    initWiFi();
    initSPIFFS();

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
    // Recebimento dos dados do firebase
    // Gaveta a ser aberta

    if(digitalRead(RX_PIN) == HIGH){
        take_photo = true;
        Serial.println("RX pin set to true. Order to start saving images.");
    }
    else if(digitalRead(RX_PIN) == LOW){
        take_photo = false;
        Serial.println("RX pin set to false. Order to stop saving images.");
    }

    // DO NOT SEND MORE THAN 10 IMAGES (ONLY FOR TESTING)
    if (take_photo && (photo_counter <= 10))
    {
        Serial.println("Capturing photo...");
        capturePhotoSaveSpiffs(spiffs_file);

        if (Firebase.ready())
        {
            // Build path + filename
            remote_path = remote_path_base + String(photo_counter) + ".jpeg";
            Serial.printf("Uploading picture %s... ", remote_filename);

            // MIME type should be valid to avoid the download problem.
            // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
            if (Firebase.Storage.upload(&fbdo,
                                        STORAGE_BUCKET_ID /* Firebase Storage bucket id */,
                                        spiffs_file /* path to local file */,
                                        mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */,
                                        remote_filename /* path of remote file stored in the bucket */,
                                        "image/jpeg" /* mime type */))
            {
                // If upload was ok
                Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
                download_link = fbdo.downloadURL().c_str();

                // Update photo counter
                photo_counter++;
            }
            else
            {
                Serial.println(fbdo.errorReason());
            }
        }
    }

    delay(1);
}
