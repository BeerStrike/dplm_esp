#ifndef MAIN_UART_H_
#define MAIN_UART_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define UART_LOG_TAG "uart_events"
#define BUF_SIZE (1024)
#define RX_BUF_SIZE (BUF_SIZE)

QueueHandle_t uart_queue;

void UART_init();

#endif /* MAIN_UART_H_ */
