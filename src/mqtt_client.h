#include <MQTTAsync.h>

#define ADDRESS     "tcp://127.0.0.1:30000"
#define CLIENTID    "test"
#define READ_TOPIC  "test-c"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#define USERNAME    "root"
#define PASSWORD    "7"

typedef struct mqtt_client_context_t {
	MQTTAsync_messageArrived* on_message_receive;
} mqtt_client_context_t;

#define mqtt_client_context_initializer { NULL }

void mqtt_client_boot(mqtt_client_context_t* ctx);
int mqtt_client_write(char* topic, char* payload);
int mqtt_client_destroy();