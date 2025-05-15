#ifndef UDP_SERVER_H_
#define UDP_SERVER_H_
#include "esp_err.h"
#define UDP_SERVER_LOG_TAG "UDP server"
#define UDP_SERVER_RX_BUFFER_SIZE 1024

esp_err_t start_udp_server(int port);
esp_err_t stop_udp_server();

#endif /* UDP_SERVER_H_ */
