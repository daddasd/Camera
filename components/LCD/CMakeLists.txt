idf_component_register(SRCS "LCD.c"
                       INCLUDE_DIRS "." 
                       INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/managed_components/espressif__esp_lvgl_port/include  # 添加这个路径
                       INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/managed_components/espressif__esp_lcd_touch/include  # 添加这个路径
                       INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/managed_components/espressif__esp_lcd_touch_ft5x06/include  # 添加这个路径
                       INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/managed_components/lvgl__lvgl # 添加这个路径
                       REQUIRES
                       driver
                       esp_lcd
)
