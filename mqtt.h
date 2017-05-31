/*
 * mqtt.h:
 *
 */

 #include <MQTTClient.h>

 #define ADDRESS          "tcp://test.mosquitto.org:1883"
 //#define ADDRESS        "tcp://localhost:1883"
 #define CLIENTID         "ExampleClientPub"
 #define TOPIC            "MQTT/card/test"
 #define SUBSCRIBE_TOPIC  "MQTT/card/test"
 #define PAYLOAD          "Hello World!"
 #define QOS              1
 #define TIMEOUT          10000L


// mqtt client status
typedef enum {
    MQTT_STATUS_INIT,
    MQTT_STATUS_PENDING,
    MQTT_STATUS_COMPLETE
} mqtt_status_enum;


// callback functions
void delivered(void *context, MQTTClient_deliveryToken dt);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost(void *context, char *cause);


// mqtt client
void init_MQTT();
int construct_MQTT_msg(int n, hash_type* history);
void send_MQTT_msg();
void check_send_status();
void destory_MQTT();
