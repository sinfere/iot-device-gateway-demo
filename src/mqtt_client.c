#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_client.h"

volatile MQTTAsync_token deliveredtoken;

int disc_finished = 0;
int subscribed = 0;
int finished = 0;

void on_conn_lost(void *context, char *cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    printf("\nConnection lost\n");
    if (cause) {
        printf("     cause: %s\n", cause);
    }
        
    printf("Reconnecting\n");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}

void on_disconnect(void* context, MQTTAsync_successData* response)
{
    printf("Successful disconnection\n");
    disc_finished = 1;
}


void on_subscribe(void* context, MQTTAsync_successData* response)
{
    printf("Subscribe succeeded\n");
    subscribed = 1;
}

void on_subscribe_fail(void* context, MQTTAsync_failureData* response)
{
    printf("Subscribe failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}


void on_connect_fail(void* context, MQTTAsync_failureData* response)
{
    printf("Connect failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}


void on_connect(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    printf("Successful connection\n");

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n", READ_TOPIC, CLIENTID, QOS);
    opts.onSuccess = on_subscribe;
    opts.onFailure = on_subscribe_fail;
    opts.context = client;

    deliveredtoken = 0;

    if ((rc = MQTTAsync_subscribe(client, READ_TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start subscribe, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
}


MQTTAsync mqtt_setup(MQTTAsync_messageArrived* on_message_received)
{
    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(client, client, on_conn_lost, on_message_received, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = on_connect;
    conn_opts.onFailure = on_connect_fail;
    conn_opts.context = client;
    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    return client;
}

int mqtt_destroy(MQTTAsync client) 
{
    int rc;

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    disc_opts.onSuccess = on_disconnect;

    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start disconnect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    MQTTAsync_destroy(&client);

    return rc;
}

int mqtt_write(MQTTAsync client, char* topic, char* payload) 
{
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message message = MQTTAsync_message_initializer;
    int rc;

    message.payload = payload;
    message.payloadlen = (int)strlen(payload);
    message.qos = QOS;
    message.retained = 0;
    deliveredtoken = 0;

    if ((rc = MQTTAsync_sendMessage(client, topic, &message, &opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start sendMessage, return code %d\n", rc);
    }    

    return rc;
}