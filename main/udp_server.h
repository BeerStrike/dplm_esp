#ifndef UDP_SERVER_H_
#define UDP_SERVER_H_
#define UDP_SERVER_LOG_TAG "UDP server"
#define UDP_SERVER_RX_BUFFER_SIZE 1024
#define UDP_SERVER_PORT 1917

void udp_server_task(void *params);

#endif /* UDP_SERVER_H_ */
