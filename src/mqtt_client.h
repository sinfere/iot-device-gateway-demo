#include <MQTTAsync.h>

#define ADDRESS     "tcp://127.0.0.1:30000"
#define CLIENTID    "test"
#define READ_TOPIC  "test-c"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#define USERNAME    "root"
#define PASSWORD    "7"

MQTTAsync mqtt_setup(MQTTAsync_messageArrived*);
int mqtt_write(MQTTAsync, char*, char*);
int mqtt_destroy(MQTTAsync);