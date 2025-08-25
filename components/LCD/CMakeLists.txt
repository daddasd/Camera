# =====================================================
# CMakeLists.txt for LCD component
# =====================================================

# 1. 指定 LCD 目录下的源文件
file(GLOB LCD_SRCS
    "myGUI.c"
    "LCD.c"
)

# 2. 自动收集 custom 和 generated 文件夹下所有 .c 文件
file(GLOB_RECURSE CUSTOM_SRCS "custom/*.c")
file(GLOB_RECURSE GENERATED_SRCS "generated/*.c")

# 3. 合并所有源文件
set(ALL_SRCS ${LCD_SRCS} ${CUSTOM_SRCS} ${GENERATED_SRCS})

# 4. 指定头文件搜索路径
set(INCLUDE_PATHS
    "."                  # 当前目录
    "custom"             # custom 头文件
    "generated"          # generated 头文件
    ${CMAKE_SOURCE_DIR}/managed_components/espressif__esp_lvgl_port/include
    ${CMAKE_SOURCE_DIR}/managed_components/espressif__esp_lcd_touch/include
    ${CMAKE_SOURCE_DIR}/managed_components/espressif__esp_lcd_touch_ft5x06/include
    ${CMAKE_SOURCE_DIR}/managed_components/lvgl__lvgl   # lvgl.h 所在路径
)

# 5. 注册组件
idf_component_register(
    SRCS ${ALL_SRCS}
    INCLUDE_DIRS ${INCLUDE_PATHS}
    REQUIRES driver esp_lcd 
    NetTime
)
