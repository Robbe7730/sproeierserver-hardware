#include <stdarg.h>

#include <ESP8266WiFi.h>

// Library name: PubSubClient
#include <PubSubClient.h>

// Library name: DHT sensor library
#include "DHT.h"

// Library name: elapsedMillis
#include <elapsedMillis.h>

#include "settings.h"

#ifdef SERIAL_ENABLED 
  #define PRINTLN(...)                              \
        serial_printf("%s:%d ", __FILE__, __LINE__);\
        serial_printf(__VA_ARGS__);                 \
        Serial.println()
#else
  #define PRINTLN(...)
#endif

WiFiClient wifi_client;
PubSubClient client(wifi_client);
DHT dht(DHT11_PIN, DHT11);
elapsedMillis millis_since_last_status_update;

void serial_printf() {}

void serial_printf(const char *fmt, ...) {
  char buff[SERIAL_PRINTF_BUFF_SIZE];
  
  va_list pargs;
  va_start(pargs, fmt);
  vsnprintf(buff, SERIAL_PRINTF_BUFF_SIZE, fmt, pargs);
  va_end(pargs);
  Serial.print(buff);
}

char* PUBSUB_ERRORS[10] = {
  "MQTT_CONNECTION_TIMEOUT",
  "MQTT_CONNECTION_LOST",
  "MQTT_CONNECT_FAILED",
  "MQTT_DISCONNECTED",
  "MQTT_CONNECTED",
  "MQTT_CONNECT_BAD_PROTOCOL",
  "MQTT_CONNECT_BAD_CLIENT_ID",
  "MQTT_CONNECT_UNAVAILABLE",
  "MQTT_CONNECT_BAD_CREDENTIALS",
  "MQTT_CONNECT_UNAUTHORIZED"
};

void setup_wifi() {
    delay(10);
    
    PRINTLN("Connecting to %s", WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
    
    PRINTLN("Connected");
    PRINTLN("IP address: %s", WiFi.localIP().toString().c_str());
}


void reconnect() {
  if(client.connected()) {
    client.disconnect();
  }
  
  while (!client.connected()) {
    PRINTLN("Attempting MQTT connection...");

    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      PRINTLN("Connected");
      client.subscribe(MQTT_TOPIC_SPRINKLER);
    } else {
      PRINTLN(
        "failed, rc=%d (%s), waiting 5 seconds...",
        client.state(),
        PUBSUB_ERRORS[client.state() + 4]
      );
      client.disconnect();
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
    PRINTLN("-------new message from broker-----");
    PRINTLN("topic: %s", topic);
    PRINTLN("data: %s", payload);
    PRINTLN();

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

  #ifdef SERIAL_ENABLED
  Serial.begin(115200);
  Serial.setTimeout(500);
  Serial.println();
  #endif // SERIAL_ENABLED

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
    
    PRINTLN("Sending \"%s\" to %s", buffer, topic);

    while(!client.publish(topic, buffer)) {
        PRINTLN("Could not publish, attempting reconnect");
        reconnect();
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
        digitalWrite(LED_BUILTIN, LOW);
        millis_since_last_status_update -= STATUS_UPDATE_INTERVAL;
        
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();

        PRINTLN("Humidity: %f", humidity);
        PRINTLN("Temperature: %f", temperature);

        send_temperature(temperature);
        send_humidity(humidity);
        digitalWrite(LED_BUILTIN, HIGH);
    }
}
