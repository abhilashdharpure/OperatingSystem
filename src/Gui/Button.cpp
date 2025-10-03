#include "Button.h"


Button::Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height, std::string label) :
x(x),
y(y),
width(width),
height(height),
label(label)
{

}


void Button::draw() {
    // Draw rectangle and label
}

bool Button::isClicked(int mouseX, int mouseY)
{
    return mouseX >= x && mouseX <= x + width &&
            mouseY >= y && mouseY <= y + height;
}