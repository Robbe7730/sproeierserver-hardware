#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "settings.h"

WiFiClient wifi_client;

PubSubClient client(wifi_client);

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    randomSeed(micros());
    Serial.println("Connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}


void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = MQTT_CLIENT_ID;
    
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("Connected");
      client.subscribe(MQTT_TOPIC_SPRINKLER);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(", waiting 5 seconds...");
      delay(5000);
    }
  }
}

void set_active_sprinkler(byte sprinkler_id) {
    digitalWrite(SPRINKLER_ONE, (sprinkler_id == 1) ? HIGH : LOW);
    digitalWrite(SPRINKLER_TWO, (sprinkler_id == 2) ? HIGH : LOW);
    digitalWrite(SPRINKLER_THREE, (sprinkler_id == 3) ? HIGH : LOW);
}

void callback(char* topic, byte *payload, unsigned int payloadLength) {
    Serial.println("-------new message from broker-----");
    Serial.print("topic:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, payloadLength);
    Serial.println();

    if (strncmp(topic, MQTT_TOPIC_SPRINKLER, strlen(MQTT_TOPIC_SPRINKLER)) == 0) {
        set_active_sprinkler(payload[0] - '0');
    }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SPRINKLER_ONE, OUTPUT);
  pinMode(SPRINKLER_TWO, OUTPUT);
  pinMode(SPRINKLER_THREE, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(115200);
  Serial.setTimeout(500);

  setup_wifi();

  client.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
  client.setCallback(callback);
 
  reconnect();
  
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
   client.loop();
}
