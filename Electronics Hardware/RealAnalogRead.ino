//Real Reading of Analog value

#include "driver/adc.h"                         // Use ESP32's built-in ADC functions

void setup() {
  Serial.begin(115200);                         // Start serial connection with computer
  adc1_config_width(ADC_WIDTH_BIT_12);          // Set ADC resolution to 12 bits (0–4095) [web:2]
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // Use ADC1 channel 6 (GPIO34) with 11dB range (~up to 3.3V) [web:2][web:6]
}

void loop() {
  int val = adc1_get_raw(ADC1_CHANNEL_6);       // Read raw ADC value (0–4095) [web:2]

  // Convert raw value to voltage in volts (assuming ~3.3V max range)
  float voltage = (val / 4095.0) * 3.3;         // Scale reading to 0–3.3V range [web:1][web:2]

  Serial.print("Raw: ");
  Serial.print(val);                            // Print raw ADC value
  Serial.print("  Voltage: ");
  Serial.println(voltage);                      // Print calculated voltage in volts

  delay(100);                                   // Wait 100ms before next reading
}
