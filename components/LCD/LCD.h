/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-04-22 18:54:16
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-25 20:37:20
 * @FilePath: \boot_key\components\LCD\LCD.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-04-22 18:54:16
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-04-25 19:20:26
 * @FilePath: \boot_key\components\LCD\LCD.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include <string.h>
#include "math.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_types.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_lvgl_port.h"
#include "MRX.h"
/******************************************************************************/
/***************************  I2C ↓ *******************************************/
#define BSP_I2C_SDA (GPIO_NUM_1) // SDA引脚
#define BSP_I2C_SCL (GPIO_NUM_2) // SCL引脚

#define BSP_I2C_NUM (0)        // I2C外设
#define BSP_I2C_FREQ_HZ 100000 // 100kHz

esp_err_t bsp_i2c_init(void); // 初始化I2C接口
/***************************  I2C ↑  *******************************************/
/*******************************************************************************/
// LCD参数（根据实际情况修改）
#define LCD_WIDTH 320
#define LCD_HEIGHT 240

// FT6336 I2C配置
#define FT6336_I2C_ADDR 0x38
// FT6336寄存器地址
#define FT6336_REG_NUM_TOUCH 0x02
#define FT6336_REG_STATUS 0x02
#define FT6336_REG_XH 0x03
#define FT6336_REG_XL 0x04
#define FT6336_REG_YH 0x05
#define FT6336_REG_YL 0x06
/***********************************************************/
/***************    IO扩展芯片 ↓   *************************/
#define PCA9557_INPUT_PORT 0x00
#define PCA9557_OUTPUT_PORT 0x01
#define PCA9557_POLARITY_INVERSION_PORT 0x02
#define PCA9557_CONFIGURATION_PORT 0x03

#define LCD_CS_GPIO BIT(0)   // PCA9557_GPIO_NUM_1
#define PA_EN_GPIO BIT(1)    // PCA9557_GPIO_NUM_2
#define DVP_PWDN_GPIO BIT(2) // PCA9557_GPIO_NUM_3

#define PCA9557_SENSOR_ADDR 0x19 /*!< Slave address of the MPU9250 sensor */

#define SET_BITS(_m, _s, _v) ((_v) ? (_m) | ((_s)) : (_m) & ~((_s)))

void pca9557_init(void);
void lcd_cs(uint8_t level);
void pa_en(uint8_t level);
void dvp_pwdn(uint8_t level);
/***************    IO扩展芯片 ↑   *************************/
/***********************************************************/

/***********************************************************/
/****************    LCD显示屏 ↓   *************************/

#define COLOR_BLACK 0x0000   // 黑
#define COLOR_WHITE 0xFFFF   // 白
#define COLOR_RED 0xF800     // 红
#define COLOR_GREEN 0x07E0   // 绿
#define COLOR_BLUE 0x001F    // 蓝
#define COLOR_YELLOW 0xFFE0  // 黄
#define COLOR_CYAN 0x07FF    // 青
#define COLOR_MAGENTA 0xF81F // 品红
#define COLOR_GRAY 0x8410    // 灰
#define COLOR_ORANGE 0xFD20  // 橙

#define BSP_LCD_PIXEL_CLOCK_HZ (80 * 1000 * 1000)
#define BSP_LCD_SPI_NUM (SPI3_HOST)
#define LCD_CMD_BITS (8)
#define LCD_PARAM_BITS (8)
#define BSP_LCD_BITS_PER_PIXEL (16)
#define LCD_LEDC_CH LEDC_CHANNEL_0

#define BSP_LCD_H_RES (320)
#define BSP_LCD_V_RES (240)
#define BSP_LCD_RAM (20)

#define BSP_LCD_SPI_MOSI (GPIO_NUM_40)
#define BSP_LCD_SPI_CLK (GPIO_NUM_41)
#define BSP_LCD_SPI_CS (GPIO_NUM_NC)
#define BSP_LCD_DC (GPIO_NUM_39)
#define BSP_LCD_RST (GPIO_NUM_NC)
#define BSP_LCD_BACKLIGHT (GPIO_NUM_42)

esp_err_t bsp_display_brightness_init(void);
esp_err_t bsp_display_brightness_set(int brightness_percent);
esp_err_t bsp_display_backlight_off(void);
esp_err_t bsp_display_backlight_on(void);
esp_err_t bsp_lcd_init(void);
void lcd_set_color(uint16_t color);
void lcd_draw_pictrue(int x_start, int y_start, int x_end, int y_end, const unsigned char *gImage);
void lcd_draw_point(int x, int y, uint16_t color);
void LCD_Touch(void);
void bsp_lvgl_start(void);
/***************    LCD显示屏 ↑   *************************/
/***********************************************************/
