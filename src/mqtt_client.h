#include <MQTTAsync.h>
#include "common.h"

typedef struct mqtt_client_context_t {
    char* server_addr;
    char* username;
    char* password;
    char* client_id;
    char* read_topic;
    int qos;
    int timeout;

	MQTTAsync_messageArrived* on_message_receive;
} mqtt_client_context_t;

// #define mqtt_client_context_initializer { NULL, NULL, NULL, NULL, NULL, 1, 10000, NULL }

void mqtt_client_boot(mqtt_client_context_t* ctx);
int mqtt_client_write(char* topic, char* payload);
int mqtt_client_destroy();