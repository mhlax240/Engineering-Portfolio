// Fake Narcolepsy Dataset Test

const int vibPin = 3;       
const int dataLength = 70;     
float hrData[dataLength];
float tempData[dataLength];

const int historySize = 35;
float pastHRvalues[historySize];
float pastTemp[historySize];
int historyCount = 0;        
int historyHead = 0;          

const int baselineCountTarget = 20;
int baselineCount = 0;
float baselineHRsum = 0.0;
float baselineTempsum = 0.0;
float baselineHRavg = 0.0;
float baselineTempavg = 0.0;
bool baselineReady = false;

int dataIndex = 0;

void setupDatasets() {
  for (int i = 0; i < 60; i++) {
    hrData[i] = 70.0;
    tempData[i] = 98.6;
  }

  for (int j = 0; j < 5; j++) {
    int idx = 60 + j;
    float frac = (j + 1) / 5.0;
    hrData[idx] = 70.0 + (88.0 - 70.0) * frac;
    tempData[idx] = 98.6 + (99.5 - 98.6) * frac;
  }

  for (int i = 65; i < 70; i++) {
    hrData[i] = 88.0;
    tempData[i] = 99.5;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(vibPin, OUTPUT);
  digitalWrite(vibPin, LOW);

  setupDatasets();

  Serial.println("Starting fake HR/Temp dataset replay...");
}

void addToHistory(float hr, float temp) {
  pastHRvalues[historyHead] = hr;
  pastTemp[historyHead] = temp;

  if (historyCount < historySize) {
    historyCount++;
  }

  historyHead++;
  if (historyHead >= historySize) {
    historyHead = 0;
  }
}

float averageLastN(float *buffer, int n) {
  if (historyCount < n) {
    return 0.0;
  }

  float sum = 0.0;
  for (int i = 0; i < n; i++) {
    int idx = historyHead - 1 - i;
    if (idx < 0) {
      idx += historySize;
    }
    sum += buffer[idx];
  }
  return sum / n;
}

void loop() {
  if (dataIndex >= dataLength) {
    return;
  }

  float currentHR = hrData[dataIndex];
  float currentTemp = tempData[dataIndex];
  dataIndex++;

  if (!baselineReady) {
    if (baselineCount < baselineCountTarget) {
      baselineHRsum += currentHR;
      baselineTempsum += currentTemp;
      baselineCount++;

      if (baselineCount == baselineCountTarget) {
        baselineHRavg = baselineHRsum / baselineCountTarget;
        baselineTempavg = baselineTempsum / baselineCountTarget;
        baselineReady = true;

        Serial.print("Baseline HR avg (first 20): ");
        Serial.println(baselineHRavg);
        Serial.print("Baseline Temp avg (first 20): ");
        Serial.println(baselineTempavg);
      }
    }
  }

  addToHistory(currentHR, currentTemp);

  float avgHR5 = 0.0;
  float avgTemp5 = 0.0;
  bool haveLast5 = (historyCount >= 5);
  bool alert = false;

  if (baselineReady && haveLast5) {
    avgHR5 = averageLastN(pastHRvalues, 5);
    avgTemp5 = averageLastN(pastTemp, 5);

    bool hrCondition = (avgHR5 >= baselineHRavg * 1.15);
    bool tempCondition = (avgTemp5 >= baselineTempavg + 0.5);

    alert = hrCondition && tempCondition;
  }

  if (alert) {
    digitalWrite(vibPin, HIGH);
  } else {
    digitalWrite(vibPin, LOW);
  }

  Serial.print("HR: ");
  Serial.print(currentHR);
  Serial.print(" bpm");

  if (baselineReady && haveLast5) {
    Serial.print(" (avg last 5: ");
    Serial.print(avgHR5);
    Serial.print(")");
  } else {
    Serial.print(" (avg last 5: N/A)");
  }

  Serial.print("  |  Temp: ");
  Serial.print(currentTemp);
  Serial.print(" F");

  if (baselineReady && haveLast5) {
    Serial.print(" (avg last 5: ");
    Serial.print(avgTemp5);
    Serial.print(")");
  } else {
    Serial.print(" (avg last 5: N/A)");
  }

  Serial.print("  |  Vibration: ");
  if (alert) {
    Serial.print("ON  ");
    Serial.print("ALERT ALERT ALERT VIBRATION ON");
  } else {
    Serial.print("OFF");
  }

  Serial.println();

  delay(1000);
}