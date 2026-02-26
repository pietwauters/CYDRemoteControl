#ifndef MSGBOX_H
#define MSGBOX_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_msgbox_show(
    const char * title,
    const char * text,
    const char * btn1,
    const char * btn2
);

void ui_toast(const char * text);

#ifdef __cplusplus
}
#endif

#endif