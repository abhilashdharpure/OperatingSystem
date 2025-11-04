#include <stdio.h>
#include <wayland-client.h>
#include "client.h"
#include <string>
#include <iostream>

int main() {

    std::string displayName = "luma-0";
    Client client(displayName);

    client.Initialize();
    // struct wl_display *display = wl_display_connect(NULL);
    // if (!display)
    // {
    //     std::cout<<"Failed to connect to Wayland display"<<std::endl;
    //     fprintf(stderr, "Failed to connect to Wayland display.\n");
    //     return 1;
    // }

    // std::cout<<"Connection established!"<<std::endl;

    // fprintf(stderr, "Connection established!\n");

    // wl_display_disconnect(display);
    return 0;
}
