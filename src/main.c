#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ev.h>

#include "mqtt_client.h"
#include "iot_client.h"

ev_io stdin_w;

MQTTAsync mqttClient;

int on_mqtt_message_received(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int i;
    char* payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

    payloadptr = message->payload;
    for (i = 0; i < message->payloadlen; i++) {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

static void stdin_cb (EV_P_ ev_io *w, int revents)
{
    char action[20];
    fgets(action, 100, stdin);
    size_t ln = strlen(action)-1;
    if (action[ln] == '\n') {
        action[ln] = '\0';
    }

    printf("Action: [%s] \r\n", action);

    if (strcmp(action, "q") == 0 || strcmp(action, "Q") == 0) {
        exit(0);
    }

    if (strcmp(action, "1") == 0) {
        mqtt_write(mqttClient, "w", "dd");
    }

    if (strcmp(action, "2") == 0) {
        char* content = "dd";
        buffer* b = buffer_new((u_int8_t *)content, strlen(content));
        iot_client_write(b);
    }    
}

void destroy() {
    mqtt_destroy(mqttClient);
}

int main(int argc, char const *argv[])
{
    printf("boot\r\n");

    // mqttClient = mqtt_setup(on_mqtt_message_received);
    
    iot_client_boot();

    usleep(500 * 1000L);

    struct ev_loop *loop = EV_DEFAULT;
    ev_io_init(&stdin_w, stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
    ev_io_start(EV_A_ &stdin_w);

    ev_loop(EV_A_ 0);

    destroy();

    return 0;
}

