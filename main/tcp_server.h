#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_
#include "globals.h"

#define TCP_SERVER_LOG_TAG "TCP server"
#define TCP_SERVER_RX_BUFFER_SIZE 1024
#define TCP_SERVER_PORT 1917

void start_tcp_server();
void tcp_server_task(void *params);
void tcp_request_handle_cycle(int sct);
int tcp_server_init();
void stop_tcp_server();
#endif /* TCP_SERVER_H_ */
