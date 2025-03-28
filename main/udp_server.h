#ifndef UDP_SERVER_H_
#define UDP_SERVER_H_
#define UDP_SERVER_LOG_TAG "UDP server"
#define UDP_SERVER_RX_BUFFER_SIZE 1024
#define UDP_SERVER_PORT 1917

void start_udp_server();
void udp_server_task(void *params);
int udp_server_init();
void stop_udp_server();

#endif /* UDP_SERVER_H_ */
