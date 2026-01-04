#include "time.h"
#include <arch/x86_64/pit.h>


uint64_t get_system_time(void) 
{
    get_system_time_us();
}
