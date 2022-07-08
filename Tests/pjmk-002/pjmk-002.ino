#include <WiFi.h>         
#include <IOXhop_FirebaseESP32.h>                           
#include <ArduinoJson.h>                   

#define WIFI_SSID "Galaxy A7"                   
#define WIFI_PASSWORD "lari080803"         
#define FIREBASE_HOST "https://smartcabinet-a190c-default-rtdb.firebaseio.com/"    
#define FIREBASE_AUTH "agQbiq7CmdPzQ1o7XAoQfkfyMJE9PtUGaqSJk4Vg"   

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando ao wifi");

  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }

  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  Firebase.setString("/gaveta1/operador", "Kiyoshi");
  Firebase.setInt("/gaveta1/horario", 15);
  Firebase.setBool("/gaveta1/ocupado", true);
  Serial.println("Enviado /gaveta1/operador = Kiyoshi, /gaveta1/horario = 15, /gaveta1/ocupado = true");
  delay(3000);
  Firebase.setString("/gaveta2/operador", "Guilherme");
  Firebase.setInt("/gaveta2/horario", 16);
  Firebase.setBool("/gaveta2/ocupado", true);
  Serial.println("Enviado /gaveta2/operador = Guilherme, /gaveta2/horario = 16, /gaveta2/ocupado = true");
  delay(3000);
}
