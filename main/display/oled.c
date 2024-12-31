#include "oled.h"

#include "freertos/FreeRTOS.h"

#include "driver/gpio.h"

#define SSD1306_ADDR        0x3C

// Control bytes
#define COMMAND_MODE 0x00
#define SINGLE_COMMAND_MODE 0x80
#define DATA_MODE 0x40

// Fundamental Command
#define DISPLAY_LINE_START 0x40
#define CONTRAST_CONTROL 0x81
#define DISABLE_ENTIRE_DISPLAY 0xA4
#define DISPLAY_ON_REG02 0xA5
#define NORMAL_DISPLAY 0xA6
#define DISPLAY_INVERSE  0xA7
#define DISPLAY_RESET 0xAE
#define DISPLAY_ON 0xAF

// Addressing Setting
#define MEM_ADDR_MODE 0x20
#define LOWER_COL_START_ADDR 0x00
#define HIGHER_COL_START_ADDR 0x10
#define PAGE_START_ADDR 0xB0

// Scrolling
#define DEACTIVATE_SCROLL 0x2E
#define ACTIVATE_SCROLL 0x2F
#define VERTICAL_AND_RIGHT_HOR_SCROLL 0x29
#define DUMMY_BYTE 0x00
#define SIX_FRAMES_PER_SEC 0x00
#define VERTICAL_OFFSET_ONE 0x01

// Hardware Config
#define DISPLAY_START_LINE 0x40
#define SEGMENT_REMAP_NORMAL 0xA0
#define SEGMENT_REMAP_INVERSE 0xA1
#define MULTIPLEX_RATIO 0xA8
#define COM_OUTPUT_SCAN_DIR_NORMAL 0xC0
#define COM_OUTPUT_SCAN_DIR_REMAP 0xC8
#define DISPLAY_OFFSET 0xD3
#define COM_PINS_HARDWARE_CONFIG 0xDA

// Timing & Driving Scheme
#define DISPLAY_CLK_RATIO 0xD5
#define PRE_CHARGE_PER 0xD9
#define VCOMH_DESELECT_LEVEL 0xDB
#define NOOPERATION 0xE3

// Charge Pump
#define CHARGE_PUMP_SET 0x8D

uint8_t display_content[DISPLAY_PAGES][DISPLAY_WIDTH];

esp_err_t oled_init(i2c_port_t i2c_port) {
        oled_clear();

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        i2c_master_write_byte(cmd, (SSD1306_ADDR << 1) | I2C_MASTER_WRITE, true);

        // Initialization (page 64)
        // The next bytes are commands
        i2c_master_write_byte(cmd, COMMAND_MODE, true);

        // Mux Ratio
        i2c_master_write_byte(cmd, MULTIPLEX_RATIO, true);
        i2c_master_write_byte(cmd, 0x3F, true);

        // Set display offset
        i2c_master_write_byte(cmd, DISPLAY_OFFSET, true);
        i2c_master_write_byte(cmd, 0x00, true);

        // Set display line start
        i2c_master_write_byte(cmd, DISPLAY_LINE_START, true);
        i2c_master_write_byte(cmd, 0x00, true);

        // Set Segment re-map
        i2c_master_write_byte(cmd, SEGMENT_REMAP_INVERSE, true);

        // Set COM output scan dir
        i2c_master_write_byte(cmd, COM_OUTPUT_SCAN_DIR_REMAP, true);

        // Set COM pins hardware config
        i2c_master_write_byte(cmd, COM_PINS_HARDWARE_CONFIG, true);
        i2c_master_write_byte(cmd, 0x12, true);

        // Set contrast Control
        i2c_master_write_byte(cmd, CONTRAST_CONTROL, true);
        i2c_master_write_byte(cmd, 0x7F, true);

        // Disable entire display
        i2c_master_write_byte(cmd, DISABLE_ENTIRE_DISPLAY, true);

        // Set normal display
        i2c_master_write_byte(cmd, NORMAL_DISPLAY, true);

        // Set OSC frequency
        i2c_master_write_byte(cmd, DISPLAY_CLK_RATIO, true);
        i2c_master_write_byte(cmd, 0x80, true);

        // Enable charge pump regulator
        i2c_master_write_byte(cmd, CHARGE_PUMP_SET, true);
        i2c_master_write_byte(cmd, 0x14, true);

        // Display on
        i2c_master_write_byte(cmd, DISPLAY_ON, true);

        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(DISPLAY_TIMEOUT_MS));
        i2c_cmd_link_delete(cmd);

        return ret;
}

esp_err_t oled_refresh(i2c_port_t i2c_port) {
        i2c_cmd_handle_t cmd = NULL;
        esp_err_t ret = ESP_OK;

        for(uint8_t page = 0; page < DISPLAY_PAGES; page++) {
                cmd = i2c_cmd_link_create();

        	i2c_master_start(cmd);

        	i2c_master_write_byte(cmd, (SSD1306_ADDR << 1) | I2C_MASTER_WRITE, true);

                i2c_master_write_byte(cmd, SINGLE_COMMAND_MODE, true);
        	i2c_master_write_byte(cmd, (PAGE_START_ADDR | page), true);

        	i2c_master_write_byte(cmd, DATA_MODE, true);
                for(uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
        	       i2c_master_write_byte(cmd, display_content[page][col], true);
                }
        	i2c_master_stop(cmd);

                ret = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(DISPLAY_TIMEOUT_MS));

                i2c_cmd_link_delete(cmd);

                if(ret != ESP_OK) {
                        return ret;
                }
        }

        return ret;
}

void oled_clear(void) {
        memset(display_content, 0, (DISPLAY_WIDTH * DISPLAY_PAGES));
}

void oled_print(const char *text, int row, int col, int color) {
        col *= FONT_WIDTH;

        for(int i = 0; text[i] != '\0'; i++) {
                uint16_t char_index = (text[i] <= 0) ? 0 : text[i];
                for(uint8_t j = 0; j < FONT_WIDTH; j++){
                        if(color) {
                                display_content[row][col] = FONTS[char_index][j];
                        }
                        else {
                                display_content[row][col] = ~FONTS[char_index][j];
                        }
                        col++;
                }
        }
}

void oled_draw_pixel(int x, int y, int color) {
        if(color == 1) {
                display_content[y/8][x] |= 1 << (y % 8);
        }
        else {
                display_content[y/8][x] &= ~(1 << (y % 8));
        }
}

void oled_draw_horizontal_line(int width, int x, int y, int color) {
        for(int i = x; i < x + width; i++) {
                oled_draw_pixel(i, y, color);
        }
}

void oled_draw_rectangle(int width, int height, int x, int y, int color, bool fill) {
        for(int j = y; j < y + height; j++) {
                for(int i = x; i < x + width; i++) {
                        if((fill == 1) || (j == y || j == y + height - 1 || i == x || i == x + width - 1)) {
                                oled_draw_pixel(i, j, color); 
                        }
                }
        }
}
