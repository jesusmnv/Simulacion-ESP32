#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <DHT.h>

// ==========================================
//        CONFIGURACIÓN DE USUARIO
// ==========================================

// --- Umbrales de Riego ---
const int UMBRAL_HUMEDAD_RIEGO  = 30; // % (Regar si baja de esto)
const int UMBRAL_HUMEDAD_AUX    = 50; // % (Riego de auxilio por calor)

// --- Umbrales Ambientales ---
const int UMBRAL_TEMP_FRIO      = 10; // °C (No regar si hace más frío que esto)
const int UMBRAL_TEMP_CALOR     = 35; // °C (Activar auxilio si supera esto)
const int UMBRAL_LUZ_MAXIMA     = 40; // % (No regar si hay más luz que esto)

// --- Protección Sistema ---
const int UMBRAL_TANQUE_MINIMO  = 10; // % (Apagar bomba si baja de esto)

// --- Configuración WiFi ---
const char* SSID_WIFI     = "Wokwi-GUEST";
const char* PASS_WIFI     = "";
const char* SERVIDOR_NTP  = "pool.ntp.org";
const long  ZONA_HORARIA  = -21600; // GMT-6 (Ajustar segun pais)

// ==========================================
//          DEFINICIÓN DE HARDWARE
// ==========================================

// --- Pines Sensores ---
#define PIN_RELAY    18
#define PIN_DHT      26
#define PIN_RAIN     14
#define PIN_LDR      34
#define PIN_NTC      35
#define PIN_SOIL     32
#define PIN_WATER    33

// --- Pines Semáforo ---
#define LED_ROJO     13
#define LED_NARANJA  12
#define LED_AMARILLO 27
#define LED_VERDE    25

const float BETA_NTC = 3950; 
DHT dht(PIN_DHT, DHT22);

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // Configurar salidas
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_NARANJA, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);

  // Iniciar sensores
  dht.begin();
  pinMode(PIN_RAIN, INPUT);

  // Conectar WiFi
  WiFi.begin(SSID_WIFI, PASS_WIFI);
  while (WiFi.status() != WL_CONNECTED) { delay(100); }
  
  // Configurar hora
  configTime(ZONA_HORARIA, 0, SERVIDOR_NTP); 
}

float leerTempSuelo() {
  int val = analogRead(PIN_NTC);
  if (val == 0 || val == 4095) return -999; 
  return 1 / (log(1 / (4095. / val - 1)) / BETA_NTC + 1.0 / 298.15) - 273.15;
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
  // 1. OBTENER HORA
  struct tm ti;
  getLocalTime(&ti);
  char tiempo[20];
  sprintf(tiempo, "%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);

  // 2. LECTURAS SENSORES
  float aireT = dht.readTemperature();
  float aireH = dht.readHumidity();
  float sueloT = leerTempSuelo();
  bool llueve = digitalRead(PIN_RAIN);
  
  // Mapeos de analógico a porcentaje (0-100%)
  int luz = map(analogRead(PIN_LDR), 0, 4095, 100, 0); 
  int humSuelo = map(analogRead(PIN_SOIL), 0, 4095, 0, 100);
  int tanque = map(analogRead(PIN_WATER), 0, 4095, 0, 100);

  // Calculo necesidad (Inverso a humedad)
  int necesidad = 100 - humSuelo;
  actualizarSemaforo(necesidad);

  // 3. LÓGICA DE DECISIÓN (Usando las constantes)
  bool riego = false;
  String razon = "";

  if (llueve) {
    riego = false; 
    razon = "Lluvia detectada";
  } 
  else if (tanque < UMBRAL_TANQUE_MINIMO) {
    riego = false; 
    razon = "Tanque vacio";
  }
  else if (aireT < UMBRAL_TEMP_FRIO) {
    // Protección contra heladas
    riego = false;
    razon = "Temp < Minima (Frio)";
  }
  else if (aireT > UMBRAL_TEMP_CALOR && humSuelo < UMBRAL_HUMEDAD_AUX) {
    // Auxilio térmico (Calor extremo)
    riego = true;
    razon = "Auxilio Termico (Calor)";
  }
  else if (humSuelo < UMBRAL_HUMEDAD_RIEGO) {
    // Riego estándar
    if (luz < UMBRAL_LUZ_MAXIMA) {
       riego = true; 
       razon = "Suelo Seco + Luz OK";
    } else {
       riego = false;
       razon = "Espera (Mucho Sol)";
    }
  }
  else {
    riego = false;
    razon = "Humedad correcta";
  }

  // 4. ACTUAR
  digitalWrite(PIN_RELAY, riego ? HIGH : LOW);

  // 5. SALIDA DE DATOS (Formato CSV Clean)
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

  delay(2000); 
}