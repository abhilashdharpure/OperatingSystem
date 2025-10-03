#include "Button.h"


extern "C" void start_gui() {
    // Your GUI initialization and loop
    guiLoop();  // or whatever your main GUI function is
}

uint32_t* framebuffer = (uint32_t*)0xE0000000; // Example address
int screenWidth = 800;
int screenHight = 600;

void drawPixel(int x, int y, uint32_t color) {
    framebuffer[y * screenWidth + x] = color;
}



void guiLoop() {
    Button myButton(100, 100, 80, 30, "Click Me");

    while (true) {
        myButton.draw();
        // if (mouseClicked && myButton.isClicked(mouseX, mouseY)) {
        //     // Handle button click
        // }
    }
}