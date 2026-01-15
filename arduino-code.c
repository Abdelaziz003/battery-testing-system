/****************************************************
 * Battery Multi-Channel Discharge Test System
 * -----------------------------------------------
 * - Supports multiple batteries
 * - Timed test or full discharge test
 * - Measures voltage, shunt voltage, current, capacity
 * - Controlled via Serial commands
 ****************************************************/

const int NUM_BATTERIES = 2;

/****************************************************
 * Pin mapping structure for each battery
 ****************************************************/
struct BatteryPins {
  int relayPin;          // Relay control pin
  int voltagePin;        // Battery voltage through load
  int shuntHigh;         // Shunt high side
  int shuntLow;          // Shunt low side
  int directVoltagePin;  // Direct battery positive (no load)
};

/****************************************************
 * Battery hardware configuration
 ****************************************************/
BatteryPins batteries[NUM_BATTERIES] = {
  {2, A0, A1, A2},   // Battery 1
  {3, A3, A4, A5},   // Battery 2
};

/****************************************************
 * System constants
 ****************************************************/
const float SHUNT_RESISTANCE = 0.8f;   // Ohms
const float V_REF = 5.01f;             // ADC reference voltage
const int TEST_DURATION = 30;           // Default test duration (seconds)
int testDuration = TEST_DURATION;

/****************************************************
 * Runtime measurement data for each battery
 ****************************************************/
struct BatteryData {
  float voltage;       // Battery voltage
  float shuntHigh;     // Shunt high voltage
  float shuntLow;      // Shunt low voltage
  float deff;          // Shunt voltage difference
  float current;       // Calculated current
  float capacity;      // Accumulated capacity (Ah)
  float lastCurrent;   // For current smoothing (optional)
};

BatteryData battData[NUM_BATTERIES];
bool activeBatteries[NUM_BATTERIES] = {false};

/****************************************************
 * System initialization
 ****************************************************/
void setup() {
  Serial.begin(9600);

  for (int i = 0; i < NUM_BATTERIES; i++) {
    pinMode(batteries[i].relayPin, OUTPUT);
    digitalWrite(batteries[i].relayPin, LOW);
  }

  Serial.println("System Ready");
  Serial.println("START:1,2        -> Timed test (default 30s)");
  Serial.println("START:1,2:60     -> Timed test (60s)");
  Serial.println("FULLTEST:1,2     -> Full discharge test");

  float v1 = analogRead(A0) * (V_REF / 1023.0f);
  Serial.print("Battery 1 voltage before test: ");
  Serial.println(v1);
}

/****************************************************
 * Main loop: command handling
 ****************************************************/
void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.startsWith("START:")) {
      parseCommand(command);
      startTesting();
    } 
    else if (command.startsWith("FULLTEST:")) {
      parseCommand(command);
      startFullTest();
    }
  }
}

/****************************************************
 * Parse serial command and activate batteries
 ****************************************************/
void parseCommand(String cmd) {
  for (int i = 0; i < NUM_BATTERIES; i++) activeBatteries[i] = false;
  testDuration = TEST_DURATION;

  int firstColon = cmd.indexOf(':');
  if (firstColon == -1) return;

  String rest = cmd.substring(firstColon + 1);
  rest.trim();

  int secondColon = rest.indexOf(':');

  String battPart;
  String durationPart = "";

  if (secondColon == -1) {
    battPart = rest;
  } else {
    battPart = rest.substring(0, secondColon);
    durationPart = rest.substring(secondColon + 1);
  }

  // Parse battery numbers
  while (battPart.length() > 0) {
    int commaPos = battPart.indexOf(',');
    String numStr = (commaPos == -1) ? battPart : battPart.substring(0, commaPos);
    int battNum = numStr.toInt() - 1;

    if (battNum >= 0 && battNum < NUM_BATTERIES) {
      activeBatteries[battNum] = true;
    }

    if (commaPos == -1) break;
    battPart = battPart.substring(commaPos + 1);
  }

  // Optional duration override
  if (durationPart.length() > 0) {
    int d = durationPart.toInt();
    if (d > 0 && d <= 3600) testDuration = d;
  }
}

/****************************************************
 * Timed test mode
 ****************************************************/
void startTesting() {
  // Pre-test measurement (no load)
  for (int i = 0; i < NUM_BATTERIES; i++) {
    if (activeBatteries[i]) {
      takeMeasurements(i, -2);
      sendSerialData(i, -2);
    }
  }

  // Enable relays
  for (int i = 0; i < NUM_BATTERIES; i++) {
    if (activeBatteries[i]) {
      digitalWrite(batteries[i].relayPin, HIGH);
      battData[i].capacity = 0;
      battData[i].lastCurrent = 0;
    }
  }

  delay(500);

  // Timed sampling loop
  for (int sec = 0; sec < testDuration; sec++) {
    unsigned long loopStart = millis();

    for (int i = 0; i < NUM_BATTERIES; i++) {
      if (activeBatteries[i]) {
        takeMeasurements(i, sec);
        sendSerialData(i, sec);
      }
    }

    while (millis() - loopStart < 1000) {}
  }

  // Disable relays
  for (int i = 0; i < NUM_BATTERIES; i++) {
    digitalWrite(batteries[i].relayPin, LOW);
  }

  // Post-test measurement
  delay(3000);
  for (int i = 0; i < NUM_BATTERIES; i++) {
    if (activeBatteries[i]) {
      takeMeasurements(i, testDuration + 3);
      sendSerialData(i, testDuration + 3);
    }
  }

  Serial.println("TEST_COMPLETE");
}

/****************************************************
 * Full discharge test mode (until 3.0V)
 ****************************************************/
void startFullTest() {
  for (int i = 0; i < NUM_BATTERIES; i++) {
    if (activeBatteries[i]) {
      takeMeasurements(i, -2);
      sendSerialData(i, -2);
    }
  }

  for (int i = 0; i < NUM_BATTERIES; i++) {
    if (activeBatteries[i]) {
      digitalWrite(batteries[i].relayPin, HIGH);
      battData[i].capacity = 0;
    }
  }

  delay(500);

  int sec = 0;
  bool finished[NUM_BATTERIES] = {false};
  bool stillRunning = true;

  while (stillRunning) {
    stillRunning = false;

    for (int i = 0; i < NUM_BATTERIES; i++) {
      if (activeBatteries[i] && !finished[i]) {
        takeMeasurements(i, sec);
        sendSerialData(i, sec);

        if (battData[i].voltage <= 3.0) {
          finished[i] = true;
          digitalWrite(batteries[i].relayPin, LOW);
          Serial.print("TEST_COMPLETE:");
          Serial.println(i + 1);
        } else {
          stillRunning = true;
        }
      }
    }

    delay(1000);
    sec++;
  }

  delay(3000);
}

/****************************************************
 * Perform ADC measurements and calculations
 ****************************************************/
void takeMeasurements(int battNum, int sec) {
  int voltageReadPin = (sec < 0) ?
    batteries[battNum].directVoltagePin :
    batteries[battNum].voltagePin;

  battData[battNum].voltage = readFilteredAnalog(voltageReadPin);
  battData[battNum].shuntHigh = readFilteredAnalog(batteries[battNum].shuntHigh);
  battData[battNum].shuntLow  = readFilteredAnalog(batteries[battNum].shuntLow);

  battData[battNum].deff =
    battData[battNum].shuntHigh - battData[battNum].shuntLow;

  battData[battNum].current =
    battData[battNum].deff / SHUNT_RESISTANCE;

  if (sec > 0) {
    battData[battNum].capacity += battData[battNum].current / 3600.0f;
  }
}

/****************************************************
 * ADC noise-reduced reading
 ****************************************************/
float readFilteredAnalog(int pin) {
  analogRead(pin);
  delayMicroseconds(300);

  float sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pin);
    delayMicroseconds(150);
  }

  return (sum / 10.0f) * (V_REF / 1023.0f);
}

/****************************************************
 * Serial data output (CSV-like format)
 ****************************************************/
void sendSerialData(int battNum, int sec) {
  Serial.print("BATT:");
  Serial.print(battNum + 1);
  Serial.print("|SEC:");
  Serial.print(sec);
  Serial.print("|V_BAT:");
  Serial.print(battData[battNum].voltage, 3);
  Serial.print("|V_SH_H:");
  Serial.print(battData[battNum].shuntHigh, 4);
  Serial.print("|V_SH_L:");
  Serial.print(battData[battNum].shuntLow, 4);
  Serial.print("|V_DIFF:");
  Serial.print(battData[battNum].deff, 4);
  Serial.print("|A:");
  Serial.print(battData[battNum].current, 3);
  Serial.print("|mAh:");
  Serial.print(battData[battNum].capacity * 1000.0f, 1);
  Serial.println();
}
