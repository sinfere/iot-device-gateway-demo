#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ev.h>

#include "mqtt_client.h"
#include "iot_client.h"
#include "config.h"
#include "json.h"

ev_io stdin_w;

int on_mqtt_message_receive(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    LOGI("[main] on mqtt message receive: topic: %s, message: ", topicName);

    int i = 0;
    char* p = message->payload;
    for (i = 0; i < message->payloadlen; i++) {
        putchar(*p++);
    }
    putchar('\n');    

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    
    return 1;
}

void on_iot_client_connect() {
    LOGI("[main] on iot client connect");

    iot_client_write_login_frame(DEVICE_ID);
    LOGI("[main] login to iot");
}

void on_iot_client_message_receive(bkv* b) {
    LOGI("[main] on iot client message receive");

    dump_bkv(b);

    bkv_free(b);
}

void destroy() {
    mqtt_client_destroy();
}

// command line action handler
static void stdin_cb(EV_P_ ev_io *w, int revents)
{
    char action[99];
    fgets(action, 30, stdin);
    size_t ln = strlen(action)-1;
    if (action[ln] == '\n') {
        action[ln] = '\0';
    }

    printf("Action: [%s] \r\n", action);

    if (strcmp(action, "q") == 0 || strcmp(action, "Q") == 0) {
        destroy();
        exit(0);
    }

    if (strcmp(action, "1") == 0) {
        mqtt_client_write("w", "dd");
        return;
    }

    if (strcmp(action, "2") == 0) {
        char* content = "dd";
        buffer* b = buffer_new((u_int8_t *)content, strlen(content));
        iot_client_write(b);
        return;
    }   

    if (strcmp(action, "json-parse-1") == 0) {
        parse_z3_gateway_device_joinded_message_example();
        return;
    }        
}

static void boot_mqtt() {
    mqtt_client_context_t* mqtt_client_ctx = malloc(sizeof(mqtt_client_context_t));
    memset(mqtt_client_ctx, 0, sizeof(mqtt_client_context_t));
    mqtt_client_ctx->server_addr = MQTT_ADDRESS;
    mqtt_client_ctx->username = MQTT_USERNAME;
    mqtt_client_ctx->password = MQTT_PASSWORD;
    mqtt_client_ctx->client_id = MQTT_CLIENT_ID;
    mqtt_client_ctx->read_topic = MQTT_READ_TOPIC;
    mqtt_client_ctx->qos = MQTT_QOS;
    mqtt_client_ctx->on_message_receive = on_mqtt_message_receive;
    mqtt_client_boot(mqtt_client_ctx);
}

static void boot_iot() {
    iot_client_context_t* iot_client_ctx = malloc(sizeof(iot_client_context_t));
    memset(iot_client_ctx, 0, sizeof(iot_client_context_t));
    iot_client_ctx->gateway_server_ip = IOT_GATEWAY_IP;
    iot_client_ctx->gateway_server_port = IOT_GATEWAY_PORT;
    iot_client_ctx->on_connect = on_iot_client_connect;
    iot_client_ctx->on_message_receive = on_iot_client_message_receive;
    iot_client_boot(iot_client_ctx);
}

int main(int argc, char const *argv[])
{
    LOGI("[main] boot");

    boot_mqtt();

    boot_iot();
    
    usleep(500 * 1000L);

    struct ev_loop *loop = EV_DEFAULT;

    ev_io_init(&stdin_w, stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
    ev_io_start(EV_A_ &stdin_w);

    ev_loop(EV_A_ 0);

    destroy();

    return 0;
}

