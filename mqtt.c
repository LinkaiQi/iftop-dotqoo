/*
 * mqtt.c:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#include "mqtt.h"
#include "hash.h"
#include "iftop.h"
#include "addr_hash.h"

#define ADDRESS          "tcp://test.mosquitto.org:1883"
//#define ADDRESS        "tcp://localhost:1883"
#define CLIENTID         "ExampleClientPub"
#define TOPIC            "MQTT/card/test"
#define SUBSCRIBE_TOPIC  "MQTT/card/test"
#define PAYLOAD          "Hello World!"
#define QOS              1
#define TIMEOUT          10000L

MQTTClient client;
//MQTTClient_connectOptions conn_opts;
//MQTTClient_message pubmsg;
MQTTClient_deliveryToken token;

volatile MQTTClient_deliveryToken deliveredtoken;
// pointer to the constructed message
unsigned char *data;
int struct_size;
long msg_len;
// mqtt status
mqtt_status_enum status;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void init_MQTT() {
    //MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;;
    //MQTTClient_message pubmsg = MQTTClient_message_initializer;
    //pubmsg = MQTTClient_message_initializer;
    //MQTTClient_deliveryToken token;
    int rc;
    struct_size = sizeof(long)*2 + sizeof(short int)*3 + sizeof(double long)*2;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    // set call backs (Asynchronous)
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    // connect to broker
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    // subscribe the client to a topic
    MQTTClient_subscribe(client, SUBSCRIBE_TOPIC, QOS);
    // change status
    status = MQTT_STATUS_INIT;
}


int construct_MQTT_msg(int n, hash_type* history) {
    addr_pair* addr_info;
    history_type* stat;
    hash_node_type* node = NULL;
    //int struct_size = sizeof(long)*2 + sizeof(short int)*3 + sizeof(double long)*2;
    if (status == MQTT_STATUS_PENDING)
        return 1;
    // allocate string data
    data = (unsigned char *)calloc(1, struct_size * n);
    unsigned char *current = data;
    // change status
    status = MQTT_STATUS_PENDING;

    // go through the history hash table
    hash_next_item(history, &node);
    while(node != NULL) {
        hash_node_type* next = node;
        history_type* d = (history_type*)node->rec;
        hash_next_item(history, &next);

        addr_info = (addr_pair*)node->key;
        stat = (history_type*)node->rec;

        //memcpy(data, &p1, sizeof(structure));
        //memcpy(data+sizeof(structure), &p2, sizeof(structure));

        // src ip-address
        memcpy(current, &(addr_info->src.s_addr), sizeof(long));
        current += sizeof(long);
        // src port-number
        memcpy(current, &(addr_info->src_port), sizeof(short int));
        current += sizeof(short int);
        // dst ip-address
        memcpy(current, &(addr_info->dst.s_addr), sizeof(long));
        current += sizeof(long);
        // dst port-number
        memcpy(current, &(addr_info->dst_port), sizeof(short int));
        current += sizeof(short int);
        // protocol
        memcpy(current, &(addr_info->protocol), sizeof(short int));
        current += sizeof(short int);
        // total data send/rev
        memcpy(current, &(stat->total_sent), sizeof(double long));
        current += sizeof(double long);
        memcpy(current, &(stat->total_recv), sizeof(double long));
        current += sizeof(double long);

        node = next;
    }

    msg_len = struct_size * n;
    // return in success
    return 0;
}


void send_MQTT_msg() {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    //MQTTClient_deliveryToken token;

    pubmsg.payload = data;
    pubmsg.payloadlen = msg_len;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;

    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    printf("Waiting for publication\n"
            "on topic %s for client with ClientID: %s\n",
            TOPIC, CLIENTID);
    //construct_msg();
}


void check_send_status() {
    if (deliveredtoken == token) {          // msg sent successful
        deliveredtoken = 0;
        // change status
        status = MQTT_STATUS_COMPLETE;
        // free string data buffer
        free(data);
        printf("msg sent successful\n");
    }
}


void destory_MQTT() {
    // Waiting for publication of last msg
    while (status == MQTT_STATUS_PENDING) {
        check_send_status();
    }
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}
