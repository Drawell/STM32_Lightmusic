#include "main.h"

int main(void)
{

    SystemInit();
    init_LED_manager();
    init_controller();
    init_microphone_driver(&microphone_interrupt_handler);

    while (1)
    {
    }
}
