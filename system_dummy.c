#include <stdint.h>
#include "nrf52833.h"

uint32_t SystemCoreClock = 64000000;

void SystemInit(void)
{
    /* Enable the FPU if the compiler used hard-float */
    #if (__FPU_USED == 1)
        SCB->CPACR |= (3UL << 20) | (3UL << 22);
        __DSB();
        __ISB();
    #endif
}
