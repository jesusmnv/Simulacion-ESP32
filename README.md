# 🌱 Sistema de Riego Inteligente IoT con ESP32

Este proyecto implementa un sistema de riego automatizado y autónomo basado en el microcontrolador ESP32. Utiliza múltiples sensores para monitorear las condiciones ambientales y del suelo, tomando decisiones inteligentes para optimizar el uso del agua y proteger el cultivo.

## 🚀 Características Principales

* **Monitoreo Integral:** Lectura de Temperatura/Humedad (Aire), Temperatura/Humedad (Suelo), Luz Solar, Lluvia y Nivel de Tanque.
* **Lógica Avanzada:** Algoritmo de decisión que evalúa:
    * Protección contra heladas (<10°C).
    * Auxilio térmico por calor extremo (>35°C).
    * Prevención de riego con lluvia o tanque vacío.
* **Dashboard en Tiempo Real:** Salida de datos limpia y estructurada por puerto serie (formato CSV-friendly).
* **Indicadores Visuales:** Semáforo de estado (LEDs) para indicar la urgencia del riego.
* **Sincronización Horaria:** Obtención de fecha y hora real vía NTP (WiFi).
* **Simulación Lista:** Configurado para funcionar inmediatamente con **Wokwi** en VS Code.

---

## 🛠️ Requisitos de Hardware

Si deseas montar este proyecto físicamente, necesitarás:

* 1x Placa ESP32 (DevKit V1).
* 1x Sensor DHT22 (Temperatura y Humedad Aire).
* 1x LDR (Fotoresistencia) + Resistencia 10kΩ.
* 1x Termistor NTC 10k + Resistencia 10kΩ.
* 2x Potenciómetros (Simulan Humedad de Suelo y Nivel de Agua).
* 1x Interruptor Deslizante (Simula Sensor de Lluvia).
* 1x Módulo de Relevador (5V/3.3V).
* 4x LEDs (Rojo, Naranja, Amarillo, Verde) + Resistencias 220Ω.
* Cables de conexión (Jumpers) y Protoboard.

---

## 💻 Requisitos de Software

1.  **Visual Studio Code (VS Code):** Editor de código.
2.  **Extensión PlatformIO IDE:** Para gestionar las librerías y la compilación del ESP32.
3.  **Extensión Wokwi Simulator:** Para simular el circuito sin hardware físico.

---

## ⚙️ Instalación y Configuración

Sigue estos pasos para ejecutar el proyecto en tu computadora:

1.  **Clonar el Repositorio:**
    ```bash
    git clone [https://github.com/TU_USUARIO/TU_REPOSITORIO.git](https://github.com/TU_USUARIO/TU_REPOSITORIO.git)
    ```
2.  **Abrir en VS Code:**
    Abre la carpeta del proyecto clonado en Visual Studio Code.
3.  **Instalar Dependencias:**
    PlatformIO detectará automáticamente el archivo `platformio.ini` e instalará las librerías necesarias (`Adafruit DHT`, `Adafruit Unified Sensor`).
4.  **Iniciar Simulación:**
    * Abre el archivo `diagram.json`.
    * Presiona `F1` y selecciona **"Wokwi: Start Simulator"**.

---

## 📊 Guía de Pines (Pinout)

| Sensor / Actuador | Pin ESP32 | Notas |
| :--- | :--- | :--- |
| **DHT22 (Aire)** | GPIO 26 | Digital |
| **Sensor Lluvia** | GPIO 14 | Digital |
| **Relevador (Válvula)** | GPIO 18 | Salida Digital |
| **LDR (Luz)** | GPIO 34 | Entrada Analógica (Input Only) |
| **NTC (Temp Suelo)** | GPIO 35 | Entrada Analógica (Input Only) |
| **Humedad Suelo** | GPIO 32 | Entrada Analógica |
| **Nivel Tanque** | GPIO 33 | Entrada Analógica |
| **LED Rojo (Crítico)** | GPIO 13 | Salida |
| **LED Naranja (Alto)** | GPIO 12 | Salida |
| **LED Amarillo (Medio)**| GPIO 27 | Salida |
| **LED Verde (Bajo)** | GPIO 25 | Salida |

---

## 🚦 Interpretación del Sistema

### Semáforo de Necesidad de Riego
El sistema calcula qué tanta "sed" tiene el cultivo basándose en la humedad del suelo:

* 🔴 **ROJO:** Necesidad Crítica (>= 80%) -> Suelo muy seco.
* 🟠 **NARANJA:** Necesidad Alta (50% - 79%).
* 🟡 **AMARILLO:** Necesidad Media (25% - 49%).
* 🟢 **VERDE:** Necesidad Baja (< 25%) -> Suelo húmedo.

### Dashboard (Monitor Serie)
El sistema imprime un reporte cada 2 segundos con el siguiente formato simplificado para fácil lectura:

```text
---------------------------
Sistema, Hora, 14:30:05
Ambiente, Temperatura, 38.00 C
Ambiente, Humedad, 40.00 %
...
Decision, Riego, SI
Decision, Razon, Temp > 35C + Hum < 50%
```
* const int HUMEDAD_MINIMA = 30;  // Umbral para activar riego
* const int LUZ_MAXIMA = 40;      // Umbral para evitar riego con mucho sol
* const int TANQUE_MINIMO = 10;   // Protección de bomba