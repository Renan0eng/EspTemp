//Programa: WifiManager com ESP8266 e ESP32
//Autor: Arduino e Cia

//Carrega as biblioteca de acordo com a placa utilizada

#if defined(ESP8266)
//Bibliotecas ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#else
//Bibliotecas ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#endif

//Bibliotecas Wifi
#include <DNSServer.h>
#include <WiFiManager.h>

// Bibliotecas Json
#include <ArduinoJson.h>

// Bibliotecas Sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Bibliotecas Display
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h> 
#include <SPI.h>

// watchdog
hw_timer_t *timer = NULL; //faz o controle do temporizador (interrupção por tempo)

void IRAM_ATTR resetModule(){
    ets_printf("(watchdog) reiniciar\n"); //imprime no log
    esp_restart(); //reinicia o chip
}

// display
#define TFT_DC 12 //A0
#define TFT_CS 13 //CS
#define TFT_MOSI 14 //SDA
#define TFT_CLK 15 //SCK
#define TFT_RST 0  
#define TFT_MISO 0 

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

// config OneWire sensores
#define    ONE_WIRE_BUS     0

OneWire oneWire(ONE_WIRE_BUS);        
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

float tempMin = 999;   //armazena temperatura mínima
float tempMax = 0;     //armazena temperatura máxima
float tempC = 0;

// void mostra_endereco_sensor(DeviceAddress deviceAddress);  //Função para mostrar endereço do sensor

void setup() {

  timer = timerBegin(0, 80, true); //timerID 0, div 80
  //timer, callback, interrupção de borda
  timerAttachInterrupt(timer, &resetModule, true);
  //timer, tempo (us), repetição
  timerAlarmWrite(timer, 1000000000, true);
  timerAlarmEnable(timer); //habilita a interrupção 

  Serial.begin(115200);
  delay(10);

  // Inicia o sensor
  sensors.begin();

  //Cria um AP (Access Point) com: ("nome da rede", "senha da rede")
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(240);
  if (!wifiManager.autoConnect("ESP", "esp12345")) {
    Serial.println(F("Falha na conexao. Resetar e tentar novamente..."));
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  //Liga o led ao conectar na rede
  digitalWrite(LED_BUILTIN, LOW);

  //Mensagem caso conexao Ok
  Serial.println(F("Conectado na rede Wifi."));
  Serial.print(F("Endereco IP: "));
  Serial.println(WiFi.localIP());

  //Inicia o display
  tft.initR(INITR_BLACKTAB); 

  tft.setTextWrap(false);
  tft.fillScreen(0x408E2F);
  tft.setTextColor(0x5AEB);

  tft.setTextSize(1);
  tft.setCursor(12, 70);
  tft.print("IP: ");
  tft.println(WiFi.localIP());
  delay(5000);
  tft.fillScreen(0x408E2F);
  Serial.print("0");
  xTaskCreatePinnedToCore(
      Dispaly,   //Função que será executada
      "Dispaly", //Nome da tarefa
      10000,      //Tamanho da pilha
      NULL,       //Parâmetro da tarefa (no caso não usamos)
      2,          //Prioridade da tarefa
      NULL,       //Caso queria manter uma referência para a tarefa que vai ser criada (no caso não precisamos)
      0);         //Número do core que será executada a tarefa (usamos o core 0 para o loop ficar livre com o core 1)

}

void loop() {
  timerWrite(timer, 0); //reseta o temporizador (alimenta o watchdog) 
  
  long tme = millis();

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  WiFiClient client;

  HTTPClient http;
  http.begin(client, "http://192.168.1.253:3000/api/esp");
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["time"] = tme;
  doc["temperatura"] = tempC;
  doc["temperatura_esp"] = temperatureRead();
  doc["delayLoop"] = millis() - tme;
  doc["id_esp"] = "0";
  doc["type"] = "CREATE";
  String postData;
  serializeJson(doc, postData);

  int httpResponseCode = http.POST(postData);
  if (httpResponseCode > 0) {
    // Cria um objeto JsonDocument para armazenar a resposta
    DynamicJsonDocument responseDoc(1024);

    // Obtém a resposta do servidor como uma string
    String response = http.getString();

    // // Converte a string em um objeto JSON
    // deserializeJson(responseDoc, response);

    // Imprime o código e a resposta no monitor serial
    // Serial.println(httpResponseCode);
    Serial.println("response", response);

    // // Acessa o campo logic do objeto JSON, que é um objeto aninhado
    // JsonObject logic = responseDoc["logic"].as<JsonObject>();

  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();


  delay(1000 * 60 );
  Serial.print("tempo passado dentro do loop (ms) = ");
  tme = millis() - tme; //calcula o tempo (atual - inicial)
  Serial.println(tme);
}