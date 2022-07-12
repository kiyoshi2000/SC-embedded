// MAIN

// Include
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

#include <Firebase_ESP_Client.h>
#include <Adafruit_Fingerprint.h> //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library

// Configurações do WIFI
const char *ssid = "Backupnet";
const char *password = "lanevoxje";

// Firebase
// Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "smartcabinet-a190c"

// Chave do Firebase
#define API_KEY "AIzaSyCdOSH0AbxaNYZj4NtK6WuL4PVoKrNnwlU" // WEB API KEY

// Email e senha do Firebase
#define USER_EMAIL "kiyoshi.araki@usp.br"
#define USER_PASSWORD "projetomk2022"

// LCD
#define BACKLIGHT_PIN 13
LiquidCrystal_I2C lcd(0x3F, 16, 2); // Set the LCD I2C address
// LiquidCrystal_I2C lcd(0x38, BACKLIGHT_PIN, POSITIVE);  // Set the LCD I2C address

// Digital
// SoftwareSerial mySerial(2, 3);
const uint32_t password_dig = 0x0;
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&Serial2, password_dig);

// AQUI
void initWiFi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Conectando ao WiFi...");

    lcd.setCursor(0,0);
    lcd.print("Conectando ");
    lcd.setCursor(0,1);
    lcd.print("ao WiFi...");
  }
  
  Serial.println("Conectando ao WiFi: OK!");
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Conectado! ");
}

void setupFingerprintSensor()
{
  // Inicializa o sensor
  fingerprintSensor.begin(57600);

  // Verifica se a senha está correta
  if (!fingerprintSensor.verifyPassword())
  {
    // Se chegou aqui significa que a senha está errada ou o sensor está problemas de conexão
    Serial.println(F("Não foi possível conectar ao sensor. Verifique a senha ou a conexão"));
    // while (true){
    //   lcd.setCursor(0,0);
    //   lcd.print("Erro ao conector com sensor biometrico");
    // }
  }
}

// Variaveis
// global de erro da digital
int erro = 0;
String Operador;

// Variaveis globais do loop
int gav_input = 0;
int sai = 1;
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
int Ano = 0;
int Mes = 0;
int Dia = 0;
String Data = "00/00/00";
int Contador_Comando = 1;
int erro_Comando = 1;
// PORTAS
int gaveta_1 = 15;
int gaveta_2 = 2;
int gav_open_1 = 4;
int gav_open_2 = 5;
int arrombamento = 50;
int grava = 18;
int Comando = 23;

void setup()
{
  Serial.begin(9600);

  // LCD
  lcd.init();
  lcd.begin(16, 2);       // SETA A QUANTIDADE DE COLUNAS(16) E O NÚMERO DE LINHAS(2) DO DISPLAY
  lcd.setBacklight(HIGH); // LIGA O BACKLIGHT (LUZ DE FUNDO)
  
  // Inicialização
  lcd.setCursor(0, 0);
  lcd.print("  Bem Vindo   ");
  delay(1500);

  // MODULO WIFI
  initWiFi();

  // Firebase
  //  Assign the api key
  configF.api_key = API_KEY;
  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.RTDB.enableClassicRequest(&fbdo, true);

  // Inicializa o sensor de digitais
  setupFingerprintSensor();

  // Portas                  //definir as portas corretas da esp32cam
  pinMode(gaveta_1, OUTPUT); // Reles
  pinMode(gaveta_2, OUTPUT);
  pinMode(gav_open_1, INPUT); // Sensores
  pinMode(gav_open_2, INPUT);
  pinMode(arrombamento, OUTPUT);
  pinMode(grava, OUTPUT);
  pinMode(Comando, OUTPUT);

  //Serial.println("Main loop started!");
}

void loop()
{
  while (digitalRead(Comando) && erro_Comando)
  {

    // Transforma em inteiro
    int location = Contador_Comando; // nao pode passar de 149!!!

    Serial.println(F("Encoste o dedo no sensor"));
    lcd.setCursor(0, 0);
    lcd.print("Coloque o dedo    ");
    // Espera até pegar uma imagem válida da digital
    while (fingerprintSensor.getImage() != FINGERPRINT_OK)
      ;

    // Converte a imagem para o primeiro padrão
    if (fingerprintSensor.image2Tz(1) != FINGERPRINT_OK)
    {
      // Se chegou aqui deu erro, então abortamos os próximos passos
      Serial.println(F("Erro image2Tz 1"));
      lcd.setCursor(0, 0);
      lcd.print("ERRO           ");
      erro_Comando = 0;
    }
    if (erro_Comando)
    {
      Serial.println(F("Tire o dedo do sensor"));
      lcd.setCursor(0, 0);
      lcd.print("Retire o dedo  ");
      delay(2000);

      // Espera até tirar o dedo
      while (fingerprintSensor.getImage() != FINGERPRINT_NOFINGER)
        ;

      // Antes de guardar precisamos de outra imagem da mesma digital
      Serial.println(F("Encoste o mesmo dedo no sensor"));
      lcd.setCursor(0, 0);
      lcd.print("Encoste o dedo   ");
      lcd.setCursor(0, 1);
      lcd.print("Novamente     ");

      // Espera até pegar uma imagem válida da digital
      while (fingerprintSensor.getImage() != FINGERPRINT_OK)
        ;

      // Converte a imagem para o segundo padrão
      if (fingerprintSensor.image2Tz(2) != FINGERPRINT_OK)
      {
        // Se chegou aqui deu erro, então abortamos os próximos passos
        Serial.println(F("Erro image2Tz 2"));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ERRO");
        erro_Comando = 0;
      }

      // Cria um modelo da digital a partir dos dois padrões
      if (erro_Comando == 1)
      {
        if (fingerprintSensor.createModel() != FINGERPRINT_OK)
        {
          // Se chegou aqui deu erro, então abortamos os próximos passos
          Serial.println(F("Erro createModel"));
          lcd.setCursor(0, 0);
          lcd.print("ERRO           ");
          erro_Comando = 0;
        }
      }

      // Guarda o modelo da digital no sensor
      if (erro_Comando == 1)
      {
        if (fingerprintSensor.storeModel(location) != FINGERPRINT_OK)
        {
          // Se chegou aqui deu erro, então abortamos os próximos passos
          Serial.println(F("Erro storeModel"));
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("ERRO           ");
          erro_Comando = 0;
        }
      }
      // Se chegou aqui significa que todos os passos foram bem sucedidos
      Serial.println(F("Sucesso!!!"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sucesso!!!");
      Contador_Comando++;
    }
  }

  // Recebimento dos dados do firebase
  // Gaveta a ser aberta
  sai = 1;
  while (sai)
  {
    Segundo = Segundo + (millis() - tempo) / 1000;
    tempo = millis();
    if (Segundo > 60)
    {
      Segundo = Segundo - 60;
      Minuto++;
    }
    if (Minuto > 60)
    {
      Minuto = Minuto - 60;
      Hora++;
    }
    if (Hora == 24)
    {
      Dia++;
      Hora = 0;
    }
    if (Dia == 31)
    {
      Mes++;
      Dia = 1;
    }
    if (Mes == 13)
    {
      Ano++;
      Mes = 0;
    }

    Horario = String(String(Hora) + ":" + String(Minuto) + ":" + String(Segundo));
    Data = String(String(Dia) + "/" + String(Mes) + "/" + String(Ano));
    // lcd.clear();
    // lcd.setCursor(0, 0);
    // lcd.print("Escolha os");
    // lcd.setCursor(0, 1);
    // lcd.print("remedios no APP");
    bool atualizador;
    Firebase.RTDB.getBool(&fbdo, "Atualizador", &atualizador);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Atualizador: ");
    lcd.setCursor(0, 1);
    lcd.print(String(atualizador).c_str());

    
    if (atualizador == true)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ordem recebida");
      delay(500);

      Firebase.RTDB.getInt(&fbdo, "Gaveta", &gav_input);
      // gav_input = fbdo.to<int>();

      Firebase.RTDB.getInt(&fbdo, "Hora", &Hora);
      // Hora = fbdo.to<int>();

      Firebase.RTDB.getInt(&fbdo, "Minuto", &Minuto);
      // Minuto = fbdo.to<int>();

      Firebase.RTDB.getInt(&fbdo, "Segundo", &Segundo);
      // Segundo = fbdo.to<int>();

      Firebase.RTDB.getString(&fbdo, "Dia", &Dia);
      // Dia = fbdo.to<int>();

      Firebase.RTDB.getString(&fbdo, "Mês", &Mes);
      // Mes = fbdo.to<int>();

      Firebase.RTDB.getInt(&fbdo, "Ano", &Ano);
      // Ano = fbdo.to<int>();

      tempo = millis();
      sai = 0;
    }
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Processando dados...");
  Firebase.RTDB.setBool(&fbdo, "Atualizador", false);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Dados processados!");

  // Central de autentificação
  // Digital
  do
  {
    erro = 0;
    Serial.println(F("Encoste o dedo no sensor"));

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Insira a digital");

    // Espera até pegar uma imagem válida da digital
    while (fingerprintSensor.getImage() != FINGERPRINT_OK);

    // Converte a imagem para o padrão que será utilizado para verificar com o banco de digitais
    if (fingerprintSensor.image2Tz() != FINGERPRINT_OK)
    {
      // Se chegou aqui deu erro, então abortamos os próximos passos
      Serial.println(F("Erro image2Tz"));
      erro = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ERRO");
    }

    // Procura por este padrão no banco de digitais
    if (fingerprintSensor.fingerFastSearch() != FINGERPRINT_OK)
    {
      // Se chegou aqui significa que a digital não foi encontrada
      Serial.println(F("Digital não encontrada"));
      erro = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Digital nao");
      lcd.setCursor(0, 1);
      lcd.print("encontrada");
    }
    if (erro == 0)
    {
      // Se chegou aqui a digital foi encontrada
      // Mostramos a posição onde a digital estava salva e a confiança
      // Quanto mais alta a confiança melhor
      Serial.print(F("Digital encontrada com confiança de "));
      Serial.print(fingerprintSensor.confidence);
      Serial.print(F(" na posição "));
      Serial.println(fingerprintSensor.fingerID);
      Operador = fingerprintSensor.fingerID;
      erro = 0;
      lcd.setCursor(0, 0);
      lcd.print("Acesso");
      lcd.setCursor(0, 1);
      lcd.print("Concedido");
      digitalWrite(arrombamento, LOW);
      digitalWrite(grava, HIGH);
    }
    if ((digitalRead(gav_open_1) == 0 || digitalRead(gav_open_2) == 0) && erro == 1) // arrombamento
    {
      // Avisa a espcam
      erro = 1;
      digitalWrite(arrombamento, HIGH);
      digitalWrite(grava, HIGH);

      Endereco_erro = String(Endereco_erro + String(Contador_erro));

      // Envio de dados Firebase
      FirebaseJson content;
      content.set("Horario", Horario);
      content.set("Data", Data);
      content.set("Pasta", String(Contador_erro));
      String documentPath = Endereco_erro;

      if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
      {
        Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      }
      else
      {
        Serial.println(fbdo.errorReason());
      }

      Contador_erro++;
      Endereco_erro = "Erro/";

      // Alerta
      while (digitalRead(gav_open_1) == 0 || digitalRead(gav_open_2) == 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ARROMBAMENTO");
        delay(500);
        lcd.print("             ");
        delay(500);
      }
      digitalWrite(grava, LOW);
    }
  } while (erro);

  delay(1500);
  lcd.setCursor(0, 0);
  lcd.print("Abra a/s gaveta/s: ");

  if (gav_input = 0)
  {
    lcd.setCursor(16, 0);
    lcd.print("1");
  }
  if (gav_input = 1)
  {
    lcd.setCursor(16, 0);
    lcd.print("2");
  }
  if (gav_input = 2)
  {
    lcd.setCursor(0, 1);
    lcd.print("1 e 2");
  }

  // Abertura das gavetas
  if (erro == 0)
  {
    if (gav_input == 0)
    {
      digitalWrite(gaveta_1, HIGH);
    }
    if (gav_input == 1)
    {
      digitalWrite(gaveta_2, HIGH);
    }
    if (gav_input == 2)
    {
      digitalWrite(gaveta_2, HIGH);
      digitalWrite(gaveta_1, HIGH);
    }
  }

  delay(3000);

  // Retravamento
  do
  {
    lcd.setCursor(0, 0);
    lcd.print("Feche as gavetas");
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("                ");
  } while (digitalRead(gav_open_1) == 0 || digitalRead(gav_open_2) == 0);

  digitalWrite(gaveta_1, LOW);
  digitalWrite(gaveta_2, LOW);
  digitalWrite(grava, LOW);

  Endereco = String(Endereco + String(Contador));

  // Envio de dados Firebase
  FirebaseJson content;
  content.set("Horario", Horario);
  content.set("Data", Data);
  content.set("Medicamento", Firebase.RTDB.getString(&fbdo, "Auxiliary/Acess_Request/Medicamento"));
  content.set("Operador", Operador);
  content.set("Pasta", String(Contador));
  String documentPath = Endereco;

  if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
  {
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  }
  else
  {
    Serial.println(fbdo.errorReason());
  }

  Contador++;
  Endereco = "Gaveta/";
}