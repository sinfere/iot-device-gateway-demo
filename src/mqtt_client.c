#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_client.h"

volatile MQTTAsync_token deliveredtoken;

int disc_finished = 0;
int subscribed = 0;
int finished = 0;

static MQTTAsync mqttClient = NULL;
static mqtt_client_context_t* client_ctx = NULL;

void on_conn_lost(void *context, char *cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    LOGE("[mqtt] conn lost");
    if (cause) {
        LOGE("cause: %s", cause);
    }
        
    LOGI("[mqtt] reconnecting");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        LOGE("[mqtt] failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}

void on_disconnect(void* context, MQTTAsync_successData* response)
{
    LOGI("[mqtt] disconnect success");
    disc_finished = 1;
}


void on_subscribe(void* context, MQTTAsync_successData* response)
{
    LOGI("[mqtt] subscribe succeess");
    subscribed = 1;
}

void on_subscribe_fail(void* context, MQTTAsync_failureData* response)
{
    LOGE("[mqtt] subscribe failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}


void on_connect_fail(void* context, MQTTAsync_failureData* response)
{
    LOGE("[mqtt] connect failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}


void on_connect(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    LOGI("[mqtt] connect success");
    LOGI("[mqtt] subscribing to topic [%s] for client [%s] using QoS%d", client_ctx->read_topic, client_ctx->client_id, client_ctx->qos);

    opts.onSuccess = on_subscribe;
    opts.onFailure = on_subscribe_fail;
    opts.context = client;

    deliveredtoken = 0;

    if ((rc = MQTTAsync_subscribe(client, client_ctx->read_topic, client_ctx->qos, &opts)) != MQTTASYNC_SUCCESS) {
        LOGE("[mqtt] failed to start subscribe, return code %d", rc);
        exit(EXIT_FAILURE);
    }
}


void mqtt_client_boot(mqtt_client_context_t* ctx)
{
    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    client_ctx = ctx;

    MQTTAsync_create(&client, ctx->server_addr, ctx->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    mqttClient = client;

    if (ctx->on_message_receive != NULL) {
        MQTTAsync_setCallbacks(client, client, on_conn_lost, ctx->on_message_receive, NULL);
    }


    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = on_connect;
    conn_opts.onFailure = on_connect_fail;
    conn_opts.context = client;
    conn_opts.username = ctx->username;
    conn_opts.password = ctx->password;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        LOGE("[mqtt] failed to start connect, return code %d", rc);
        exit(EXIT_FAILURE);
    }

    mqttClient = client;
    
}

int mqtt_client_destroy() 
{
    int rc;

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    disc_opts.onSuccess = on_disconnect;

    if ((rc = MQTTAsync_disconnect(mqttClient, &disc_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start disconnect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    MQTTAsync_destroy(&mqttClient);

    return rc;
}

int mqtt_client_write(char* topic, char* payload) 
{
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message message = MQTTAsync_message_initializer;
    int rc;

    message.payload = payload;
    message.payloadlen = (int)strlen(payload);
    message.qos = client_ctx->qos;
    message.retained = 0;
    deliveredtoken = 0;

    if ((rc = MQTTAsync_sendMessage(mqttClient, topic, &message, &opts)) != MQTTASYNC_SUCCESS) {
        LOGE("[mqtt] failed to start sendMessage, return code %d", rc);
    }    

    return rc;
}