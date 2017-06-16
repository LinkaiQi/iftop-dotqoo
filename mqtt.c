/*
 * mqtt.c:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <MQTTClient.h>

#include "zlib.h"

#include "mqtt.h"
#include "hash.h"
#include "iftop.h"
#include "addr_hash.h"

MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
//MQTTClient_message pubmsg;
MQTTClient_deliveryToken token = -1;

volatile MQTTClient_deliveryToken deliveredtoken = 0;
// pointer to the constructed message
unsigned char *data;
int struct_size;
long msg_len;
//int client_id;
char id[32];
// mqtt status
mqtt_status_enum status;
mqtt_connect_enum connection;
//topic
char my_send_topic[32];
char my_retrieve_topic[32];
// time
time_t last_try;
// zlib
unsigned char *out;
unsigned long compressed_size;



void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf(" Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}


int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;
    // printf("Message arrived,  Topic: %s, Message: ", topicName);
    printf("\n <<< Receive data request, Topic: %s, Message: ", topicName);
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    // Call tick send MQTT msg
    tick(1);
    return 1;
}


void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
    connection = MQTT_CONNECT_OFF;
    // try to reconnect
    last_try = time(NULL);
    connect_and_subscribe();
}


void init_MQTT() {
    //MQTTClient client;
    //MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    //MQTTClient_message pubmsg = MQTTClient_message_initializer;
    //pubmsg = MQTTClient_message_initializer;
    //MQTTClient_deliveryToken token;
    struct_size = sizeof(long)*2 + sizeof(short int)*3 + sizeof(unsigned long long)*2 + sizeof(time_t)*2;

    // initalize status
    status = MQTT_STATUS_INIT;
    connection = MQTT_CONNECT_OFF;
    // get device ID
    get_device_id();
    // strcpy(id, "123456");
    // construct topic
    construct_topic();

    MQTTClient_create(&client, ADDRESS, id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    // set call backs (Asynchronous)
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    // connect & subscribe
    connect_and_subscribe();
    last_try = time(NULL);
}


void check_MQTT_connection(time_t t) {
    if (connection == MQTT_CONNECT_OFF && t - last_try > RECONNECT_INTERVAL) {
        last_try = t;
        connect_and_subscribe();
    }
}


int connect_and_subscribe() {
    int rc;

    if (status == MQTT_STATUS_PENDING) {
        free(data);
        status = MQTT_STATUS_INIT;
    }

    // connect to broker
    printf("Connecting... (MQTT)\n");
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return 1;
    }
    printf("Connection established! (MQTT)\n");
    // subscribe the client to a topic
    MQTTClient_subscribe(client, BROADCST, QOS);
    MQTTClient_subscribe(client, my_retrieve_topic, QOS);
    connection = MQTT_CONNECT_ON;
    return 0;
}


void get_device_id() {
    FILE *fp;
    //char id[32];
    int n_id;
    char *pos;

    /* Open the command for reading. */
    fp = popen("/bin/boxid", "r");
    if (fp == NULL) {
        printf("Failed to run command 'boxid'\n" );
        exit(1);
    }
    /* Read the output a line at a time - output it. */
    //while (fgets(path, sizeof(path), fp) != NULL) {
    //    printf("%s", path);
    //}
    if (fgets(id, sizeof(id), fp) == NULL) {
        printf("Failed to read command 'boxid' output\n" );
        exit(1);
    }
    // remove '\n' character
    if ((pos=strchr(id, '\n')) != NULL) {
        *pos = '\0';
    }
    /* close */
    pclose(fp);
}


void construct_topic() {
    //char str_id[32];

    //sprintf(str_id, "%d", client_id);
    // NWSTAT/AUTO/{ID}
    strcpy(my_send_topic, SEND_TOPIC);
    strcpy(my_send_topic+strlen(SEND_TOPIC), id);
    // NWSTAT/RETV/{ID}
    strcpy(my_retrieve_topic, RETRIEVE);
    strcpy(my_retrieve_topic+strlen(RETRIEVE), id);
}


int construct_MQTT_msg(int n, hash_type* history) {
    addr_pair* addr_info;
    history_type* stat;
    hash_node_type* node = NULL;
    //int struct_size = sizeof(long)*2 + sizeof(short int)*3 + sizeof(unsigned long long)*2;
    if (status == MQTT_STATUS_PENDING || connection == MQTT_CONNECT_OFF) {
        printf("return construct_MQTT_msg\n");
        return 1;
    }
    // allocate string data
    data = (unsigned char *)calloc(1, struct_size * n);
    // change status
    status = MQTT_STATUS_PENDING;

    unsigned char *current = data;
    // go through the history hash table
    hash_next_item(history, &node);
    while(node != NULL) {
        hash_node_type* next = node;
        history_type* d = (history_type*)node->rec;
        hash_next_item(history, &next);

        addr_info = (addr_pair*)node->key;
        stat = (history_type*)node->rec;

        if (stat->approval) {
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
            memcpy(current, &(stat->total_sent), sizeof(unsigned long long));
            current += sizeof(unsigned long long);
            memcpy(current, &(stat->total_recv), sizeof(unsigned long long));
            current += sizeof(unsigned long long);
            // add timestamp
            memcpy(current, &(stat->create_time), sizeof(time_t));
            current += sizeof(time_t);
            memcpy(current, &(stat->last_update_time), sizeof(time_t));
            current += sizeof(time_t);
        }

        node = next;
    }

    msg_len = struct_size * n;

    compress_msg();

    // return in success
    return 0;
}


void compress_msg(/* arguments */) {
    int ret, flush;
    unsigned have;
    z_stream strm;
    //unsigned char in[CHUNK];
    //unsigned char out[msg_len];
    out = (unsigned char *)calloc(1, msg_len);
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    //ret = deflateInit(&strm, Z_BEST_COMPRESSION);
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        fprintf(stderr, "deflateInit error");
        exit(1);
    }
    strm.avail_in = (uInt)msg_len; // size of input, string + terminator
    strm.next_in = (Bytef *)data; // input char array
    strm.avail_out = (uInt)msg_len; // size of output
    strm.next_out = (Bytef *)out; // output char array

    // the actual compression work.
    //deflateInit(&strm, Z_BEST_COMPRESSION);
    deflate(&strm, Z_FINISH);
    deflateEnd(&strm);

    compressed_size = strm.total_out;

    printf(" Compress MQTT Message  %ld byte -> %ld byte\n", msg_len, strm.total_out);
}


void send_MQTT_msg() {
    if (connection == MQTT_CONNECT_OFF) {
        status = MQTT_STATUS_COMPLETE;
        free(data);
        free(out);
        printf("return send_MQTT_msg\n");
        return;
    }

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    //MQTTClient_deliveryToken token;

    pubmsg.payload = out;
    pubmsg.payloadlen = compressed_size;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;

    printf(" >>> Sending data (MQTT)\n");

    MQTTClient_publishMessage(client, my_send_topic, &pubmsg, &token);
    printf(" Waiting for publication "
            "Topic: %s ClientID: %s\n",
            my_send_topic, id);
    //construct_msg();
}


void check_status() {
    if (deliveredtoken == token) {          // msg sent successful
        deliveredtoken = 0;
        // change status
        free(data);
        free(out);
        // change status
        status = MQTT_STATUS_COMPLETE;
        printf(" Data sent successful\n\n");
    }
}


void destory_MQTT() {
    // Waiting for publication of last msg
    while (status == MQTT_STATUS_PENDING) {
        check_status();
    }
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}
