
#include <Adafruit_Fingerprint.h> //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library

//Senha padrão do sensor de digitais
const uint32_t password = 0x0;

//Objeto responsável por controlar o sensor de digitais
//Utilizamos a Serial2 para comunicação e a senha padrão
//Você também pode utilizar software serial se seu dispositivo não tiver  mais HardwareSerial disponível
//Se não passar o parâmetro de senha ele também considera como senha padrão (0x0)
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&Serial2, password);

void setup()  
{
  Serial.begin(9600);

  //Inicializa o sensor de digitais
  setupFingerprintSensor();
}

void setupFingerprintSensor()
{
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

void loop()
{
  //Exibe o menu no monitor serial
  printMenu();

  //Faz a leitura do comando digitado
  String command = getCommand();
  
  //Transforma a string em inteiro
  int i = command.toInt();

  //Verifica qual o número digitado e chama a função correspondente
  switch (i)
  {
    case 1:
      storeFingerprint();
      break;
    case 2:
      checkFingerprint();
      break;
    case 3:
      printStoredFingerprintsCount();
      break;
    case 4:
      deleteFingerprint();
      break;
    case 5:
      emptyDatabase();
      break;
    default:
      Serial.println(F("Opção inválida"));
      break;
  }

  delay(1000);
}

//Exibe o menu no monitor serial
void printMenu()
{
  Serial.println();
  Serial.println(F("Digite um dos números do menu abaixo"));
  Serial.println(F("1 - Cadastrar digital"));
  Serial.println(F("2 - Verificar digital"));
  Serial.println(F("3 - Mostrar quantidade de digitais cadastradas"));
  Serial.println(F("4 - Apagar digital em uma posição"));
  Serial.println(F("5 - Apagar banco de digitais"));
}

//Espera até que se digite algo no monitor serial e retorna o que foi digitado
String getCommand()
{
  while(!Serial.available()) delay(100);
  return Serial.readStringUntil('\n');
}

//Cadastro da digital
void storeFingerprint()
{
  Serial.println(F("Qual a posição para guardar a digital? (1 a 149)"));

  //Lê o que foi digitado no monitor serial
  String strLocation = getCommand();

  //Transforma em inteiro
  int location = strLocation.toInt();

  //Verifica se a posição é válida ou não
  if(location < 1 || location > 149)
  {
    //Se chegou aqui a posição digitada é inválida, então abortamos os próximos passos
    Serial.println(F("Posição inválida"));
    return;
  }

  Serial.println(F("Encoste o dedo no sensor"));

  //Espera até pegar uma imagem válida da digital
  while (fingerprintSensor.getImage() != FINGERPRINT_OK);
  
  //Converte a imagem para o primeiro padrão
  if (fingerprintSensor.image2Tz(1) != FINGERPRINT_OK)
  {
    //Se chegou aqui deu erro, então abortamos os próximos passos
    Serial.println(F("Erro image2Tz 1"));
    return;
  }
  
  Serial.println(F("Tire o dedo do sensor"));

  delay(2000);

  //Espera até tirar o dedo
  while (fingerprintSensor.getImage() != FINGERPRINT_NOFINGER);

  //Antes de guardar precisamos de outra imagem da mesma digital
  Serial.println(F("Encoste o mesmo dedo no sensor"));

  //Espera até pegar uma imagem válida da digital
  while (fingerprintSensor.getImage() != FINGERPRINT_OK);

  //Converte a imagem para o segundo padrão
  if(fingerprintSensor.image2Tz(2) != FINGERPRINT_OK)
  {
    //Se chegou aqui deu erro, então abortamos os próximos passos
    Serial.println(F("Erro image2Tz 2"));
    return;
  }

  //Cria um modelo da digital a partir dos dois padrões
  if(fingerprintSensor.createModel() != FINGERPRINT_OK)
  {
    //Se chegou aqui deu erro, então abortamos os próximos passos
    Serial.println(F("Erro createModel"));
    return;
  }

  //Guarda o modelo da digital no sensor
  if(fingerprintSensor.storeModel(location) != FINGERPRINT_OK)
  {
    //Se chegou aqui deu erro, então abortamos os próximos passos
    Serial.println(F("Erro storeModel"));
    return;
  }

  //Se chegou aqui significa que todos os passos foram bem sucedidos
  Serial.println(F("Sucesso!!!"));
}

//Verifica se a digital está cadastrada
void checkFingerprint()
{
  Serial.println(F("Encoste o dedo no sensor"));

  //Espera até pegar uma imagem válida da digital
  while (fingerprintSensor.getImage() != FINGERPRINT_OK);

  //Converte a imagem para o padrão que será utilizado para verificar com o banco de digitais
  if (fingerprintSensor.image2Tz() != FINGERPRINT_OK)
  {
    //Se chegou aqui deu erro, então abortamos os próximos passos
    Serial.println(F("Erro image2Tz"));
    return;
  }

  //Procura por este padrão no banco de digitais
  if (fingerprintSensor.fingerFastSearch() != FINGERPRINT_OK)
  {
    //Se chegou aqui significa que a digital não foi encontrada
    Serial.println(F("Digital não encontrada"));
    return;
  }

  //Se chegou aqui a digital foi encontrada
  //Mostramos a posição onde a digital estava salva e a confiança
  //Quanto mais alta a confiança melhor
  Serial.print(F("Digital encontrada com confiança de "));
  Serial.print(fingerprintSensor.confidence);
  Serial.print(F(" na posição "));
  Serial.println(fingerprintSensor.fingerID);
}

void printStoredFingerprintsCount()
{
  //Manda o sensor colocar em "templateCount" a quantidade de digitais salvas
  fingerprintSensor.getTemplateCount();

  //Exibe a quantidade salva
  Serial.print(F("Digitais cadastradas: "));
  Serial.println(fingerprintSensor.templateCount);
}

void deleteFingerprint()
{
  Serial.println(F("Qual a posição para apagar a digital? (1 a 149)"));

  //Lê o que foi digitado no monitor serial
  String strLocation = getCommand();

  //Transforma em inteiro
  int location = strLocation.toInt();

  //Verifica se a posição é válida ou não
  if(location < 1 || location > 149)
  {
    //Se chegou aqui a posição digitada é inválida, então abortamos os próximos passos
    Serial.println(F("Posição inválida"));
    return;
  }

  //Apaga a digital nesta posição
  if(fingerprintSensor.deleteModel(location) != FINGERPRINT_OK)
  {
    Serial.println(F("Erro ao apagar digital"));
  }
  else
  {
    Serial.println(F("Digital apagada com sucesso!!!"));
  }
}

void emptyDatabase()
{
  Serial.println(F("Tem certeza? (s/N)"));

  //Lê o que foi digitado no monitor serial
  String command = getCommand();

  //Coloca tudo em maiúsculo para facilitar a comparação
  command.toUpperCase();

  //Verifica se foi digitado "S" ou "SIM"
  if(command == "S" || command == "SIM")
  {
    Serial.println(F("Apagando banco de digitais..."));

    //Apaga todas as digitais
    if(fingerprintSensor.emptyDatabase() != FINGERPRINT_OK)
    {
      Serial.println(F("Erro ao apagar banco de digitais"));
    }
    else
    {
      Serial.println(F("Banco de digitais apagado com sucesso!!!"));
    }
  }
  else
  {
    Serial.println(F("Cancelado"));
  }
}
