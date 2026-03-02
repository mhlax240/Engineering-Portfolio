#include <Arduino.h>
#include <math.h>
#include <Wire.h>

// ---------------- Pins & options ----------------
const int VIBRA_PIN = 9;

const bool USE_DIGITAL_PULSE_HR = true;
const int HR_PIN_DIGITAL = 2;
const int HR_PIN_ANALOG  = A1;

// Thermistor on A7 (REPLACES MAX30205)
const int ThermistorPin = A7;
int Vo;
float R1 = 10000.0f;   // series resistor 10k
float logR2, R2, T;    // T will be in Fahrenheit after conversion
float c1 = 1.009249522e-03f;
float c2 = 2.378405444e-04f;
float c3 = 2.019202697e-07f;

// ---------------- Thresholds & timing -----------
const float HR_RISE     = 0.15f;
const float TEMP_DELTA  = 1.0f;                     // °F change for alert band
const unsigned long TEMP_SUSTAIN = 5UL * 60UL * 1000UL;
const float TONE_DELTA  = 0.50f;

const unsigned long SAMPLE_MS = 1000;
const unsigned long CALIB_MS  = 6000;

// Baselines
float hr_x = 70.0f, temp_x = 95.0f, tone_x = 1.0f;
// Current readings
float hr = 0, tempC = 0, toneVal = 0;

bool tempHigh = false;
unsigned long tempStart = 0;
unsigned long lastSample = 0;

// ---------------- Heart-rate code ---------------
volatile unsigned long lastBeatMs = 0;
volatile float rrMsEMA = NAN;

void onBeat() {
  unsigned long now = millis();
  if (now - lastBeatMs < 300) return;  // debounce RR
  unsigned long rr = now - lastBeatMs;
  lastBeatMs = now;
  if (isnan(rrMsEMA)) rrMsEMA = rr;
  else rrMsEMA = 0.7f * rrMsEMA + 0.3f * rr;
}

float bpmFromRR(float rr_ms) {
  if (rr_ms <= 0) return NAN;
  return 60000.0f / rr_ms;
}

float readBpmAnalogECG() {
  static int thresh = 600;
  static bool above = false;
  static unsigned long lastMs = 0;
  static float rrEMA = NAN;
  int s = analogRead(HR_PIN_ANALOG);
  unsigned long now = millis();
  float bpm = NAN;
  if (!above && s >= thresh) {
    if (now - lastMs > 300) {
      unsigned long rr = now - lastMs;
      lastMs = now;
      if (isnan(rrEMA)) rrEMA = rr;
      else rrEMA = 0.7f * rrEMA + 0.3f * rr;
      bpm = bpmFromRR(rrEMA);
    }
    above = true;
  } else if (above && s < thresh - 30) {
    above = false;
  }
  return bpm;
}

float readHeartRateBPM() {
  if (USE_DIGITAL_PULSE_HR) {
    if (lastBeatMs == 0 || (millis() - lastBeatMs) > 3000) return NAN;
    return bpmFromRR(rrMsEMA);
  } else {
    return readBpmAnalogECG();
  }
}

// --------------- Thermistor replacement --------
// Returns temperature in °C 
float readSkinTempC() {
  Vo = analogRead(ThermistorPin);
  if (Vo == 0) return NAN;  // avoid divide-by-zero

  // Compute thermistor resistance
  R2 = R1 * (1023.0f / (float)Vo - 1.0f);
  logR2 = log(R2);

  // Steinhart–Hart (Kelvin)
  T = 1.0f / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2);

  // Convert to °C
  T = T - 273.15f;

  return T;
}

// --------------- Vibration motor ---------------
void vibrate(uint8_t p, uint16_t ms) {
  analogWrite(VIBRA_PIN, p);
  delay(ms);
  analogWrite(VIBRA_PIN, 0);
}

// --------------- Calibration -------------------
void calibrate() {
  double sum_hr = 0, sum_temp = 0;
  unsigned long cnt_hr = 0, cnt_temp = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < CALIB_MS) {
    float hb = readHeartRateBPM();
    if (!isnan(hb)) { sum_hr += hb; cnt_hr++; }

    float tp = readSkinTempC();   // now uses thermistor
    if (!isnan(tp)) { sum_temp += tp; cnt_temp++; }


    delay(100);
  }

  if (cnt_hr == 0) hr_x = 72.0f; else hr_x = sum_hr / cnt_hr;
  if (cnt_temp == 0) temp_x = 34.0f; else temp_x = sum_temp / cnt_temp;

  Serial.print("Baseline: HR~"); Serial.print(hr_x, 1);
  Serial.print("  T~"); Serial.print(temp_x, 2);

}

// --------------- Setup & loop ------------------
void setup() {
  Serial.begin(115200);

  pinMode(VIBRA_PIN, OUTPUT);
  analogWrite(VIBRA_PIN, 0);

  // Thermistor is analog only
  pinMode(ThermistorPin, INPUT);

  if (USE_DIGITAL_PULSE_HR) {
    pinMode(HR_PIN_DIGITAL, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(HR_PIN_DIGITAL), onBeat, RISING);
  } else {
    pinMode(HR_PIN_ANALOG, INPUT);
  }


  delay(500);
  calibrate();
  lastSample = millis();
}

void loop() {
  unsigned long now = millis();
  if (now - lastSample < SAMPLE_MS) return;
  lastSample = now;

  hr      = readHeartRateBPM();
  tempC   = readSkinTempC();       // thermistor-based °C
 

  // Temperature alert logic (still in °C)
  bool above = (!isnan(tempC) && (tempC >= temp_x + TEMP_DELTA));
  if (above) {
    if (!tempHigh) { tempHigh = true; tempStart = now; }
  } else {
    tempHigh = false;
  }
  bool tempAlert = tempHigh && (now - tempStart >= TEMP_SUSTAIN);

  bool hrAlert   = (!isnan(hr) && (hr > hr_x * (1.0f + HR_RISE)));

  Serial.print("HR=");  if (isnan(hr)) Serial.print("--"); else Serial.print(hr, 1);
  Serial.print("  T="); if (isnan(tempC)) Serial.print("--"); else Serial.print(tempC, 2);
  Serial.print("  -> ");

  if (hrAlert || tempAlert) {
    Serial.println("Alert");
    uint8_t duty = 120 + 45 * (hrAlert + tempAlert);
    vibrate(duty, 200);
  } else {
    Serial.println("OK");
  }
}
