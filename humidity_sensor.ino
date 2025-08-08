#include "secrets.h"
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Set to your sensor/hardware type
#define DHTPIN    4
#define DHTTYPE   DHT11
#define LCD_PORT  0x27

#define MQTT_MAX_PACKET_SIZE  1024

// set the LCD number of columns and rows
const int lcdColumns = 16;
const int lcdRows = 2;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_password = MQTT_PASSWORD;

const char* sub_topic = "homeassistant/test";
const char* topic_base = "homeassistant/sensor/";

char client_id[20];

// Create our dependencies
WiFiClient wifiClient;
PubSubClient client(wifiClient);
LiquidCrystal_I2C lcd(LCD_PORT, lcdColumns, lcdRows);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  dht.begin();
  
  connectWiFi();
  client.setBufferSize(MQTT_MAX_PACKET_SIZE);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // read the sensor and publish every 30 seconds
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 30000) 
  {
    lastMsg = now;
    checkTemperature(client_id);
  }
}

void connectWiFi() {
  lcd.setCursor(0, 0);
  // Set WiFi to station mode and disconnect from any existing network
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Connect to Wi-Fi network
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

   // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // create a unique client id from the mac address
  String clientId = WiFi.macAddress();
  clientId.replace(":", "");
  strcpy(client_id, clientId.c_str());

  ShowIdentityOnDisplay();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Example: Respond to a specific message
  if (strcmp(topic, sub_topic) == 0 && message == "identify") {
    Serial.println("Received 'identify'");
    ShowIdentityOnDisplay();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect
    if (client.connect(client_id, mqtt_user, mqtt_password)) {
      Serial.println("Connected. Publishing discovery message.");
      // Once connected, publish an announcement...
      publishDiscoveryMessage(client_id);
      // ... and subscribe to a topic
      client.subscribe(sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void checkTemperature(char* clientid)
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!")); 
    return;
  }
  /*
  Serial.print(F("Humidity: ")); 
  Serial.print(humidity); 
  Serial.print(F("% Temperature: ")); 
  Serial.print(temperature); 
  Serial.println(F("°C"));
  */
  ShowHumidity(humidity, temperature);

  // Convert the float to a char array
  char humidity_str[8];
  dtostrf(humidity, 4, 2, humidity_str);

  // Publish the humidity reading to the state topic
  Serial.print("Publishing humidity: ");
  Serial.println(humidity_str);
  client.publish(createStateTopic(clientid).c_str(), humidity_str);
}

String createDiscoveryTopic(String clientId)
{
  String discoveryTopic = topic_base;
  discoveryTopic += clientId;;
  discoveryTopic += "/humidity/config";

  return discoveryTopic;
}

String createStateTopic(String clientId)
{
  String stateTopic = topic_base;
  stateTopic += clientId;
  stateTopic += "/humidity/state";

  return stateTopic;
}

void publishDiscoveryMessage(String clientId) 
{ 
  // Create discovery topic
  String discovery = createDiscoveryTopic(clientId);
  const char* discovery_topic = discovery.c_str();

  // Create state topic
  String state = createStateTopic(clientId);
  const char* state_topic = state.c_str();

  // Build the JSON payload
  char payload[512];
  snprintf(payload, sizeof(payload), 
    "{\"name\": \"ESP32 Humidity\", \"device_class\": \"humidity\", \"state_topic\": \"%s\", \"unit_of_measurement\": \"%%\", \"unique_id\": \"humidity_sensor_%s\", \"device\": {\"identifiers\": [\"%s\"], \"name\": \"My ESP32 Device\", \"model\": \"ESP32\", \"manufacturer\": \"Jeff Shannon\"}}",
    state_topic, clientId, clientId);

  // Publish the discovery message
  if (client.publish(discovery_topic, payload, true))
  {
    Serial.println("Discovery message published.");
  }
  else
  {
    Serial.println("Discovery message failed to publish.");
    Serial.print("discovery_topic: ");
    Serial.println(discovery_topic);
    Serial.print("payload: ");
    Serial.println(payload);
  }
}

void  ShowIdentityOnDisplay()
{
  lcd.setCursor(0, 0);
  lcd.print("ID:");
  lcd.print(client_id);
  lcd.setCursor(0, 1);
  lcd.print("IP:");
  lcd.print(WiFi.localIP());
}

void ShowHumidity(float humidity, float temperature)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%"); 
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");
}