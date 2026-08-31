digitalWrite(TRIG_PIN, LOW);
delayMicroseconds(2);
digitalWrite(TRIG_PIN, HIGH);
delayMicroseconds(10);
digitalWrite(TRIG_PIN, LOW);

// Measure Echo Duration
long duration = pulseIn(ECHO_PIN, HIGH, 30000);

// Calculate Distance (Speed of sound = 0.0343 cm/us)
float distanceCm = (duration > 0) ? ((duration * 0.0343) / 2.0) : BIN_HEIGHT_CM;

// Convert to Percentage
float usableHeight = BIN_HEIGHT_CM - SENSOR_OFFSET_CM;
float wasteLevel = BIN_HEIGHT_CM - distanceCm;
currentFillPercent = constrain((int)((wasteLevel / usableHeight) * 100.0), 0, 100);

