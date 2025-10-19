// Libraries
#include <SPI.h>
#include <MFRC522.h>      // RFID library
#include <Servo.h>        // Servo motor library
#include <DHT.h>         // DHT sensor library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// **Pin Definitions**
#define RST_PIN D3         // RFID Reset pin
#define SS_PIN D4          // RFID Slave Select pin

#define DHTPIN D2          // DHT22 Temperature sensor pin
#define DHTTYPE DHT22      // Sensor type

#define PIR_SENSOR D1      // PIR Motion sensor pin
#define BUZZER D0          // Buzzer pin

#define LED_PIN 3         // LED for Motion Detection

#define FLAME_A A0   // Using analog input for flame detection

#define SERVO_PIN D8   // PWM supported

// **Wi-Fi Configuration**
const char* ssid = "𝕞.𝕒.𝕣.𝕔.𝕦.𝕤";
const char* password = "20011002@Gypsy";
ESP8266WebServer server(80);

// **Initialize Components**
MFRC522 mfrc522(SS_PIN, RST_PIN);   // RFID module
Servo doorServo;                    // Servo motor
DHT dht(DHTPIN, DHTTYPE);           // Temperature sensor

// **RFID Authorized & Unauthorized UID**
String authorizedTagUID = "3 1B 6E 2D";  // RFID Tag (GRANTS access)
String unauthorizedCardUID = "6B 6F 3D 2";  // RFID Card (DENIES access)

String rfidStatus = "Idle";  // Tracks RFID Access status for dashboard

// **Serve Dashboard Page**
const char* htmlPage PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Home Security Dashboard</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; }
        .container { max-width: 400px; margin: 20px auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
        h2 { color: #333; }
        .sensor { font-size: 18px; margin: 10px 0; }
        .status { font-size: 20px; font-weight: bold; padding: 5px 10px; border-radius: 5px; }
        .safe { background: #28a745; color: white; }
        .alert { background: #dc3545; color: white; }
        .refresh { padding: 8px 15px; font-size: 16px; cursor: pointer; border: none; background: #007bff; color: white; border-radius: 5px; }
        .warning { background: #ffc107; color: black; } /* Yellow for Scanning */
        .alert { background: #dc3545; color: white; }  /* Red for Access Denied */
    </style>
</head>
<body>
    <div class="container">
        <h2>Smart Home Security Dashboard</h2>
        <div class="sensor">Temperature: <span id="temperature">--</span>°C</div>
        <div class="sensor">Motion Detected: <span id="motion" class="status">--</span></div>
        <div class="sensor">Flame Detected: <span id="flame" class="status">--</span></div>
        <div class="sensor">RFID Access: <span id="access" class="status">--</span></div>
        <button class="refresh" onclick="updateData()">Refresh Data</button>
    </div>

    <script>
        function updateData() {
            fetch('/status')
            .then(response => response.json())
            .then(data => {
                document.getElementById("temperature").innerText = data.temperature.toFixed(1);
                document.getElementById("motion").innerText = data.motion ? "Detected" : "No Movement";
                document.getElementById("motion").className = "status " + (data.motion ? "alert" : "safe");
                document.getElementById("flame").innerText = data.flame;
                document.getElementById("flame").className = "status " + (data.flame === "Fire Detected" ? "alert" : "safe");
                document.getElementById("access").innerText = data.access;

                let accessClass = "safe"; // Default green
                if (data.access === "Access Denied") {
                    accessClass = "alert";  // Red
                } else if (data.access === "Scanning") {
                    accessClass = "warning"; // Yellow
                }

                document.getElementById("access").className = "status " + accessClass;


            })
            .catch(error => console.error("Error fetching data:", error));
        }

        setInterval(updateData, 2000);  // Auto-refresh every 2 seconds
        updateData();  // Initial load
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("\nConnecting to Wi-Fi...");
  
  int timeout = 20; // Timeout after 10 seconds
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi!");
    Serial.print("ESP8266 IP Address: ");
    Serial.println(WiFi.localIP());  // Print the IP Address
  } else {
    Serial.println("\nFailed to connect to Wi-Fi. Please check your SSID/Password.");
  }

  server.on("/status", HTTP_GET, []() {
    DynamicJsonDocument doc(256);
    doc["temperature"] = dht.readTemperature();
    doc["motion"] = digitalRead(PIR_SENSOR);
    
    int flameValue = analogRead(FLAME_A);
    bool fireDetected = (flameValue < 800);  
    doc["flame"] = fireDetected ? "Fire Detected" : "No Fire";  
    doc["flameValue"] = flameValue;  

    // **Send the RFID status update to the dashboard**
    doc["access"] = rfidStatus;  

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  server.on("/", HTTP_GET, []() {
    Serial.println("Serving Dashboard...");
    server.send_P(200, "text/html", htmlPage);
  });

  server.begin();

  SPI.begin();
  mfrc522.PCD_Init();
  dht.begin();
  
  pinMode(PIR_SENSOR, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(FLAME_A, INPUT);
  
  doorServo.attach(SERVO_PIN);
  doorServo.write(0);  

  Serial.println("Smart Home Security System Ready!");
}

void loop() {
  server.handleClient();
  checkRFID();
  checkTemperature();
  checkMotion();
  checkFlame();
  delay(50);
}

void checkRFID() {
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;

    rfidStatus = "Scanning";  // Update dashboard status
    
    String tagUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        tagUID += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) tagUID += " ";
    }

    tagUID.toUpperCase();
    tagUID.trim();

    Serial.print("Scanned UID: ");
    Serial.println(tagUID);

    String trimmedAuthorizedUID = authorizedTagUID;
    String trimmedUnauthorizedUID = unauthorizedCardUID;
    trimmedAuthorizedUID.trim();
    trimmedUnauthorizedUID.trim();

    if (tagUID.equalsIgnoreCase(trimmedAuthorizedUID)) {  
        Serial.println("Access Granted! Unlocking door...");
        rfidStatus = "Access Granted";

        doorServo.write(90);  // Unlock door (servo moves to 90 degrees)
        delay(10000);  // Keep unlocked for 10 seconds
        doorServo.write(0);   // Lock door (servo moves back to 0 degrees)

    } else {  
        Serial.println("Access Denied! Locking door...");
        rfidStatus = "Access Denied";

        activateBuzzer(3);
        doorServo.write(0);  // Ensure the door remains locked
    }

    mfrc522.PICC_HaltA();
}

// **Function: Check Temperature**
void checkTemperature() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println("Failed to read temperature!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println("°C");

  if (temp > 40.0) {
    Serial.println("Temperature too high! Alert triggered.");
    activateBuzzer(5);
  }
}

// **Function: Check Motion Detection**
void checkMotion() {
  if (digitalRead(PIR_SENSOR) == HIGH) {
    Serial.println("Motion Detected!");
    digitalWrite(LED_PIN, HIGH);  // **Turn ON LED when motion detected**
  } else {
    digitalWrite(LED_PIN, LOW);  // **Turn OFF LED when no motion**
  }
}

// **Function: Check Flame Detection**
void checkFlame() {
  int flameAnalog = analogRead(FLAME_A);
  
  if (flameAnalog < 800) {  // Adjust threshold if needed
    Serial.println("Fire Detected!");
    activateBuzzer(10);
  }
}

// **Function: Activate Buzzer**
void activateBuzzer(int times) {
  for (int i = 0; i < times; i++) {
        digitalWrite(BUZZER, HIGH);
        delay(500);  // Increased ON duration
        digitalWrite(BUZZER, LOW);
        delay(200);  // Small delay before next beep
  }
}
