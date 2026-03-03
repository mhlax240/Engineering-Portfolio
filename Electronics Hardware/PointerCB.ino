//Pointer Style Circular Buffer
#include "driver/adc.h"   // ESP32 low-level ADC driver

#define N 10              // Number of samples stored in circular buffer
#define ADC_CHANNEL ADC1_CHANNEL_6   // LoRa32 board pin 6 = GPIO34

// Circular buffer storage (volatile = may change outside normal flow)
volatile int buffer[N];

// Pointer that always points to the next write location
volatile int *writePtr = buffer;

void setup() {
  Serial.begin(115200);   // Start serial output for debugging

  // Configure ADC resolution (12-bit → values 0–4095)
  adc1_config_width(ADC_WIDTH_BIT_12);

  // Set ADC channel range (~0–3.3V input range)
  adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);

  // Initialize buffer to zero using pointer arithmetic
  int *ptr = (int*)buffer;
  for(int i = 0; i < N; i++) {
    *ptr++ = 0;           // Write 0, then move pointer forward
  }
}

void loop() {

  // Store a new ADC reading at the current write location
  *writePtr = adc1_get_raw(ADC_CHANNEL);

  // Move pointer to next buffer position
  writePtr++;

  // If pointer reaches end of buffer, wrap back to start
  if (writePtr == buffer + N) {
    writePtr = buffer;
  }

  // Calculate moving average of all samples
  long sum = 0;
  int *ptr = (int*)buffer;   // Pointer used to walk through buffer

  // Add all buffer values together
  for(int i = 0; i < N; i++) {
    sum += *ptr++;            // Read value and advance pointer
  }

  // Compute average value
  float avg = sum / (float)N;

  // Convert ADC average to voltage (0–3.3V range)
  float voltage = avg * (3.3 / 4095.0);

  // Print filtered voltage reading
  Serial.println(voltage);

  // Wait before taking next sample
  delay(20);
}
