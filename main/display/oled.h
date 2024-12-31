#pragma once

#include <string.h>
#include <stdbool.h>

#include "driver/i2c.h"

#include "font.h"

#define DISPLAY_WIDTH       128
#define DISPLAY_HEIGHT      64
#define DISPLAY_PAGES       8
#define DISPLAY_COLS        DISPLAY_WIDTH/FONT_WIDTH
#define DISPLAY_ROWS        DISPLAY_HEIGHT/DISPLAY_PAGES
#define DISPLAY_TIMEOUT_MS  500     

esp_err_t oled_init(i2c_port_t i2c_port);

esp_err_t oled_refresh(i2c_port_t i2c_port);

void oled_clear(void);

void oled_print(const char *text, int row, int col, int color);

void oled_draw_pixel(int x, int y, int color);

void oled_draw_horizontal_line(int width, int x, int y, int color);

void oled_draw_rectangle(int width, int height, int x, int y, int color, bool fill);
