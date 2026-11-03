#include "time.h"
#include <arch/x86_64/pit.h>


uint64_t get_system_time(void) 
{
    return get_system_time_us();
}

uint64_t get_system_time_ms(void) 
{
    return get_system_time_us() / 1000;
}
