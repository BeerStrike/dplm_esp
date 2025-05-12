#ifndef MAIN_UART_H_
#define MAIN_UART_H_
#include "esp_err.h"

#define UART_LOG_TAG "UART"
#define BUF_SIZE (1024)
#define RX_BUF_SIZE (1024)

esp_err_t uart_init();

#endif /* MAIN_UART_H_ */
