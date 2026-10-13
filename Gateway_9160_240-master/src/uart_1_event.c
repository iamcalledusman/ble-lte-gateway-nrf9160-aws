
#include <zephyr/kernel.h>
//#include <sys/printk.h>
#include <app_event_manager.h>

#include "uart_1_event.h"




static void log_uart_module_event(const struct app_event_header *aeh)
{
        struct uart_module_event * ue = cast_uart_module_event(aeh);

       // APP_EVENT_MANAGER_LOG(aeh, "val1=%c ", ue->data);

       APP_EVENT_MANAGER_LOG(aeh, "val1=%c ", ue->data);
}



APP_EVENT_TYPE_DEFINE(uart_module_event,                                                  /* Unique event name. */
                  log_uart_module_event,                                                  /* Function logging event data. */
                  NULL,                                                              /* No event info provided. */
                  APP_EVENT_FLAGS_CREATE(APP_EVENT_TYPE_FLAGS_INIT_LOG_ENABLE));     /* Flags managing event type. */