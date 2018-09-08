#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ev.h>

#include "mqtt_client.h"
#include "iot_client.h"

ev_io stdin_w;

int on_mqtt_message_receive(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void on_iot_client_connect() {
    LOGI("[main] on iot client connect");

}

void on_iot_client_message_receive(bkv* b) {
    LOGI("[main] on iot client message receive");

    bkv_free(b);
}

void destroy() {
    mqtt_client_destroy();
}

// command line action handler
static void stdin_cb(EV_P_ ev_io *w, int revents)
{
    char action[20];
    fgets(action, 100, stdin);
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
    }

    if (strcmp(action, "2") == 0) {
        char* content = "dd";
        buffer* b = buffer_new((u_int8_t *)content, strlen(content));
        iot_client_write(b);
    }    
}

int main(int argc, char const *argv[])
{
    LOGI("[main] boot");

    mqtt_client_context_t mqtt_client_ctx = mqtt_client_context_initializer;
    mqtt_client_ctx.on_message_receive = on_mqtt_message_receive;
    mqtt_client_boot(&mqtt_client_ctx);
    
    iot_client_context_t iot_client_ctx = iot_client_context_initializer;
    iot_client_ctx.on_connect = on_iot_client_connect;
    iot_client_ctx.on_message_receive = on_iot_client_message_receive;
    iot_client_boot(&iot_client_ctx);

    usleep(500 * 1000L);

    struct ev_loop *loop = EV_DEFAULT;
    ev_io_init(&stdin_w, stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
    ev_io_start(EV_A_ &stdin_w);

    ev_loop(EV_A_ 0);

    destroy();

    return 0;
}

