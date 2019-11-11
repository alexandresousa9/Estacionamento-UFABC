/*
   Curso: Engenharia Unificada II da UFABC
   
   Controle Inteligente do Estacionamento da UFABC
   
   Autor: Alexandre de Sousa Santos  Data: Novembro de 2019
*/

/* Bibliotecas auxiliares */
#include <ArduinoJson.h>
#include <UIPEthernet.h>
#include <MFRC522.h>
#include <SPI.h>
#include <TimerOne.h>


// --- Constantes do Programa ---
#define VAGAS_ESTUDANTES 100 
#define VAGAS_SERVIDORES 200
#define SS_PIN 5
#define RST_PIN 9
#define ETH_PIN 10
#define RFID_PIN 8

// --- Variáveis Globais ---
short before;
short vagas_livres_estudante;
short vagas_livres_servidor;;

// --- Mapeamento de Hardware ---
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Cria instância com MFRC522
  
// --- Funções utilizadas no programa --- //
void registra_usuario(EthernetClient client, String RFID);
void contador_time();
void print_categoria(char* categoria);
void print_permissao(char* permissao);

void registra_usuario(EthernetClient client, String RFID){

  digitalWrite(SS_PIN, HIGH);
  digitalWrite(ETH_PIN, LOW);
  
  // Send HTTP request
  RFID = RFID.substring(0, 2) + "%20" + RFID.substring(3, 5) + "%20" + RFID.substring(6, 8) + "%20" + RFID.substring(9, 11);
  //Serial.println(RFID);
  
  client.print("GET /UFABC/api/v1/estacionamento/arduino");
  client.print("?cod_RFID=");
  client.print(RFID);
  client.println("&portaria=2 HTTP/1.0");
  client.println(F("Host:34.95.241.108"));
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

  //char status2[100] = {0};
  //client.readBytesUntil('}', status2, sizeof(status2));
  //Serial.println(status2);

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }
  
  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(5) + 50;
  DynamicJsonBuffer jsonBuffer(capacity);

  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(client);
  if (!root.success()) {
    Serial.println(F("Parsing failed!"));
    return;
  }

  //Informações do usuário
  char* codigo = root["cod"];
  char* permissao = root["auth"];
  char* categoria = root["group"];
  short vagas_ocupadas_estudantes = root["amt_a"];
  short vagas_ocupadas_servidores = root["amt_s"];

  if (codigo[0] != '-'){
    
    Serial.print(F("Registro do "));
    print_categoria(categoria);
    Serial.print(F(": "));
    Serial.println(codigo);
  }

  vagas_livres_estudante = VAGAS_ESTUDANTES - vagas_ocupadas_estudantes;
  vagas_livres_servidor = VAGAS_SERVIDORES - vagas_ocupadas_servidores;
  
  Serial.print(F("Quantidade de vagas livres para estudantes: "));
  Serial.println(vagas_livres_estudante);

  Serial.print(F("Quantidade de vagas livres para servidores: "));
  Serial.println(vagas_livres_servidor);
  
  print_permissao(permissao);

  // Disconnect
  client.stop();

  digitalWrite(SS_PIN, LOW);
  digitalWrite(ETH_PIN, HIGH);
}

void print_categoria(char* categoria){
  if (*categoria == '0'){
    Serial.print(F("Aluno"));
  }else if(*categoria == '1'){
    Serial.print(F("Professor"));
  }else if(*categoria == '2'){
    Serial.print(F("Técnico"));
  }else if(*categoria == '3'){
    Serial.print(F("Visitante"));
  }else{
    Serial.print(F("-1"));
  }
}

void print_permissao(char* permissao){
  if (*permissao == '1'){
    Serial.println(F("ENTRADA AUTORIZADA"));
  }else if(*permissao == '2'){
    Serial.println(F("ENTRADA NEGADA : Usuário não possui cadastro no estacionamento da UFABC"));
  }else if(*permissao == '3'){
    Serial.println(F("SAÍDA AUTORIZADA"));
  }else{
    Serial.println(F("Código de permissão não cadastrado"));
  }
}


void setup() {

  //Define a porta do led como saida
  pinMode(RFID_PIN, OUTPUT);
  digitalWrite(RFID_PIN, HIGH);
  
  Serial.begin(9600);   // Inicia comunicação Serial em 9600 baud rate
  SPI.begin();          // Inicia comunicação SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522
  
  Serial.println();
  Serial.println(F("Aproxime o seu cartao do leitor..."));
  
  // Initialize Serial port
  Serial.begin(9600);
  while (!Serial) continue;

  // Initialize Ethernet library
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  
  if (!Ethernet.begin(mac)) {
    Serial.println(F("Failed to configure Ethernet"));
    return;
  }
  
  Timer1.initialize(5000000);
  Timer1.attachInterrupt(contador_time);
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
  String conteudo= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  conteudo.toUpperCase();
  
  if (before > 10){

    //Apaga o LED
    digitalWrite(RFID_PIN, LOW);
    
    before = 0;

    Serial.print(F("Tag RFID lida: "));
    Serial.println(conteudo.substring(1));
    
    Serial.println(F("Connecting..."));
    
    // Connect to HTTP server
    EthernetClient client;
    client.setTimeout(1000);
      
    if (!client.connect("34.95.241.108", 80)) {
      Serial.println(F("Connection failed"));
      return;
    }
    
    Serial.println(F("Connected!"));
    
    registra_usuario(client, conteudo.substring(1));
    Serial.println();
  }
}


void contador_time(){
  before+=12;
  digitalWrite(RFID_PIN, HIGH);
  //Serial.print(F("Time atual: "));
  //Serial.println(before);
}