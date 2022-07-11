#include "LiquidCrystal_I2C.h"
//Verifica se a digital está cadastrada
  void checkFingerprint(){
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
