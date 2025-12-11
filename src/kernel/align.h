#pragma once
#define ALIGN_UP(x, a)   (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x,a) ((x) & ~((a) - 1))
