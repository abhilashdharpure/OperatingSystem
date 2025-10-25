#pragma once



#define COLOR(r,g,b) ((b) | (g << 8) | (r << 16))
#define RGB(r, g, b) ((uint32_t)(((r) << 16) | ((g) << 8) | (b)))
#define COLOR_WHITE RGB(255, 255, 255)
#define COLOR_BLACK RGB(0, 0, 0)

// Color macros (ARGB)
#define GFX_COLOR(r,g,b) ((uint32_t)(((r)&0xFF)<<16 | ((g)&0xFF)<<8 | ((b)&0xFF)))
#define COLOR_BLACK GFX_COLOR(0,0,0)
#define COLOR_WHITE GFX_COLOR(255,255,255)
#define COLOR_RED   GFX_COLOR(255,0,0)
#define COLOR_GREEN GFX_COLOR(0,255,0)
#define COLOR_BLUE  GFX_COLOR(0,0,255)