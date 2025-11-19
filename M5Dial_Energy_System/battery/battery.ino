#include <M5Dial.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Let's Encrypt Root CA Certificate (ISRG Root X1)
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

// Page navigation
enum PageType {
  PAGE_CAPACITY,        // 第0頁（起始頁）：電池容量設定
  PAGE_CHARGE_LIMIT,        // 第1頁：充電限制設定
  PAGE_DISCHARGE_LIMIT,     // 第2頁：放電限制設定
  PAGE_SERVER_DATA,         // 第3頁（最後一頁）：伺服器數據顯示
  PAGE_COUNT
};

// Global variables
int currentPage = PAGE_CAPACITY;                // Current page
bool isEditing = false;             // Whether in editing mode
long oldPosition = -999;            // Encoder position
float editStartValue = 0;           // Value when editing started
unsigned long lastButtonPress = 0;  // Debounce
unsigned long lastRefreshTime = 0;  // Last server data refresh
unsigned long lastPublishTime = 0;

// Battery parameters (push to server)
float batteryCapacity = 100.0;      // kWh
float batteryChargeLimit = 5.0;     // kW
float batteryDischargeLimit = 5.0;  // kW

// Server data (received from server)
float batterySOC = 0.0;             // % (from server)
float batteryActualPower = 0.0;     // kW (from server)
bool hasServerData = false;          // Whether we have received server data

// Parameter ranges
struct ParamRange {
  float min;
  float max;
  float step;
  const char* unit;
};

ParamRange capacityRange = {10.0, 1000.0, 1.0, "kWh"};
ParamRange chargeLimitRange = {0.1, 50.0, 0.1, "kW"};
ParamRange dischargeLimitRange = {0.1, 50.0, 0.1, "kW"};

// Color definitions
#define COLOR_BG 0x0000         // Black
#define COLOR_PRIMARY 0x07FF    // Cyan
#define COLOR_TEXT 0xFFFF       // White
#define COLOR_ACCENT 0xFFE0     // Yellow
#define COLOR_PROGRESS 0x07E0   // Green
#define COLOR_WARNING 0xF800    // Red
#define COLOR_GRAY 0x7BEF       // Gray
#define COLOR_DARK_GRAY 0x4208  // Dark Gray

// WiFi and MQTT settings - 根據 API spec
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";        // API spec endpoint
const int mqtt_port = 8884;                             // TLS port

// MQTT credentials from API spec
const char* mqtt_username = "";
const char* mqtt_password = "";

// Device ID
String DEVICE_ID = "";
String CLIENT_ID = "";  
String publish_topic = "";
String subscribe_topic = "";

WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastReconnectAttempt = 0;

// Function declarations
void drawProgressRing(float progress, uint16_t color);
void drawPageIndicators();
void drawEditPage(const char* title, float value, const char* unit, ParamRange range);
void drawServerDataPage();
void refreshServerData();

void setup_wifi() {
  delay(10);
  M5Dial.Display.fillScreen(COLOR_BG);
  M5Dial.Display.setTextSize(1.5);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.drawString("Connecting WiFi...", 120, 120);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    M5Dial.Display.fillCircle(60 + attempts * 5, 160, 2, COLOR_PRIMARY);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    M5Dial.Display.fillScreen(COLOR_BG);
    M5Dial.Display.drawString("WiFi Connected!", 120, 100);
    
    // Generate device ID from MAC address
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    char macStr[13];
    sprintf(macStr, "%02x%02x%02x%02x%02x%02x", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    char shortMacStr[7];
    sprintf(shortMacStr, "%02x%02x%02x", mac[3], mac[4], mac[5]);
    DEVICE_ID = String(shortMacStr);

    CLIENT_ID = "M5Dial-" + String(macStr);
    publish_topic = "dial.battery." + DEVICE_ID;
    subscribe_topic = "dial.battery.response." + DEVICE_ID;
    
    M5Dial.Display.drawString("ID: " + DEVICE_ID, 120, 130);
    M5Dial.Display.drawString("IP: " + WiFi.localIP().toString(), 120, 160);
    delay(2000);
  } else {
    M5Dial.Display.fillScreen(COLOR_BG);
    M5Dial.Display.drawString("WiFi Failed!", 120, 100);
    M5Dial.Display.drawString("Restarting...", 120, 130);
    delay(3000);
    ESP.restart();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<512> doc;
  char message[512];
  
  unsigned int copyLength = (length < 511) ? length : 511;
  for (unsigned int i = 0; i < copyLength; i++) {
    message[i] = (char)payload[i];
  }
  message[copyLength] = '\0';

  Serial.print("[SUBSCRIBE] Topic: ");
  Serial.println(topic);
  Serial.print("[SUBSCRIBE] Payload: ");
  Serial.println(message);

  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    return;
  }

  // Receive data from server
  bool dataChanged = false;
  
  if (doc.containsKey("soc")) {
    batterySOC = doc["soc"];
    dataChanged = true;
    hasServerData = true;
  }
  if (doc.containsKey("power")) {
    batteryActualPower = doc["power"];
    dataChanged = true;
    hasServerData = true;
  }
  
  if (dataChanged) {
    lastRefreshTime = millis();
    if (currentPage == PAGE_SERVER_DATA) {
      drawServerDataPage();
    }
  }
}

bool reconnect() {
  M5Dial.Display.fillScreen(COLOR_BG);
  M5Dial.Display.setTextSize(1.2);
  M5Dial.Display.drawString("Connecting MQTT...", 120, 60);
  M5Dial.Display.drawString(mqtt_server, 120, 85);
  
  Serial.println("\n=== MQTT Connection Attempt ===");
  Serial.print("Server: ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(mqtt_port);
  
  bool connected = client.connect(CLIENT_ID.c_str(), mqtt_username, mqtt_password);
  
  if (connected) {
    Serial.println("MQTT Connected Successfully!");
    
    M5Dial.Display.fillScreen(COLOR_BG);
    M5Dial.Display.setTextSize(1.5);
    M5Dial.Display.drawString("MQTT Connected!", 120, 100);
    delay(1000);
    
    // Subscribe to response topic
    client.subscribe(subscribe_topic.c_str());
    
    // Request initial data
    refreshServerData();
    
    return true;
  } else {
    int state = client.state();
    Serial.print("MQTT Connection Failed! State: ");
    Serial.println(state);
    
    M5Dial.Display.fillScreen(COLOR_BG);
    M5Dial.Display.drawString("MQTT Failed!", 120, 60);
    
    return false;
  }
}

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setTextSize(1.5);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextColor(COLOR_TEXT);

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== M5Dial Battery Controller v2.0 ===");
  Serial.println("Page-based UI with circular progress");
  
  setup_wifi();

  espClient.setCACert(root_ca);
  espClient.setTimeout(15);
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(512);
  client.setKeepAlive(60);

  if (!reconnect()) {
    delay(5000);
  }
  
  // Draw initial page
  drawCurrentPage();
  oldPosition = M5Dial.Encoder.read();
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    setup_wifi();
  }

  // Check MQTT connection
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
  
  M5Dial.update();
  
  // Handle encoder rotation
  long newPosition = M5Dial.Encoder.read();
  
  if (newPosition != oldPosition) {
    if (isEditing) {
      // In editing mode - adjust value
      handleValueAdjustment(newPosition - oldPosition);
      oldPosition = newPosition;
    } else {
      // Not editing - navigate between pages
      int delta = (newPosition - oldPosition) / 4;
      if (delta != 0) {
        currentPage = (currentPage + delta + PAGE_COUNT) % PAGE_COUNT;
        oldPosition = newPosition;
        drawCurrentPage();
      }
    }
  }
  
  // Handle button press with debounce
  if (M5Dial.BtnA.wasPressed()) {
    unsigned long now = millis();
    if (now - lastButtonPress > 200) {  // 200ms debounce
      lastButtonPress = now;
      handleButtonPress();
    }
  }
  
  delay(10);
}

void drawCurrentPage() {
  switch (currentPage) {
    case PAGE_CAPACITY:
      drawEditPage("Battery Capacity", batteryCapacity, capacityRange.unit, capacityRange);
      break;
    case PAGE_CHARGE_LIMIT:
      drawEditPage("Charge Limit", batteryChargeLimit, chargeLimitRange.unit, chargeLimitRange);
      break;
    case PAGE_DISCHARGE_LIMIT:
      drawEditPage("Discharge Limit", batteryDischargeLimit, dischargeLimitRange.unit, dischargeLimitRange);
      break;
    case PAGE_SERVER_DATA:
      drawServerDataPage();
      break;
  }
}

void drawEditPage(const char* title, float value, const char* unit, ParamRange range) {
  M5Dial.Display.fillScreen(COLOR_BG);
  
  // Draw title at top
  M5Dial.Display.setTextSize(1.8);
  M5Dial.Display.setTextColor(COLOR_PRIMARY);
  M5Dial.Display.drawString(title, 120, 40);
  
  // Draw large value in center
  M5Dial.Display.setTextSize(3.5);
  M5Dial.Display.setTextColor(isEditing ? COLOR_ACCENT : COLOR_TEXT);
  String valueStr = String(value, (range.step < 1) ? 1 : 0);
  M5Dial.Display.drawString(valueStr, 120, 120);
  
  // Draw unit
  M5Dial.Display.setTextSize(2);
  M5Dial.Display.setTextColor(COLOR_GRAY);
  M5Dial.Display.drawString(unit, 120, 160);
  
  // Draw status/instruction
  M5Dial.Display.setTextSize(1.2);
  if (isEditing) {
    M5Dial.Display.setTextColor(COLOR_ACCENT);
    M5Dial.Display.drawString("Rotate to adjust", 120, 190);
    M5Dial.Display.drawString("Press to save", 120, 205);
    
    // Draw progress ring when editing
    float progress = (value - range.min) / (range.max - range.min);
    drawProgressRing(progress, COLOR_PROGRESS);
  } else {
    M5Dial.Display.setTextColor(COLOR_GRAY);
    M5Dial.Display.drawString("Press to edit", 120, 190);
    M5Dial.Display.drawString("Rotate for pages", 120, 205);
  }
  
  // Draw page indicators
  drawPageIndicators();
}

void drawServerDataPage() {
  M5Dial.Display.fillScreen(COLOR_BG);
  
  // Draw title
  M5Dial.Display.setTextSize(1.8);
  M5Dial.Display.setTextColor(COLOR_PRIMARY);
  M5Dial.Display.drawString("Server Data", 120, 40);
  
  if (hasServerData) {
    // Draw SOC with battery icon
    drawBatteryIcon(120, 80, batterySOC);
    
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.setTextColor(COLOR_TEXT);
    M5Dial.Display.drawString("SOC: " + String(batterySOC, 1) + "%", 120, 110);
    
    // Draw power with direction indicator
    M5Dial.Display.setTextSize(1.5);
    String powerText;
    uint16_t powerColor;
    
    if (batteryActualPower > 0.05) {
      powerText = "Discharging";
      powerColor = COLOR_ACCENT;
      // Draw discharge arrow
      drawArrow(90, 150, true);
    } else if (batteryActualPower < -0.05) {
      powerText = "Charging";
      powerColor = COLOR_PROGRESS;
      // Draw charge arrow
      drawArrow(90, 150, false);
    } else {
      powerText = "Idle";
      powerColor = COLOR_GRAY;
    }
    
    M5Dial.Display.setTextColor(powerColor);
    M5Dial.Display.drawString(powerText, 120, 150);
    
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.setTextColor(COLOR_TEXT);
    M5Dial.Display.drawString(String(abs(batteryActualPower), 1) + " kW", 120, 175);
    
    // Draw refresh button hint
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.setTextColor(COLOR_GRAY);
    M5Dial.Display.drawString("Press to refresh", 120, 205);
    
  } else {
    // No data yet
    M5Dial.Display.setTextSize(1.5);
    M5Dial.Display.setTextColor(COLOR_GRAY);
    M5Dial.Display.drawString("No data yet", 120, 120);
    
    M5Dial.Display.setTextSize(1.2);
    M5Dial.Display.drawString("Press to refresh", 120, 160);
  }
  
  // Draw page indicators
  drawPageIndicators();
  
  // Draw connection status
  drawConnectionStatus();
}

void drawProgressRing(float progress, uint16_t color) {
  int centerX = 120;
  int centerY = 120;
  int radius = 110;
  int thickness = 8;
  
  // Draw background ring
  for (int angle = 0; angle < 360; angle += 2) {
    float rad = angle * PI / 180;
    int x1 = centerX + (radius - thickness) * cos(rad);
    int y1 = centerY + (radius - thickness) * sin(rad);
    int x2 = centerX + radius * cos(rad);
    int y2 = centerY + radius * sin(rad);
    M5Dial.Display.drawLine(x1, y1, x2, y2, COLOR_DARK_GRAY);
  }
  
  // Draw progress
  int endAngle = -90 + (progress * 360);
  for (int angle = -90; angle < endAngle; angle += 2) {
    float rad = angle * PI / 180;
    int x1 = centerX + (radius - thickness) * cos(rad);
    int y1 = centerY + (radius - thickness) * sin(rad);
    int x2 = centerX + radius * cos(rad);
    int y2 = centerY + radius * sin(rad);
    M5Dial.Display.drawLine(x1, y1, x2, y2, color);
  }
}

void drawPageIndicators() {
  int y = 230;
  int spacing = 15;
  int startX = 120 - ((PAGE_COUNT - 1) * spacing / 2);
  
  for (int i = 0; i < PAGE_COUNT; i++) {
    int x = startX + i * spacing;
    if (i == currentPage) {
      M5Dial.Display.fillCircle(x, y, 4, COLOR_PRIMARY);
    } else {
      M5Dial.Display.drawCircle(x, y, 3, COLOR_GRAY);
    }
  }
}

void drawBatteryIcon(int x, int y, float soc) {
  int width = 40;
  int height = 20;
  
  // Draw battery outline
  M5Dial.Display.drawRect(x - width/2, y - height/2, width, height, COLOR_TEXT);
  M5Dial.Display.fillRect(x + width/2, y - 3, 3, 6, COLOR_TEXT);
  
  // Fill based on SOC
  int fillWidth = (width - 4) * soc / 100;
  uint16_t fillColor = COLOR_PROGRESS;
  if (soc < 20) fillColor = COLOR_WARNING;
  else if (soc < 50) fillColor = COLOR_ACCENT;
  
  M5Dial.Display.fillRect(x - width/2 + 2, y - height/2 + 2, 
                          fillWidth, height - 4, fillColor);
}

void drawArrow(int x, int y, bool right) {
  if (right) {
    // Right arrow (discharge)
    M5Dial.Display.drawLine(x, y, x + 20, y, COLOR_ACCENT);
    M5Dial.Display.drawLine(x + 20, y, x + 15, y - 5, COLOR_ACCENT);
    M5Dial.Display.drawLine(x + 20, y, x + 15, y + 5, COLOR_ACCENT);
  } else {
    // Left arrow (charge)
    M5Dial.Display.drawLine(x + 20, y, x, y, COLOR_PROGRESS);
    M5Dial.Display.drawLine(x, y, x + 5, y - 5, COLOR_PROGRESS);
    M5Dial.Display.drawLine(x, y, x + 5, y + 5, COLOR_PROGRESS);
  }
}

void drawConnectionStatus() {
  int y = 10;
  M5Dial.Display.setTextSize(0.8);
  
  if (client.connected()) {
    M5Dial.Display.setTextColor(COLOR_PROGRESS);
    M5Dial.Display.drawString("Connected", 120, y);
  } else {
    M5Dial.Display.setTextColor(COLOR_WARNING);
    M5Dial.Display.drawString("Offline", 120, y);
  }
}

void handleButtonPress() {
  if (currentPage == PAGE_SERVER_DATA) {
    // On server data page (last page) - refresh data
    refreshServerData();
    
    // Show refresh animation
    M5Dial.Display.fillCircle(120, 120, 20, COLOR_PRIMARY);
    M5Dial.Display.setTextColor(COLOR_BG);
    M5Dial.Display.setTextSize(1.2);
    M5Dial.Display.drawString("Refresh", 120, 120);
    delay(300);
    
    drawCurrentPage();
  } else {
    // On setting pages (first 3 pages) - toggle edit mode
    if (isEditing) {
      // Save and exit edit mode
      isEditing = false;
      publishSettings();
      
      // Show confirmation
      M5Dial.Display.fillRect(0, 100, 240, 40, COLOR_PROGRESS);
      M5Dial.Display.setTextColor(COLOR_BG);
      M5Dial.Display.setTextSize(1.5);
      M5Dial.Display.drawString("Saved!", 120, 120);
      delay(500);
      
      drawCurrentPage();
    } else {
      // Enter edit mode
      isEditing = true;
      
      // Store starting value
      switch (currentPage) {
        case PAGE_CAPACITY:
          editStartValue = batteryCapacity;
          break;
        case PAGE_CHARGE_LIMIT:
          editStartValue = batteryChargeLimit;
          break;
        case PAGE_DISCHARGE_LIMIT:
          editStartValue = batteryDischargeLimit;
          break;
      }
      
      drawCurrentPage();
    }
  }
}

void handleValueAdjustment(int delta) {
  float* valuePtr = nullptr;
  ParamRange* rangePtr = nullptr;
  
  switch (currentPage) {
    case PAGE_CAPACITY:
      valuePtr = &batteryCapacity;
      rangePtr = &capacityRange;
      break;
    case PAGE_CHARGE_LIMIT:
      valuePtr = &batteryChargeLimit;
      rangePtr = &chargeLimitRange;
      break;
    case PAGE_DISCHARGE_LIMIT:
      valuePtr = &batteryDischargeLimit;
      rangePtr = &dischargeLimitRange;
      break;
    default:
      return;
  }
  
  if (valuePtr && rangePtr) {
    *valuePtr += delta * rangePtr->step;
    *valuePtr = constrain(*valuePtr, rangePtr->min, rangePtr->max);
    drawCurrentPage();
  }
}

void publishSettings() {
  if (!client.connected()) {
    Serial.println("[PUBLISH] Cannot publish: MQTT not connected");
    return;
  }
  
  StaticJsonDocument<256> doc;
  doc["capacity"] = batteryCapacity;
  doc["chargeLimit"] = batteryChargeLimit;
  doc["dischargeLimit"] = batteryDischargeLimit;
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  bool success = client.publish(publish_topic.c_str(), buffer, false);
  
  if (success) {
    Serial.println("[PUBLISH] Settings sent successfully");
    Serial.print("  Topic: ");
    Serial.println(publish_topic);
    Serial.print("  Payload: ");
    Serial.println(buffer);
  } else {
    Serial.println("[PUBLISH] Failed to send settings");
  }
}

void refreshServerData() {
  // Send empty message to request data
  if (client.connected()) {
    StaticJsonDocument<64> doc;
    doc["request"] = "status";
    
    char buffer[64];
    serializeJson(doc, buffer);
    
    client.publish(publish_topic.c_str(), buffer, false);
    
    Serial.println("[REQUEST] Requesting server data");
  }
}
