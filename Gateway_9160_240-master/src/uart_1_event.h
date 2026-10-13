

#include <zephyr/kernel.h>
#include <app_event_manager.h>


enum uart_module_event_type {
	
	UART_EVT_RECIEVED,
        UART_EVT_NOT_RECIEVED,

};

struct uart_module_event {
        struct app_event_header header;
        enum uart_module_event_type type;

        /* Custom data fields. */
    
         char data[150];
  
};

APP_EVENT_TYPE_DECLARE(uart_module_event);


