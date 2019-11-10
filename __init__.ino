/*
   Curso Engenharia Unificada II da UFABC
   
   Controle Inteligento do Estacionamento da UFABC
   
   Autor: Alexandre de Sousa Santos  Data: Novembro de 2019
*/

/* Bibliotecas auxiliares */
#include <ArduinoJson.h>
#include <UIPEthernet.h>
#include <MFRC522.h>
#include <SPI.h>


// --- Mapeamento de Hardware ---
#define SS_PIN 5
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Cria instância com MFRC522

// --- Variáveis Globais --- 
char st[20];

void connect_API(){
  Serial.println(F("Connecting..."));

  // Connect to HTTP server
  EthernetClient client;
  client.setTimeout(1000);
  
  if (!client.connect("34.95.241.108", 80)) {
    Serial.println(F("Connection failed"));
    return;
  }

  Serial.println(F("Connected!"));

  //if (client.available()) {
  //  char c = client.read();
  //  Serial.print(c);
  //}else{
  //  Serial.print(F("We don't connection avaliable!"));
  //}
  
  // Send HTTP request
  client.print("GET /UFABC/api/v1/estacionamento/vagas");
  //client.print("46%2085%20AF%20A9");
  client.println(" HTTP/1.0");
  client.println(F("Host: 34.95.241.108"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  // Utilizado em testes
  //char c = client.read();
  //c = client.read();
  //Serial.print(c);
  
  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(20) + 60;
  //const size_t capacity = JSON_OBJECT_SIZE(5);
  DynamicJsonBuffer jsonBuffer(capacity);

  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(client);
  if (!root.success()) {
    Serial.println(F("Parsing failed!"));
    return;
  }

  // Extract values
  //Serial.println(F("Response:"));
  //Serial.println(root["cod_rfid"].as<char*>());
  //Serial.println(root["cod_usuario"].as<char*>());
  //Serial.println(root["nome"].as<char*>());
  //Serial.println(root["descricao"].as<char*>());
  //Serial.println(root["data"][1].as<char*>());

  //Vagas ocupadas
  Serial.println(F("Response:"));
  Serial.println(root["Retorno"].as<char*>());
  Serial.println(root["Status"].as<char*>());
  Serial.println(root["Vagas_Ocupadas"].as<char*>());
  //Serial.println(root["descricao"].as<char*>());
  //Serial.println(root["data"][1].as<char*>());

  // Disconnect
  client.stop();
}

void setup() {

  Serial.begin(9600);   // Inicia comunicação Serial em 9600 baud rate
  SPI.begin();          // Inicia comunicação SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522
  
  Serial.println("Aproxime o seu cartao do leitor...");
  Serial.println();
  
  digitalWrite(5, HIGH);
  
  // Initialize Serial port
  Serial.begin(9600);
  while (!Serial) continue;
  
  // Initialize Ethernet library
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  
  if (!Ethernet.begin(mac)) {
    Serial.println(F("Failed to configure Ethernet"));
    return;
  }
  delay(1000);

  connect_API();

}

void loop() {

  // Verifica novos cartões
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Seleciona um dos cartões
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  
  // Mostra UID na serial
  Serial.print("UID da tag :");
  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase();
  
  if (conteudo.substring(1) == "91 7D FC 2F") //UID 1 - Chaveiro
  {
    Serial.println("Chaveiro identificado!");
    Serial.println();
    delay(3000);
     
  }
 
  if (conteudo.substring(1) == "54 DB 03 C5") //UID 2 - Cartao
  {
    Serial.println("Cartao identificado");
    Serial.println();
    delay(3000);
  }
}