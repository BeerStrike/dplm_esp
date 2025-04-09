#include "uart.h"
#include "driver/uart.h"
#include "json_functions.h"
#include <string.h>

void UART_event_task(){
	uart_event_t event;
    uint8_t *rx_buff = (uint8_t *) malloc(RX_BUF_SIZE);
	while(1){
		if (xQueueReceive(uart_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
            switch (event.type) {
            	case UART_DATA:
            		 uart_read_bytes(UART_NUM_0, rx_buff, event.size, portMAX_DELAY);
            		 rx_buff[event.size]='\0';
            		 //ESP_LOGI(UART_LOG_TAG,"%s",rx_buff);
            		 char * resp=uart_setup_json_handler((char *)rx_buff);
            		 if(resp!=NULL){
						 uart_write_bytes(UART_NUM_0,resp,strlen(resp));
						 cJSON_free(resp);
            		 }
            		 break;
            	default:
            		break;
            }
		}
	}
    free(rx_buff);
	vTaskDelete(NULL);
}


void UART_init(){
	 uart_config_t uart_config = {
	        .baud_rate = 78800,
	        .data_bits = UART_DATA_8_BITS,
	        .parity = UART_PARITY_DISABLE,
	        .stop_bits = UART_STOP_BITS_1,
	        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	 };
	 uart_param_config(UART_NUM_0, &uart_config);
	 uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 100, &uart_queue, 0);
     xTaskCreate(UART_event_task, "UART event task", 2048, NULL, 12, NULL);
}
