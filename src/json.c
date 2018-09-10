#include "json.h"
#include "common.h"

void parse_z3_gateway_device_joinded_message_example() {
    char* json = "{\"nodeId\":\"0xXXXX\",\"deviceState\":1,\"deviceType\":\"1\",\"timeSinceLastMessage\":10,\"deviceEndpoint\":{\"eui64\":\"0xXXXXXXXXXXXXXXXX\",\"endpoint\":0,\"clusterInfo\":[{\"clusterId\":\"0xXXXX\",\"clusterType\":\"In\"}]}}";

    struct json_object* root_jo = json_tokener_parse(json);

    struct json_object* node_id_jo;
    json_pointer_get(root_jo, "/nodeId", &node_id_jo);
    const char* node_id = json_object_get_string(node_id_jo);
    LOGI("[json] parse example: node_id = %s", node_id);  

    struct json_object* eui_jo;
    json_pointer_get(root_jo, "/deviceEndpoint/eui64", &eui_jo);
    const char* eui = json_object_get_string(eui_jo);   
    LOGI("[json] parse example: eui = %s", eui);  

    struct json_object* endpoint_jo;
    json_pointer_get(root_jo, "/deviceEndpoint/endpoint", &endpoint_jo);
    int32_t endpoint = json_object_get_int(endpoint_jo);        

    LOGI("[json] parse example: endpoint = %d", endpoint);    
}