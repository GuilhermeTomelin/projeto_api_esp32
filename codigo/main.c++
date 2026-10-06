#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// =====================================================
// WIFI
// =====================================================

const char* WIFI_SSID = "Tomelin_";
const char* WIFI_PASSWORD = "1hora=50pila";


// =====================================================
// AWESOME API
// =====================================================

const char* API_URL =
  "https://economia.awesomeapi.com.br/json/last/USD-BRL";


// =====================================================
// OLED 128x32 I2C
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_SDA 21
#define OLED_SCL 22

#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);


// =====================================================
// INTERVALO
// =====================================================

unsigned long ultimaConsulta = 0;

const unsigned long intervaloConsulta = 10000;


// =====================================================
// INICIAR OLED
// =====================================================

void iniciarOLED() {

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS
      )) {

    Serial.println("ERRO: OLED nao encontrado!");

    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("ESP32 + AwesomeAPI");

  display.println();

  display.println("Inicializando...");

  display.display();

  Serial.println("OLED inicializado!");
}


// =====================================================
// CONECTAR WIFI
// =====================================================

void conectarWiFi() {

  Serial.println();
  Serial.println("Conectando ao WiFi...");

  display.clearDisplay();

  display.setCursor(0, 0);

  display.println("Conectando WiFi...");

  display.display();


  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  int tentativas = 0;


  while (
    WiFi.status() != WL_CONNECTED &&
    tentativas < 30
  ) {

    delay(500);

    Serial.print(".");

    tentativas++;
  }


  Serial.println();


  if (
    WiFi.status() == WL_CONNECTED
  ) {
    Serial.println("WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(
      WiFi.localIP()
    );
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi conectado!");
    display.setCursor(0, 16);
    display.println(
      WiFi.localIP()
    );
    display.display();
    delay(1500);
  }
  else {
    Serial.println(
      "Erro ao conectar WiFi!"
    );
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ERRO WIFI");
    display.setCursor(0, 16);
    display.println("Verifique rede");
    display.display();
    delay(3000);
  }
}
// =====================================================
// CONSULTAR AWESOME API
// =====================================================
void consultarCotacao() {
  if (
    WiFi.status() != WL_CONNECTED
  ) {
    conectarWiFi();
    return;
  }
  Serial.println();
  Serial.println("Consultando AwesomeAPI...");
  HTTPClient http;
  http.begin(API_URL);
  int codigoHTTP = http.GET();
  Serial.print("HTTP: ");
  Serial.println(codigoHTTP);
  if (codigoHTTP == 200) {
    String resposta =
      http.getString();
    JsonDocument doc;
    DeserializationError erro =
      deserializeJson(
        doc,
        resposta
      );
    if (erro) {
      Serial.println(
        "Erro ao interpretar JSON!"
      );
      http.end();
      return;
    }
    // =================================================
    // DADOS DA API
    // =================================================
    float compra =
      doc["USDBRL"]["bid"];
    float venda =
      doc["USDBRL"]["ask"];
    float variacao =
      doc["USDBRL"]["pctChange"];
    // =================================================
    // SERIAL
    // =================================================
    Serial.println();
    Serial.println("==============================");
    Serial.print("Compra: R$ ");
    Serial.println(compra, 2);
    Serial.print("Venda: R$ ");
    Serial.println(venda, 2);
    Serial.print("Variacao: ");
    Serial.print(variacao, 2);
    Serial.println("%");
    Serial.println("==============================");
    // =================================================
    // OLED 128x32
    // =================================================
    display.clearDisplay();
    display.setTextColor(
     SSD1306_WHITE
    );
    display.setTextSize(1);
    // Linha 1
    display.setCursor(0, 0);
    display.println("USD/BRL");
    // Linha 2
    display.setCursor(0, 10);
    display.print("C:");
    display.print(compra, 2);
    display.print(" V:");
    display.print(venda, 2);
    // Linha 3
    display.setCursor(0, 20);
    display.print("Var: ");
    display.print(variacao, 2);
    display.print("%");
    display.display();
  }
  else {
    Serial.print(
      "Erro HTTP: "
    );
    Serial.println(
      codigoHTTP
    )
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ERRO API");
    display.setCursor(0, 16);
    display.print("HTTP: ");
    display.println(codigoHTTP);
    display.display();
  }
  http.end();
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" ESP32 + AWESOME API");
  Serial.println(" OLED 128x32");
  Serial.println("==============================");

  iniciarOLED();
  conectarWiFi();
  consultarCotacao();
  ultimaConsulta = millis();
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  unsigned long agora =
    millis();
  if (
    agora - ultimaConsulta >=
    intervaloConsulta
  ) {
    ultimaConsulta = agora;
    consultarCotacao();
  }
} 
