#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <DHT.h>

// --- PINES ---
#define PIN_RELAY    18
#define PIN_DHT      26
#define PIN_RAIN     14
#define PIN_LDR      34
#define PIN_NTC      35
#define PIN_SOIL     32
#define PIN_WATER    33

// --- LEDS SEMÁFORO ---
#define LED_ROJO     13
#define LED_NARANJA  12
#define LED_AMARILLO 27
#define LED_VERDE    25

// --- CONFIG WIFI ---
const char* ssid     = "Wokwi-GUEST";
const char* password = "";
const float BETA = 3950; 

DHT dht(PIN_DHT, DHT22);

void setup() {
  Serial.begin(115200);
  delay(500);
  
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_NARANJA, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);

  dht.begin();
  pinMode(PIN_RAIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(100); }
  configTime(-21600, 0, "pool.ntp.org"); 
}

float leerTempSuelo() {
  int val = analogRead(PIN_NTC);
  if (val == 0 || val == 4095) return -999; 
  return 1 / (log(1 / (4095. / val - 1)) / BETA + 1.0 / 298.15) - 273.15;
}

void actualizarSemaforo(int necesidad) {
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_NARANJA, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_VERDE, LOW);
  if (necesidad >= 80) digitalWrite(LED_ROJO, HIGH);
  else if (necesidad >= 50) digitalWrite(LED_NARANJA, HIGH);
  else if (necesidad >= 25) digitalWrite(LED_AMARILLO, HIGH);
  else digitalWrite(LED_VERDE, HIGH);
}

void loop() {
  // 1. OBTENER DATOS
  struct tm ti;
  getLocalTime(&ti);
  char tiempo[20];
  sprintf(tiempo, "%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);

  float aireT = dht.readTemperature();
  float aireH = dht.readHumidity();
  float sueloT = leerTempSuelo();
  bool llueve = digitalRead(PIN_RAIN);
  
  int luz = map(analogRead(PIN_LDR), 0, 4095, 100, 0); 
  int humSuelo = map(analogRead(PIN_SOIL), 0, 4095, 0, 100);
  int tanque = map(analogRead(PIN_WATER), 0, 4095, 0, 100);

  int necesidad = 100 - humSuelo;
  actualizarSemaforo(necesidad);

  // 2. LÓGICA DE DECISIÓN (Tu criterio solicitado)
  bool riego = false;
  String razon = "";

  if (llueve) {
    riego = false; 
    razon = "Lluvia detectada";
  } 
  else if (tanque < 10) {
    riego = false; 
    razon = "Tanque vacio";
  }
  else if (aireT < 10) {
    // Caso: Frio extremo (Proteccion)
    riego = false;
    razon = "Temp < 10C (Frio)";
  }
  else if (aireT > 35 && humSuelo < 50) {
    // Caso: Calor extremo (Auxilio termico)
    riego = true;
    razon = "Temp > 35C + Hum < 50%";
  }
  else if (humSuelo < 30) {
    // Caso: Riego estandar por sequedad
    riego = true;
    razon = "Suelo seco (<30%)";
  }
  else {
    riego = false;
    razon = "Humedad correcta";
  }

  digitalWrite(PIN_RELAY, riego ? HIGH : LOW);

  // 3. SALIDA PLANA (SENSOR, VARIABLE, VALOR)
  Serial.println("---------------------------");
  Serial.print("Sistema, Hora, "); Serial.println(tiempo);
  Serial.print("Ambiente, Temperatura, "); Serial.print(aireT); Serial.println(" C");
  Serial.print("Ambiente, Humedad, "); Serial.print(aireH); Serial.println(" %");
  Serial.print("Ambiente, Lluvia, "); Serial.println(llueve ? "SI" : "NO");
  Serial.print("Ambiente, Luz, "); Serial.print(luz); Serial.println(" %");
  Serial.print("Suelo, Temperatura, "); Serial.print(sueloT); Serial.println(" C");
  Serial.print("Suelo, Humedad, "); Serial.print(humSuelo); Serial.println(" %");
  Serial.print("Sistema, Tanque, "); Serial.print(tanque); Serial.println(" %");
  Serial.print("Calculo, Necesidad, "); Serial.print(necesidad); Serial.println(" %");
  Serial.print("Decision, Riego, "); Serial.println(riego ? "SI" : "NO");
  Serial.print("Decision, Razon, "); Serial.println(razon);

  delay(2000); // Actualiza cada 2 segundos
}