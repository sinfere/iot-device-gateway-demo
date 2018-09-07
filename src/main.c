#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mqtt_client.h"

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

static void getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return ;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return ;
}

int main(int argc, char const *argv[])
{
    printf("boot\r\n");

    mqttClient = mqtt_setup(on_mqtt_message_received);
    usleep(500 * 1000L);

    char action[20];
    do {
		getLine("\r\nAction: ", action, 20);

        if (strcmp(action, "q") == 0 || strcmp(action, "Q") == 0) {
            break;
        }

        if (strcmp(action, "1") == 0) {
            mqtt_write(mqttClient, "w", "dd");
        }

	} while (1);

    mqtt_destroy(mqttClient);

    return 0;
}

