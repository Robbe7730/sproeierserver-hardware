#include <ESP8266WiFi.h>

// Library name: PubSubClient
#include <PubSubClient.h>

// Library name: DHT sensor library
#include "DHT.h"

// Library name: elapsedMillis
#include <elapsedMillis.h>

#include "settings.h"

WiFiClient wifi_client;
PubSubClient client(wifi_client);
DHT dht(DHT11_PIN, DHT11);
elapsedMillis millis_since_last_status_update;

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
    Serial.println("IP address:");
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
  pinMode(DHT11_PIN, INPUT);

  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(115200);
  Serial.setTimeout(500);

  setup_wifi();

  client.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
  client.setCallback(callback);
 
  reconnect();
  
  digitalWrite(LED_BUILTIN, HIGH);

  dht.begin();
}

void send_float(const char* topic, float value) {
    char buffer[6] = {0};
    snprintf(buffer, 6, "%f", value);
    
    Serial.print("Sending \"");
    Serial.print(buffer);
    Serial.print("\" to ");
    Serial.println(topic);

    if (!client.publish(topic, buffer)) {
        Serial.print("Could not publish");
    }
}

void send_temperature(float temperature) {
    send_float(MQTT_TOPIC_TEMPERATURE, temperature);
}

void send_humidity(float humidity) {
    send_float(MQTT_TOPIC_HUMIDITY, humidity);
}

void loop() {
    client.loop();

    if (millis_since_last_status_update > STATUS_UPDATE_INTERVAL) {
        millis_since_last_status_update -= STATUS_UPDATE_INTERVAL;
        
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();
  
        Serial.print("Humidity: ");
        Serial.println(humidity);
        Serial.print("Temperature: ");
        Serial.println(temperature);

        send_temperature(temperature);
        send_humidity(humidity);
    }
}
