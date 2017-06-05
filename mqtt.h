/*
 * mqtt.h:
 *
 */

 #include <MQTTClient.h>
 #include <time.h>

 #include "iftop.h"
 #include "hash.h"


 #define ADDRESS          "tcp://test.mosquitto.org:1883"
 //#define ADDRESS        "tcp://localhost:1883"
 //#define CLIENTID         "ExampleClientPub"
 #define QOS              1
 #define TIMEOUT          10000L

 #define SEND_TOPIC       "NWSTAT/AUTO/"
 #define RETRIEVE         "NWSTAT/RETV/"
 #define BROADCST         "NWSTAT/BROADCAST"
 /* set the reconnect internal to 10 mins
  * if the connection is lost, we will try to reconnect to the broker
  * in the next 10 mins */
 #define RECONNECT_INTERVAL 600


// mqtt client status
typedef enum {
    MQTT_STATUS_INIT,
    MQTT_STATUS_LOST,
    MQTT_STATUS_PENDING,
    MQTT_STATUS_COMPLETE
} mqtt_status_enum;

typedef enum {
    MQTT_CONNECT_ON,
    MQTT_CONNECT_OFF,
} mqtt_connect_enum;


// callback functions
void delivered(void *context, MQTTClient_deliveryToken dt);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost(void *context, char *cause);


// mqtt client
void init_MQTT(void);
void check_MQTT_connection(time_t t);
int connect_and_subscribe();
void get_device_id(void);
void construct_topic(void);
int construct_MQTT_msg(int n, hash_type* history);
void compress_msg(void);
void send_MQTT_msg(void);
void check_status(void);
void destory_MQTT(void);
