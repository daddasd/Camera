#include "myGUI.h"
#include "LCD.h"
#include "esp_lvgl_port.h"
#include "myGUI.h"

/*
 * @Author: 'daddasd' '3323169544@qq.com'
 * @Date: 2025-04-29 09:19:45
 * @LastEditors: 'daddasd' '3323169544@qq.com'
 * @LastEditTime: 2025-05-22 17:50:20
 * @FilePath: \lv_port_win_codeblocks-release-v8.3\myGUI.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "myGUI.h"
#include "lvgl.h"

lv_obj_t *superclass;
lv_obj_t *subclass;
static void my_event_cb(lv_event_t *e)
{
    static int py = 0;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (target == superclass)
    {
        lv_obj_align(superclass, LV_ALIGN_TOP_MID, py, 0); // x=20, y=0
        py = 20 + py;
    }
    else if (target == subclass)
    {
        py = 0;
        lv_obj_align(superclass, LV_ALIGN_TOP_MID, 0, 0);
    }
}

void my_gui(void)
{
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_hex(0xa4d1f9));
    lv_style_set_radius(&style, 20); // 添加圆角优化视觉效果
    superclass = lv_obj_create(lv_scr_act());
    lv_obj_add_style(superclass, &style, LV_STATE_DEFAULT);
    lv_obj_set_size(superclass, 320 * 2 / 3, 240 * 2 / 3);
    lv_obj_set_align(superclass, LV_ALIGN_TOP_MID);

    static lv_style_t style1;
    lv_style_init(&style1);
    lv_style_set_bg_color(&style1, lv_color_hex(0x66a000));
    lv_style_set_radius(&style1, 20); // 添加圆角优化视觉效果
    subclass = lv_obj_create(superclass);
    lv_obj_add_style(subclass, &style1, LV_STATE_DEFAULT);
    lv_obj_set_size(subclass, 320 * 2 / 5, 240 * 2 / 5);
    lv_obj_set_align(subclass, LV_ALIGN_LEFT_MID);

    lv_obj_add_event_cb(superclass, my_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(subclass, my_event_cb, LV_EVENT_LONG_PRESSED, NULL);
}
