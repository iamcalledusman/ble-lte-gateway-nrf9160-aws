/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include "uart_1_event.h"
#include <app_event_manager.h>

 bool app_event_handler(const struct app_event_header *aeh)
{
	
   if (is_uart_module_event(aeh)) {
		struct uart_module_event * event= cast_uart_module_event(aeh);
   
     /* i comment it for time being ...

       for(char i=0; i<5; i++){
	 	printk("Uart 1 %c ",event->data[i]);

      }
    
       printk("\n");  */

       
		return false;
	}

	
	return false;
	
}

APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, uart_module_event);
