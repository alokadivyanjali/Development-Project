#include <HardwareSerial.h>
#include <DHT.h>

// ==========================================
// 1. PIN CONFIGURATION
// ==========================================
#define DHTPIN          4       // DHT11 Data Pin
#define DHTTYPE         DHT11   // DHT 11 model

#define TRIG_PIN        5       // Ultrasonic TRIG Pin
#define ECHO_PIN        18      // Ultrasonic ECHO Pin

#define TILT_PIN        19      // Tilt Sensor Digital Pin (SW-520D)

#define MOISTURE_PIN    34      // Soil Moisture Sensor Analog Pin (ADC1)
#define SMOKE_PIN       35      // Smoke/Gas Sensor Analog Pin (ADC1)

// UART2 for SIM800L (ESP32 RX2 = Pin 16, TX2 = Pin 17)
HardwareSerial sim800l(2);

// ==========================================
// 2. NETWORK & CLOUD CONFIGURATION
// ==========================================
const String APN            = "mobitel";                        
const String THINGHTTP_KEY  = "GB18XS6BR95D1VWL";               
const String PHONE_NUMBER   = "+94774601438";                    
const String BRIDGE_URL     = "http://api.thingspeak.com/apps/thinghttp/send_request";

// ==========================================
// 3. THRESHOLDS & PARAMETERS
// ==========================================
const float BIN_HEIGHT_CM       = 40.0;  
const float SENSOR_OFFSET_CM    = 5.0;   

const int   FULL_THRESHOLD_PCT  = 85;    
const float FIRE_TEMP_THRESHOLD = 50.0;  
const int   SMOKE_THRESHOLD_ADC = 1800;  
const int   WET_THRESHOLD_ADC   = 2500;  

const unsigned long CLOUD_INTERVAL_MS = 60000; 

// ==========================================
// 4. GLOBAL STATE VARIABLES
// ==========================================
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastCloudSync = 0;

int   currentFillPercent = 0;
float currentTemp        = 0.0;
float currentHumidity    = 0.0;
bool  isTilted           = false;
int   currentMoistureRaw = 0;
int   currentSmokeRaw    = 0;
String wasteType         = "DRY";

bool fullAlertSent  = false;
bool fireAlertSent  = false;
bool smokeAlertSent = false;
bool tiltAlertSent  = false;

// ==========================================
// 5. HELPER FUNCTIONS
// ==========================================

void sendAT(String command, int timeoutMs = 2000) {
  Serial.println(command); 
  sim800l.println(command);
  long int start = millis();
  while ((millis() - start) < timeoutMs) {
    while (sim800l.available()) {
      Serial.write(sim800l.read());
    }
  }
}

// Missed function restored here!
String sendATResponse(String command, int timeoutMs = 2000) {
  Serial.println(command); 
  String response = "";
  sim800l.println(command);
  long int start = millis();
  while ((millis() - start) < timeoutMs) {
    while (sim800l.available()) {
      char c = sim800l.read();
      Serial.write(c); 
      response += c;
    }
  }
  return response;
}

void readSensors() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float distanceCm = (duration > 0) ? ((duration * 0.0343) / 2.0) : BIN_HEIGHT_CM;
  float usableHeight = BIN_HEIGHT_CM - SENSOR_OFFSET_CM;
  float wasteLevel = BIN_HEIGHT_CM - distanceCm;
  currentFillPercent = constrain((int)((wasteLevel / usableHeight) * 100.0), 0, 100);

  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) currentTemp = t;
  if (!isnan(h)) currentHumidity = h;

  isTilted = (digitalRead(TILT_PIN) == HIGH);

  currentMoistureRaw = analogRead(MOISTURE_PIN);
  wasteType = (currentMoistureRaw < WET_THRESHOLD_ADC) ? "WET" : "DRY";

  currentSmokeRaw = analogRead(SMOKE_PIN);

  Serial.println("\n--- Live Sensor Readings ---");
  Serial.printf("Fill Level: %d%% (Dist: %.1f cm)\n", currentFillPercent, distanceCm);
  Serial.printf("Temperature: %.1f °C | Humidity: %.1f%%\n", currentTemp, currentHumidity);
  Serial.printf("Waste Type: %s (Raw Moisture: %d)\n", wasteType.c_str(), currentMoistureRaw);
  Serial.printf("Smoke Level: %d / 4095\n", currentSmokeRaw);
  Serial.printf("Tilt Status: %s\n", isTilted ? "TILTED / FALLEN" : "SAFE / UPRIGHT");
}

void sendSMSAlert(String message) {
  Serial.println("\n[!] DISPATCHING EMERGENCY SMS ALERT...");
  sendAT("AT+CMGF=1", 1000);
  
  sim800l.println("AT+CMGS=\"" + PHONE_NUMBER + "\"");
  delay(1000);
  
  sim800l.print("[SMART BIN ALERT]\n" + message);
  delay(500);
  
  sim800l.write(26); 
  delay(7000);
  
  while (sim800l.available()) {
    Serial.write(sim800l.read());
  }
  Serial.println("\n[!] SMS Dispatched.");
}

void checkAlertThresholds() {
  if (currentFillPercent >= FULL_THRESHOLD_PCT) {
    if (!fullAlertSent) {
      sendSMSAlert("Bin is " + String(currentFillPercent) + "% FULL! Waste collection required.");
      fullAlertSent = true;
    }
  } else if (currentFillPercent < (FULL_THRESHOLD_PCT - 10)) {
    fullAlertSent = false;
  }

  if (currentTemp >= FIRE_TEMP_THRESHOLD) {
    if (!fireAlertSent) {
      sendSMSAlert("HIGH TEMPERATURE ALERT! Temp: " + String(currentTemp, 1) + " C.");
      fireAlertSent = true;
    }
  } else if (currentTemp < (FIRE_TEMP_THRESHOLD - 5)) {
    fireAlertSent = false;
  }

  if (currentSmokeRaw >= SMOKE_THRESHOLD_ADC) {
    if (!smokeAlertSent) {
      sendSMSAlert("CRITICAL SMOKE/GAS DETECTED! Smoke Level: " + String(currentSmokeRaw) + ".");
      smokeAlertSent = true;
    }
  } else if (currentSmokeRaw < (SMOKE_THRESHOLD_ADC - 300)) {
    smokeAlertSent = false;
  }

  if (isTilted) {
    if (!tiltAlertSent) {
      sendSMSAlert("VANDALISM / FALL ALERT: Bin has been tipped over!");
      tiltAlertSent = true;
    }
  } else {
    tiltAlertSent = false;
  }
}

void uploadDataToCloud() {
  Serial.println("\n--- Initiating Cloud Upload ---");

  String tiltStatusStr = isTilted ? "TILTED" : "SAFE";
  String payload = "api_key=" + THINGHTTP_KEY +
                  "&fill=" + String(currentFillPercent) +
                  "&temp=" + String((int)currentTemp) +
                  "&humidity=" + String((int)currentHumidity) +
                  "&tilt=" + tiltStatusStr +
                  "&moisture=" + wasteType +
                  "&smoke=" + String(currentSmokeRaw);

  sendAT("AT+HTTPTERM", 1000);
  sendAT("AT+SAPBR=0,1", 2000); 

  Serial.println("\n[Cloud] Attaching to GPRS Network...");
  sendAT("AT+CGATT=1", 3000);

  sendAT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", 1000);
  sendAT("AT+SAPBR=3,1,\"APN\",\"" + APN + "\"", 1000);
  
  Serial.println("\n[Cloud] Opening Connection...");
  sendAT("AT+SAPBR=1,1", 5000); 
  
  Serial.println("\n[Cloud] Checking IP Address...");
  String ipResponse = sendATResponse("AT+SAPBR=2,1", 2000); 
  
  if (ipResponse.indexOf("0.0.0.0") != -1 || ipResponse.indexOf("ERROR") != -1) {
      Serial.println("\n[ERROR] No IP Address! Network weak. Aborting upload for this cycle.");
      return; 
  }

  sendAT("AT+HTTPINIT", 1000);
  sendAT("AT+HTTPSSL=0", 1000);
  sendAT("AT+HTTPPARA=\"CID\",1", 1000);
  sendAT("AT+HTTPPARA=\"URL\",\"" + BRIDGE_URL + "\"", 1000);
  sendAT("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"", 1000);

  Serial.println("\n[Cloud] Sending Data Length...");
  String dataCmd = "AT+HTTPDATA=" + String(payload.length()) + ",10000";
  sendAT(dataCmd, 2500);

  Serial.println("\n[Cloud] Sending Actual Payload...");
  sendAT(payload, 3000);

  Serial.println("\n[Cloud] Executing Cloud POST...");
  sim800l.println("AT+HTTPACTION=1");
  
  long int startWait = millis();
  while ((millis() - startWait) < 15000) {
    while (sim800l.available()) {
      Serial.write(sim800l.read());
    }
  }

  sendAT("AT+HTTPTERM", 1000);
  
  Serial.println("\n--- Cloud Upload Completed ---");
}

// ==========================================
// 6. SETUP & MAIN LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  sim800l.begin(9600, SERIAL_8N1, 16, 17);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TILT_PIN, INPUT);
  pinMode(MOISTURE_PIN, INPUT);
  pinMode(SMOKE_PIN, INPUT);

  dht.begin();
  delay(3000);

  Serial.println("\n==========================================");
  Serial.println("   SMART BIN MASTER FIRMWARE (FINAL)      ");
  Serial.println("==========================================");

  sendAT("AT", 1000);
  sendAT("AT+CSQ", 1000);
  sendAT("AT+CREG?", 1000);

  readSensors();
  uploadDataToCloud();
  lastCloudSync = millis();
}

void loop() {
  readSensors();
  checkAlertThresholds();

  if (millis() - lastCloudSync >= CLOUD_INTERVAL_MS) {
    uploadDataToCloud();
    lastCloudSync = millis();
  }

  delay(2000);
}