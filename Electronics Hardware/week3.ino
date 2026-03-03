// --------------------------
// ESP32 Raw ADC + Circular Buffer Example
// --------------------------

#include "driver/adc.h"

#define ADC_PIN ADC1_CHANNEL_6  // GPIO34, ADC1 channel 6
#define BUF_SIZE 100            // Circular buffer size

float buffer[BUF_SIZE];
float *ptr = buffer;

const float V_REF = 3.3;       // Reference voltage
const float ADC_RES = 4095.0;  // 12-bit ADC

void setup() {
  Serial.begin(115200);

  // Configure ADC
  adc1_config_width(ADC_WIDTH_BIT_12);       // 12-bit resolution
  adc1_config_channel_atten(ADC_PIN, ADC_ATTEN_DB_11); // 0-3.6 V range
}

void loop() {
  // 1️⃣ Read raw ADC count
  int raw = adc1_get_raw(ADC_PIN);

  // 2️⃣ Convert to voltage
  float voltage = raw * (V_REF / ADC_RES);

  // 3️⃣ Store in circular buffer using pointer
  *ptr = voltage;
  ptr++;
  if(ptr >= buffer + BUF_SIZE) ptr = buffer; // wrap-around

  // 4️⃣ Print voltage for Serial Plotter
  Serial.println(voltage, 3);

  delay(100); // ~1 kHz sampling
}
