#include <WiFi.h>
#include <BluetoothSerial.h>
#include <heltec.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClient.h>

#define PRG_BUTTON 0 // Pin number of the PRG button

// Display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Heltec.begin(true, false, true);
  display.begin(SSD1306_I2C_ADDRESS, OLED_RESET);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  SerialBT.begin("ESP32");

  pinMode(PRG_BUTTON, INPUT);
}

void loop() {
  int numNetworks = WiFi.scanNetworks();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("((o)) Scanning... ((o))");
  display.display();
  delay(500);

  int index = 0;
  while (true) {
    String ssid = WiFi.SSID(index);
    String bssid = WiFi.BSSIDstr(index);
    int rssi = WiFi.RSSI(index);
    int channel = WiFi.channel(index);
    uint8_t encryptionType = WiFi.encryptionType(index);

    int numClients = (WiFi.getMode() == WIFI_MODE_AP) ? WiFi.softAPgetStationNum() : 0;

    String encryptionString;
    switch (encryptionType) {
      case WIFI_AUTH_OPEN:
        encryptionString = "Open";
        break;
      case WIFI_AUTH_WEP:
        encryptionString = "WEP";
        break;
      case WIFI_AUTH_WPA_PSK:
        encryptionString = "WPA_PSK";
        break;
      case WIFI_AUTH_WPA2_PSK:
        encryptionString = "WPA2_PSK";
        break;
      case WIFI_AUTH_WPA_WPA2_PSK:
        encryptionString = "WPA/WPA2_PSK";
        break;
      case WIFI_AUTH_WPA2_ENTERPRISE:
        encryptionString = "WPA2_Enterprise";
        break;
      default:
        encryptionString = "Unknown";
        break;
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SSID: " + ssid);
    display.println("BSSID: " + bssid);
    display.println("RSSI: " + String(rssi) + " dBm");
    display.println("CHANNEL: " + String(channel));
    display.println("ENCRYPTION: " + encryptionString);
    display.println("CLIENTS: " + String(numClients));

    // Check for vulnerabilities
    checkVulnerabilities(ssid);

    // Perform Bluetooth scan
    checkBluetoothDevices();

    // Perform port scan on the first device
    if (index == 0) {
      checkOpenPorts(bssid);
    }

    // Perform MAC address lookup
    String macDetails = macAddressLookup(bssid);
    display.println("MAC DETAILS: " + macDetails);

    display.display();
    delay(5000);

    // Check for button press
    if (digitalRead(PRG_BUTTON) == HIGH) {
      while (digitalRead(PRG_BUTTON) == HIGH) {
        delay(10);
      }
      index++;
      if (index >= numNetworks) {
        index = 0;
      }
    }
  }
}

void checkVulnerabilities(String ssid) {
  SerialBT.println("Checking vulnerabilities for network: " + ssid);

  // Add your custom vulnerability checks here

  // Example: Check if default credentials are being used (replace "admin" and "password" with actual defaults)
  if (ssid.toLowerCase().contains("router") || ssid.toLowerCase().contains("wifi")) {
    SerialBT.println("Potential vulnerability: Default credentials might be in use!");
  }

  // Add more custom checks based on your requirements
}

void checkBluetoothDevices() {
  int devices = SerialBT.availableDevices();

  for (int i = 0; i < devices; i++) {
    String deviceName = SerialBT.getRemoteName(SerialBT.getRemoteAddress(i));
    String deviceAddress = SerialBT.getRemoteAddress(i);

    SerialBT.println("Bluetooth Device: " + deviceName + " (" + deviceAddress + ")");

    // Perform additional Bluetooth checks or actions here
  }
}

void checkOpenPorts(String bssid) {
  SerialBT.println("Performing port scan for device with BSSID: " + bssid);

  // Implement your port scanning logic here

  // Example: Check common ports (replace with your custom logic)
  for (int port = 80; port <= 443; port++) {
    WiFiClient client;
    if (client.connect(bssid.c_str(), port)) {
      SerialBT.println("Port " + String(port) + " open");
      client.stop();
    }
  }
}

String macAddressLookup(String bssid) {
  SerialBT.println("Performing MAC address lookup for BSSID: " + bssid);

  // Use an online MAC address lookup API (replace with a suitable one)
  String macDetails = "";
  WiFiClient client;
  if (client.connect("api.maclookup.app", 80)) {
    client.println("GET /v2/lookup/" + bssid + " HTTP/1.1");
    client.println("Host: api.maclookup.app");
    client.println("Connection: close");
    client.println();
    delay(1000); // Wait for the server response

    while (client.available()) {
      macDetails += client.readString();
    }
    client.stop();
  }

  return macDetails;
}
