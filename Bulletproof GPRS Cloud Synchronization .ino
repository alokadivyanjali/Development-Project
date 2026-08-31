Serial.println("\n[Cloud] Checking IP Address...");
String ipResponse = sendATResponse("AT+SAPBR=2,1", 2000); 

// Abort logic if cellular network fails to assign an IP
if (ipResponse.indexOf("0.0.0.0") != -1 || ipResponse.indexOf("ERROR") != -1) {
    Serial.println("\n[ERROR] No IP Address! Network weak. Aborting upload.");
    return; 
}

// Execute Cloud POST if IP is valid
sendAT("AT+HTTPACTION=1", 15000);
