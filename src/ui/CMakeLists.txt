SET(SOURCES ui_comp_scrolldots.c
    ui_comp.c
    ui_Central_Screen.c
    ui_Basic_Settings_Screen.c
    ui_No_Connection_Screen.c
    ui_Cards_Screen.c
    ui_SpecificSettingsScreen.c
    ui_Cyrano_Screen.c
    ui_Set_Time_Screen.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    ui_img_icons_icon_next_png.c
    ui_img_icons_icon_prev_png.c
    ui_img_icons_icon_settings_png.c
    ui_img_icons_icon_home_png.c)

add_library(ui ${SOURCES})
