#include "freertos/FreeRTOS.h"

#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/i2c.h"

#include "oled.h"

#define I2C_SCL         (GPIO_NUM_13)
#define I2C_SDA         (GPIO_NUM_14)
#define I2C_PORT        (I2C_NUM_1)
#define I2C_FREQ        (100000)

#define ALIGN_CENTER(width_display, obj_size)  (width_display/2 - obj_size/2)
#define ALIGN_RIGHT(width_display, obj_size)   (width_display - obj_size)
#define ALIGN_LEFT(width_display, obj_size)    (0)

static void init_i2c(void) {
        i2c_config_t conf = {
                .mode = I2C_MODE_MASTER,
                .sda_io_num = I2C_SDA,
                .sda_pullup_en = GPIO_PULLUP_ENABLE,
                .scl_io_num = I2C_SCL,
                .scl_pullup_en = GPIO_PULLUP_ENABLE,
                .master.clk_speed = I2C_FREQ,
                .clk_flags = 0
        };
        i2c_param_config(I2C_PORT, &conf);
        i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
}

void app_main(void) {
        init_i2c();

        const char header_text[] = "OLED SSD1306";
        const char box_text[] = "ESP32";
        const char footer_text[] = "Botelho";

        oled_init(I2C_PORT);
        oled_clear();
        oled_print(header_text, 0, ALIGN_CENTER(DISPLAY_COLS, strlen(header_text)), 1);
        oled_draw_horizontal_line(DISPLAY_WIDTH, ALIGN_CENTER(DISPLAY_WIDTH, DISPLAY_WIDTH), 10, 1);
        oled_draw_rectangle(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, ALIGN_CENTER(DISPLAY_WIDTH, DISPLAY_WIDTH/2), 20, 1, true);
        oled_print(box_text, 4, ALIGN_CENTER(DISPLAY_COLS, strlen(box_text)), 0);
        oled_print(footer_text, 7, ALIGN_RIGHT(DISPLAY_COLS, strlen(footer_text)), 1);
        oled_refresh(I2C_PORT);

        while (true) {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}
