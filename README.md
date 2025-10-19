# cm3040_iot - Smart Home Security System

## Overview
This project presents an offline-capable Smart Home Security System developed using the ESP8266 NodeMCU. It integrates temperature monitoring, motion detection, flame detection, and RFID-based door access control. All processing is done locally to ensure privacy, reliability, and independence from cloud connectivity.

## Key Features
- Temperature monitoring with buzzer alerts above 40°C  
- Motion detection using PIR sensor and LED indicator  
- Flame detection with immediate audio alerts  
- RFID-based access control with servo lock mechanism  
- Local web dashboard for real-time monitoring  
- Fully offline operation with low power consumption  
- Modular design for future scalability

## Hardware Components
| Component | Function |
|------------|-----------|
| ESP8266 NodeMCU | Central microcontroller and local web server |
| DHT22 Sensor | Measures room temperature |
| PIR Motion Sensor (HC-SR501) | Detects motion or intrusion |
| Flame Sensor (KY-026) | Detects fire hazards |
| RFID Module (MFRC522) | Handles user authentication |
| Servo Motor (SG90) | Controls the door lock mechanism |
| Buzzer | Provides alert signals |
| LED | Indicates motion or alert states |

## Pin Configuration
| Component | ESP8266 Pin(s) |
|------------|----------------|
| RFID (MFRC522) | RST → D3, SDA → D4, SCK → D5, MOSI → D7, MISO → D6 |
| DHT22 Sensor | D2 |
| PIR Sensor | D1 |
| Flame Sensor | A0 |
| Buzzer | D0 |
| LED | RX (GPIO3) |
| Servo Motor | D8 |

## Software and Libraries
Developed using the Arduino IDE.  
**Libraries used:**
- ESP8266WiFi.h  
- ESP8266WebServer.h  
- MFRC522.h  
- Servo.h  
- DHT.h  
- ArduinoJson.h  

## System Workflow
1. Sensors collect temperature, motion, and flame data.  
2. The ESP8266 processes input and triggers appropriate actions.  
3. RFID tags are scanned and authenticated.  
4. The servo unlocks the door for authorized tags.  
5. Alerts are issued through the buzzer or LED indicators.  
6. The web dashboard displays real-time sensor and access status.

## Web Dashboard
The ESP8266 hosts a local web interface accessible via its IP address.  
Features include:
- Real-time updates for temperature, motion, flame, and access logs  
- Color-coded status indicators  
- Automatic data refresh every few seconds  

## Testing Summary
| Test | Expected Outcome | Result |
|------|------------------|--------|
| Temperature > 40°C | Buzzer activates | Passed |
| Motion Detected | LED ON | Passed |
| Flame Detected | Buzzer ON | Passed |
| RFID Authorized | Door unlocks (10s) | Passed |
| RFID Unauthorized | Buzzer ON | Passed |
| Dashboard Refresh | Updates within 2s | Passed |

## Performance Overview
| Function | Average Response Time |
|-----------|-----------------------|
| RFID Authentication | < 1 s |
| Servo Unlock Duration | 10 s |
| Temperature Alert | < 2 s |
| Motion Detection | < 1 s |
| Flame Detection | < 2 s |
| Power Usage | 0.5–2.5 W depending on state |

## Future Improvements
- Integration of LCD display for local status feedback  
- Battery backup for power continuity  
- AES encryption for RFID data security  
- Bluetooth or mobile app for local control  
- Expansion to include gas and humidity sensors  

## Setup Instructions
1. Open `smart_home_security.ino` in Arduino IDE.  
2. Install all required libraries.  
3. Connect the ESP8266 NodeMCU via USB and select the correct COM port.  
4. Upload the sketch.  
5. Open the Serial Monitor to obtain the local IP address.  
6. Enter the IP address in a web browser to view the dashboard.

## Project Architecture Diagram
The following diagram illustrates the overall system architecture, showing the interaction between sensors, actuators, and the ESP8266 NodeMCU.

![alt text](https://github.com/marcustlh/cm3040_iot/blob/8552dcd5b6a60aa736b5b598d2952b0fcad3782d/setup/system_diagram.png?raw=true)

## Author
**Marcus Tan Lai He**  
CM3040 Physical Computing and IoT (2025)

## License
This project is open-source under the MIT License.
