#include "compositor.h"

int main() {
    LumaCompositor comp{};
    luma_init(&comp);
    luma_run(&comp);
    return 0;
}




// #include "compositor.h"
// #include <iostream>

// int main()
// {
//     try
//     {
//         LumaCompositor compositor;
//         compositor.run();
//     } 
//     catch (const std::exception &e)
//     {
//         std::cerr << "Fatal error: " << e.what() << std::endl;
//         return 1;
//     }
//     return 0;
// }
