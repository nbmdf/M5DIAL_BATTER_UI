#include <M5Dial.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

// GenSet setting items
enum GenSetSettingItem {
  GENSET_POWER_LIMIT = 0,
  GENSET_CONFIRM,
  GENSET_SETTING_COUNT
};

// Global variables
int currentSettingIndex = 0;        // Current selected setting item
bool isEditing = false;             // Whether in editing mode
long oldPosition = -999;            // Encoder position

// GenSet parameters (to server)
float genSetPowerLimit = 20.0;      // kW - Max generation power (adjustable)

// GenSet status (from server)
bool genSetEnabled = false;         // ON/OFF status (from server)
float genSetRPM = 0.0;              // RPM values (from server)
float genSetEfficiency = 0.0;       // Efficiency % (from server)

// Function prototypes
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
bool reconnect();
void publishGenSetData();
void adjustValue(int delta);
void drawGenSetPage();
void drawSettingItem(const char* label, String value, int y, bool selected, bool editing);

// Color definitions
#define COLOR_BG 0x0000
#define COLOR_GENSET 0xFD20
#define COLOR_TEXT 0xFFF
#define COLOR_SELECTED 0xFFFF
#define COLOR_EDITING 0x39E7
#define COLOR_VALUE_EDIT 0xFFE0
#define COLOR_ON 0x07E0
#define COLOR_OFF 0xF800

const char* ssid = "";
const char* password = "";

const char* mqtt_server = "";   // API spec endpoint
const int mqtt_port = ;

const char* mqtt_username = "";
const char* mqtt_password = "";

String DEVICE_ID = "";
String CLIENT_ID = "";
String publish_topic = "";
String subscribe_topic = "";

WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastPublishTime = 0;
unsigned long lastReconnectAttempt = 0;

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
    M5Dial.Display.drawString(".", 120 + attempts * 5, 140);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    M5Dial.Display.fillScreen(COLOR_BG);
    M5Dial.Display.drawString("WiFi Connected!", 120, 100);

    uint8_t mac[6];
    WiFi.macAddress(mac);

    char macStr[13];
    sprintf(macStr, "%02x%02x%02x%02x%02x%02x", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    char shortMacStr[7];
    sprintf(shortMacStr, "%02x%02x%02x", mac[3], mac[4], mac[5]);
    DEVICE_ID = String(shortMacStr);

    CLIENT_ID = "M5Dial-GenSet-" + String(macStr);

    publish_topic = "dial/genset/" + DEVICE_ID;

    subscribe_topic = "dial/genset/" + DEVICE_ID + "/state";
    
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

  bool dataChanged = false;

  if (doc.containsKey("enabled")) {
    genSetEnabled = doc["enabled"];
    dataChanged = true;
  }

  if (doc.containsKey("rpm")) {
    genSetRPM = doc["rpm"];
    dataChanged = true;
  }

  if (doc.containsKey("efficiency")) {
    genSetEfficiency = doc["efficiency"];
    dataChanged = true;
  }
  
  if (dataChanged) {
    drawGenSetPage();
  }
}

bool reconnect() {
  M5Dial.Display.fillScreen(COLOR_BG);
  M5Dial.Display.setTextSize(1.2);
  M5Dial.Display.drawString("Connecting MQTT...", 120, 60);
  M5Dial.Display.drawString(mqtt_server, 120, 85);
  M5Dial.Display.drawString("Port: " + String(mqtt_port), 120, 110);
  
  Serial.println("\n=== MQTT Connection Attempt ===");
  Serial.print("Server: ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(mqtt_port);
  Serial.print("Username: ");
  Serial.println(mqtt_username);
  Serial.print("Client ID: ");
  Serial.println(CLIENT_ID);
  Serial.print("Publish Topic: ");
  Serial.println(publish_topic);
  Serial.print("Subscribe Topic: ");
  Serial.println(subscribe_topic);
  
  bool connected = client.connect(CLIENT_ID.c_str(), mqtt_username, mqtt_password);
  
  if (connected) {
    Serial.println("MQTT Connected Successfully!");
    
    M5Dial.Display.fillScreen(COLOR_BG);
    M5Dial.Display.setTextSize(1.5);
    M5Dial.Display.drawString("MQTT Connected!", 120, 100);
    M5Dial.Display.drawString("ID: " + DEVICE_ID, 120, 130);
    delay(1500);

    if (client.subscribe(subscribe_topic.c_str())) {
      Serial.println("Subscribed to: " + subscribe_topic);
    } else {
      Serial.println("Failed to subscribe to: " + subscribe_topic);
    }

    publishGenSetData();
    return true;
    
  } else {
    int state = client.state();
    Serial.print("✗ MQTT Connection Failed! State: ");
    Serial.println(state);
    
    M5Dial.Display.fillScreen(COLOR_BG);
    M5Dial.Display.setTextSize(1.2);
    M5Dial.Display.drawString("MQTT Failed!", 120, 60);

    String errorMsg = "Error: ";
    switch(state) {
      case -4: errorMsg += "Timeout"; break;
      case -3: errorMsg += "Lost"; break;
      case -2: errorMsg += "Failed"; break;
      case -1: errorMsg += "Disconnected"; break;
      case 1: errorMsg += "Bad Protocol"; break;
      case 2: errorMsg += "Bad ClientID"; break;
      case 3: errorMsg += "Unavailable"; break;
      case 4: 
        errorMsg += "Bad Credentials"; 
        Serial.println("! Check username/password");
        break;
      case 5: 
        errorMsg += "Unauthorized";
        Serial.println("! Authorization failed");
        break;
      default: errorMsg += String(state); break;
    }
    
    M5Dial.Display.drawString(errorMsg, 120, 90);
    M5Dial.Display.drawString("Check credentials", 120, 120);

    M5Dial.Display.setTextSize(1.0);
    M5Dial.Display.drawString("User: " + String(mqtt_username), 120, 150);
    M5Dial.Display.drawString("ID: " + DEVICE_ID, 120, 170);
    
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
  Serial.println("\n\n=== M5Dial GenSet Controller ===");
  Serial.println("Version: 1.0 (API Spec Compliant + TLS Verified)");
  Serial.println("API Endpoint");
  Serial.println("Topic: dial/genset/<DEVICE_ID>");
  
  setup_wifi();

  espClient.setCACert(root_ca);

  espClient.setTimeout(15);
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(512);
  client.setKeepAlive(60);

  if (!reconnect()) {
    M5Dial.Display.fillScreen(COLOR_BG);
    M5Dial.Display.setTextSize(1.2);
    M5Dial.Display.drawString("Cannot connect", 120, 100);
    M5Dial.Display.drawString("to MQTT server", 120, 125);
    M5Dial.Display.drawString("Check Serial Monitor", 120, 150);
    delay(5000);
  }
  
  drawGenSetPage();
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    M5Dial.Display.fillScreen(COLOR_BG);
    M5Dial.Display.drawString("WiFi Lost!", 120, 120);
    delay(1000);
    setup_wifi();
  }

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

  long newPosition = M5Dial.Encoder.read();
  
  if (newPosition != oldPosition) {
    if (isEditing) {
      int delta = newPosition - oldPosition;
      oldPosition = newPosition;
      adjustValue(delta);
      drawGenSetPage();
    } else {
      int newIndex = (newPosition / 4) % GENSET_SETTING_COUNT;
      int oldIndex = (oldPosition / 4) % GENSET_SETTING_COUNT;
      if (newIndex != oldIndex) {
        currentSettingIndex = ((newIndex % GENSET_SETTING_COUNT + GENSET_SETTING_COUNT) % GENSET_SETTING_COUNT);
        oldPosition = newPosition;
        drawGenSetPage();
      }
    }
  }

  if (M5Dial.BtnA.wasPressed()) {
    if (isEditing && currentSettingIndex != GENSET_CONFIRM) {

      isEditing = false;
      publishGenSetData();
      drawGenSetPage();
    } else if (currentSettingIndex == GENSET_CONFIRM) {

      publishGenSetData();
      M5Dial.Display.fillScreen(COLOR_BG);
      M5Dial.Display.drawString("Settings Sent!", 120, 120);
      delay(1000);
      drawGenSetPage();
    } else {

      isEditing = true;
      drawGenSetPage();
    }
  }
  
  delay(10);
}

void publishGenSetData() {
  if (!client.connected()) {
    Serial.println("[PUBLISH] Cannot publish: MQTT not connected");
    return;
  }

  StaticJsonDocument<128> doc;
  doc["powerLimit"] = genSetPowerLimit;
  
  char buffer[128];
  size_t n = serializeJson(doc, buffer);

  bool success = client.publish(publish_topic.c_str(), buffer, false);
  
  if (success) {
    Serial.println("[PUBLISH] Success");
    Serial.print("  Topic: ");
    Serial.println(publish_topic);
    Serial.print("  Payload: ");
    Serial.println(buffer);
  } else {
    Serial.println("[PUBLISH] Failed!");
  }
}

void adjustValue(int delta) {
  float increment = 0.1;
  switch (currentSettingIndex) {
    case GENSET_POWER_LIMIT:
      genSetPowerLimit += delta * increment; // 0.1 kW step
      genSetPowerLimit = constrain(genSetPowerLimit, 0.0, 100.0);
      break;
  }
}

void drawGenSetPage() {
  M5Dial.Display.fillScreen(COLOR_BG);

  M5Dial.Display.fillRect(0, 0, 240, 40, COLOR_GENSET);
  M5Dial.Display.setTextColor(COLOR_BG);
  M5Dial.Display.setTextSize(1.5);
  M5Dial.Display.drawString("GenSet Settings", 120, 20);
  
  M5Dial.Display.setTextColor(COLOR_TEXT);

  int yStart = 60;
  int ySpacing = 30;

  drawSettingItem("Limit (kW)", String(genSetPowerLimit, 1), yStart, 
                  currentSettingIndex == GENSET_POWER_LIMIT, isEditing);

  M5Dial.Display.drawLine(20, yStart + 25, 220, yStart + 25, 0x39C7);

  M5Dial.Display.setTextSize(1.3);
  M5Dial.Display.setTextColor(0x7BEF);
  M5Dial.Display.drawString("Status", 120, yStart + 40);

  M5Dial.Display.setTextSize(1.5);
  String statusText = genSetEnabled ? "ON" : "OFF";
  uint16_t statusColor = genSetEnabled ? COLOR_ON : COLOR_OFF;
  M5Dial.Display.setTextColor(statusColor);
  M5Dial.Display.drawString("Status: " + statusText, 120, yStart + 65);

  M5Dial.Display.setTextColor(COLOR_TEXT);
  M5Dial.Display.setTextSize(1.3);
  M5Dial.Display.drawString("RPM: " + String(genSetRPM, 0), 120, yStart + 90);

  M5Dial.Display.drawString("Efficiency: " + String(genSetEfficiency, 1) + "%", 120, yStart + 110);

  M5Dial.Display.setTextSize(1.5);
  M5Dial.Display.setTextColor(currentSettingIndex == GENSET_CONFIRM ? COLOR_GENSET : COLOR_TEXT);
  M5Dial.Display.drawString("Send Now", 120, yStart + 140);

  M5Dial.Display.setTextSize(1.0);
  String statusLine = "ID:" + DEVICE_ID;
  if (!client.connected()) {
    statusLine += " [OFFLINE]";
    M5Dial.Display.setTextColor(0xF800);
  } else {
    M5Dial.Display.setTextColor(0x07E0);
  }
  M5Dial.Display.drawString(statusLine, 120, 230);
}

void drawSettingItem(const char* label, String value, int y, bool selected, bool editing) {
  uint16_t textColor = COLOR_TEXT;
  uint16_t valueColor = COLOR_TEXT;
  
  if (selected) {
    if (editing) {
      M5Dial.Display.fillRect(10, y - 12, 220, 24, COLOR_EDITING);
      valueColor = COLOR_VALUE_EDIT;
    } else {
      textColor = valueColor = COLOR_GENSET;
    }
  }
  
  M5Dial.Display.setTextColor(textColor);
  M5Dial.Display.setTextSize(1.3);
  M5Dial.Display.drawString(label, 60, y);
  M5Dial.Display.setTextColor(valueColor);
  M5Dial.Display.setTextSize(1.5);
  M5Dial.Display.drawString(value, 180, y);
}
