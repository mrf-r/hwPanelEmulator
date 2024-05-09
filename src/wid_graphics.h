#ifndef __WID_GRAPHICS_H
#define __WID_GRAPHICS_H

#include "widget.h"

void drawString(Widget* v, uint16_t xp, uint16_t yp, const char* str, const uint32_t color);
void drawStringCentered(Widget* v, uint16_t xp, uint16_t yp, const char* str, const uint32_t color);
void drawU16Centered(Widget* v, uint16_t xp, uint16_t yp, uint16_t value, const uint32_t color);
void drawOutline(Widget* v, const uint32_t color);
void drawLedFill(Widget* v, const uint32_t color);
void drawCircle(Widget* v, uint8_t y_bottom_cut, uint32_t color);
void drawLine(Widget* v, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t d, uint32_t color);

#define PI_F 3.14159265358979323846264338327950288f

#endif // __WID_GRAPHICS_H