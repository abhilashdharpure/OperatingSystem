#pragma once

#include <string>

class Button {
public:
    Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height, std::string label = "");
    void draw();

    bool isClicked(int mouseX, int mouseY);

private:
    int x, y, width, height;
    std::string label;
};

