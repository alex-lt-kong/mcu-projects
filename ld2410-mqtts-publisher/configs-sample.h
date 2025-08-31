#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourSecretPassword"

#define MQTT_CLIENT_ID "MyMqttClient"
#define MQTT_SERVER "mqtt.example.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC "/mqtt/topic"
#define MQTT_USERNAME "user"
#define MQTT_PASSWORD "pa$$w0rd"
#define MQTT_PUBLISH_INTERVAL_MS 1000

const char* root_ca = R"(
-----BEGIN CERTIFICATE-----
MIIDijCCAxGgAwIBAgISBTSvSClNntOf2PNprdOvDR1aMAoGCCqGSM49BAMDMDIx
...
zrzAmJMVsynGIOckYCzc3VS9tfRUFnsfLUatPWNSfUVt2aSHKS8SfN0GVBt5sw==
-----END CERTIFICATE-----
)";
