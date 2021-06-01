#define SPRINKLER_ONE D0
#define SPRINKLER_TWO D3
#define SPRINKLER_THREE D6

#define WIFI_SSID "MyWifi"
#define WIFI_PSK "MyPassword"

#define MQTT_BROKER_IP "192.168.0.100"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "sproeiers"
#define MQTT_USER "sproeiers"
#define MQTT_PASS "MyOtherPassword"

#define MQTT_TOPIC_SPRINKLER "sprinklers/active"
#define MQTT_TOPIC_TEMPERATURE "sprinklers/temperature"
#define MQTT_TOPIC_HUMIDITY "sprinklers/humidity"

#define DHT11_PIN D5

// In millis
#define STATUS_UPDATE_INTERVAL 60000
