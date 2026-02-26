#include "msgbox.h"

static void msgbox_event_cb(lv_event_t * e)
{
    lv_obj_t * mbox = lv_event_get_target(e);

    lv_obj_del(mbox);
}

void ui_msgbox_show(
    const char * title,
    const char * text,
    const char * btn1,
    const char * btn2)
{
    const char * btns[3] = { btn1, btn2, "" };

    lv_obj_t * mbox = lv_msgbox_create(
        lv_scr_act(),   // Active screen (v8)
        title,
        text,
        btns,
        true            // Add close button
    );

    lv_obj_center(mbox);

    lv_obj_add_event_cb(
        mbox,
        msgbox_event_cb,
        LV_EVENT_VALUE_CHANGED,
        NULL
    );

    lv_obj_move_foreground(mbox);
}

void ui_toast(const char * text)
{
    ui_msgbox_show("Info", text, "OK", NULL);
}