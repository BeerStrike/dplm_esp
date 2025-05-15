#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_
#include "esp_err.h"

#define TCP_SERVER_LOG_TAG "TCP server"
#define TCP_SERVER_RX_BUFFER_SIZE 1024
#define TCP_SERVER_PORT 1917

esp_err_t start_tcp_server(int port);
esp_err_t stop_tcp_server();
esp_err_t send_to_pc();

#endif /* TCP_SERVER_H_ */
