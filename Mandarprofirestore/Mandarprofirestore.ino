#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"

/* 1. Define the WiFi credentials */
#define WIFI_SSID "SEMEAR"
#define WIFI_PASSWORD "SemearEhAmor"

/* 2. Define the API Key */
#define API_KEY "AIzaSyCdOSH0AbxaNYZj4NtK6WuL4PVoKrNnwlU"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "smartcabinet-a190c"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "kiyoshi.araki@usp.br"
#define USER_PASSWORD "projetomk2022"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

void setup()
{

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    
    Firebase.reconnectWiFi(true);

}

void loop()
{

    if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    {
        dataMillis = millis();
        String content;
        FirebaseJson js;
        //We will create the nested document in the parent path "a0/b0/c0
        //a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id in the document b0.
        //and d? is the document id in the document collection id c0 which we will create.
        String documentPath = "a0/b0/c0/" + String(count);
        //double
        js.set("fields/myDouble/doubleValue", 123.45678);
        //boolean
        js.set("fields/myBool/booleanValue", true);
        //integer
        js.set("fields/myInteger/integerValue", "911");
        //null
        js.set("fields/myNull/nullValue"); // no value set
        String doc_path = "projects/";
        doc_path += FIREBASE_PROJECT_ID;
        doc_path += "/databases/(default)/documents/coll_id/doc_id"; //coll_id and doc_id are your collection id and document id
        //reference
        js.set("fields/myRef/referenceValue", doc_path.c_str());
        //timestamp
        js.set("fields/myTimestamp/timestampValue", "2014-10-02T15:01:23Z"); //RFC3339 UTC "Zulu" format
        //bytes
        js.set("fields/myBytes/bytesValue", "aGVsbG8="); //base64 encoded
        //array
        js.set("fields/myArray/arrayValue/values/[0]/stringValue", "test");
        js.set("fields/myArray/arrayValue/values/[1]/integerValue", "20");
        js.set("fields/myArray/arrayValue/values/[2]/booleanValue", true);
        //map
        js.set("fields/myMap/mapValue/fields/name/stringValue", "wrench");
        js.set("fields/myMap/mapValue/fields/mass/stringValue", "1.3kg");
        js.set("fields/myMap/mapValue/fields/count/integerValue", "3");
        //lat long
        js.set("fields/myLatLng/geoPointValue/latitude", 1.486284);
        js.set("fields/myLatLng/geoPointValue/longitude", 23.678198);
        js.toString(content);
        count++;

        Serial.print("Create a document... ");

        if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.c_str()))
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        else
            Serial.println(fbdo.errorReason());
    }
}
