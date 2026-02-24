/**
 * @file lv_conf.h
 * Tailored for ESP32 CYD (320x240, RGB565) with LVGL 9.5
 * - Switches allocator to system malloc (no 64 KB static DRAM pool)
 * - Keeps only the widgets actually used by the project
 * - Trims SW-draw colour-format support to what the display needs
 */

/* clang-format off */
#if 0   /* Disabled – project uses LVGL 8.3.11 which ships its own config */

#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
   COLOR SETTINGS
 *====================*/
/** 16-bit RGB565 – matches TFT_eSPI / ST7789 */
#define LV_COLOR_DEPTH 16

/*=========================
   STDLIB WRAPPER SETTINGS
 *=========================*/
/**
 * Use the system (Arduino) malloc/free instead of a 64 KB static pool.
 * This is the main fix for the DRAM overflow: it removes the
 * 64 KB .bss allocation that the BUILTIN allocator requires.
 */
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING    LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN  /* keep built-in sprintf */

#define LV_STDINT_INCLUDE       <stdint.h>
#define LV_STDDEF_INCLUDE       <stddef.h>
#define LV_STDBOOL_INCLUDE      <stdbool.h>
#define LV_INTTYPES_INCLUDE     <inttypes.h>
#define LV_LIMITS_INCLUDE       <limits.h>
#define LV_STDARG_INCLUDE       <stdarg.h>

/*====================
   HAL SETTINGS
 *====================*/
#define LV_DEF_REFR_PERIOD  33      /* [ms] ~30 fps */
#define LV_DPI_DEF          130

/*=================
 * OPERATING SYSTEM
 *=================*/
#define LV_USE_OS   LV_OS_NONE

/*========================
 * RENDERING CONFIGURATION
 *========================*/
#define LV_DRAW_BUF_STRIDE_ALIGN    1
#define LV_DRAW_BUF_ALIGN           4
#define LV_DRAW_TRANSFORM_USE_MATRIX 0

/** Reduce the simple-layer chunk from the default 24 KB to 8 KB */
#define LV_DRAW_LAYER_SIMPLE_BUF_SIZE   (8 * 1024)
#define LV_DRAW_LAYER_MAX_MEMORY        0

/** Draw-thread stack – kept small; OS is NONE so threading is not used */
#define LV_DRAW_THREAD_STACK_SIZE   (6 * 1024)

#define LV_USE_DRAW_SW 1
#if LV_USE_DRAW_SW == 1
    /* Keep only the formats this project actually writes/reads */
    #define LV_DRAW_SW_SUPPORT_RGB565           1
    #define LV_DRAW_SW_SUPPORT_RGB565_SWAPPED   0
    #define LV_DRAW_SW_SUPPORT_RGB565A8         0
    #define LV_DRAW_SW_SUPPORT_RGB888           1   /* gradient internals */
    #define LV_DRAW_SW_SUPPORT_XRGB8888         0
    #define LV_DRAW_SW_SUPPORT_ARGB8888         1   /* transparent layers   */
    #define LV_DRAW_SW_SUPPORT_ARGB8888_PREMULTIPLIED 0
    #define LV_DRAW_SW_SUPPORT_L8               0
    #define LV_DRAW_SW_SUPPORT_AL88             0
    #define LV_DRAW_SW_SUPPORT_A8               1   /* font anti-aliasing   */
    #define LV_DRAW_SW_SUPPORT_I1               0

    #define LV_DRAW_SW_I1_LUM_THRESHOLD 127
    #define LV_DRAW_SW_DRAW_UNIT_CNT    1
    #define LV_USE_DRAW_ARM2D_SYNC      0
    #define LV_USE_NATIVE_HELIUM_ASM    0
    #define LV_DRAW_SW_COMPLEX          1

    #if LV_DRAW_SW_COMPLEX == 1
        #define LV_DRAW_SW_SHADOW_CACHE_SIZE 0
        #define LV_DRAW_SW_CIRCLE_CACHE_SIZE 4
    #endif

    #define LV_USE_DRAW_SW_ASM  LV_DRAW_SW_ASM_NONE
    #define LV_USE_DRAW_SW_COMPLEX_GRADIENTS 0
#endif

/* Disable all hardware-accelerated back-ends */
#define LV_USE_NEMA_GFX     0
#define LV_USE_PXP          0
#define LV_USE_G2D          0
#define LV_USE_DRAW_DAVE2D  0
#define LV_USE_DRAW_SDL     0
#define LV_USE_DRAW_VG_LITE 0
#define LV_USE_DRAW_DMA2D   0
#define LV_USE_DRAW_OPENGLES 0
#define LV_USE_PPA          0
#define LV_USE_DRAW_EVE     0
#define LV_USE_DRAW_NANOVG  0

/*=======================
 * FEATURE CONFIGURATION
 *=======================*/
#define LV_USE_LOG 0

#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

#define LV_ASSERT_HANDLER_INCLUDE <stdint.h>
#define LV_ASSERT_HANDLER while(1);

#define LV_USE_REFR_DEBUG       0
#define LV_USE_LAYER_DEBUG      0
#define LV_USE_PARALLEL_DRAW_DEBUG 0

#define LV_ENABLE_GLOBAL_CUSTOM 0
#define LV_CACHE_DEF_SIZE       0
#define LV_IMAGE_HEADER_CACHE_DEF_CNT 0
#define LV_GRADIENT_MAX_STOPS   2
#define LV_COLOR_MIX_ROUND_OFS  0
#define LV_OBJ_STYLE_CACHE      0
#define LV_USE_OBJ_ID           0
#define LV_USE_OBJ_NAME         0
#define LV_OBJ_ID_AUTO_ASSIGN   LV_USE_OBJ_ID
#define LV_USE_OBJ_ID_BUILTIN   1
#define LV_USE_OBJ_PROPERTY     0
#define LV_USE_OBJ_PROPERTY_NAME 1
#define LV_USE_GESTURE_RECOGNITION 0

/*=====================
 *  COMPILER SETTINGS
 *====================*/
#define LV_BIG_ENDIAN_SYSTEM 0
#define LV_ATTRIBUTE_TICK_INC
#define LV_ATTRIBUTE_TIMER_HANDLER
#define LV_ATTRIBUTE_FLUSH_READY
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE 1
#define LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_LARGE_CONST
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY
#define LV_ATTRIBUTE_FAST_MEM
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning
#define LV_ATTRIBUTE_EXTERN_DATA
#define LV_USE_FLOAT    0
#define LV_USE_MATRIX   0
#ifndef LV_USE_PRIVATE_API
    #define LV_USE_PRIVATE_API 0
#endif

/*==================
 *   FONT USAGE
 *===================*/
/* Disable all built-in fonts we don't need */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

#define LV_FONT_MONTSERRAT_28_COMPRESSED    0
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW    0
#define LV_FONT_SOURCE_HAN_SANS_SC_14_CJK   0
#define LV_FONT_SOURCE_HAN_SANS_SC_16_CJK   0
#define LV_FONT_UNSCII_8  0
#define LV_FONT_UNSCII_16 0
#define LV_FONT_CUSTOM_DECLARE
#define LV_FONT_DEFAULT &lv_font_montserrat_14
#define LV_FONT_FMT_TXT_LARGE   0
#define LV_USE_FONT_COMPRESSED  0
#define LV_USE_FONT_PLACEHOLDER 1

/*=================
 *  TEXT SETTINGS
 *=================*/
#define LV_TXT_ENC LV_TXT_ENC_UTF8
#define LV_TXT_BREAK_CHARS          " ,.;:-_)]}"
#define LV_TXT_LINE_BREAK_LONG_LEN  0
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN  3
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3
#define LV_USE_BIDI 0
#define LV_USE_ARABIC_PERSIAN_CHARS 0
#define LV_TXT_COLOR_CMD "#"

/*==================
 * WIDGETS
 *================*/
#define LV_WIDGETS_HAS_DEFAULT_VALUE 1

#define LV_USE_ANIMIMG    0   /* not used */

#define LV_USE_ARC        1   /* needed by spinner */

#define LV_USE_ARCLABEL   0

#define LV_USE_BAR        1   /* needed by slider */

#define LV_USE_BUTTON     1

#define LV_USE_BUTTONMATRIX 1 /* needed by keyboard */

#define LV_USE_CALENDAR   0

#define LV_USE_CANVAS     0

#define LV_USE_CHART      0

#define LV_USE_CHECKBOX   1

#define LV_USE_DROPDOWN   1

#define LV_USE_IMAGE      1

#define LV_USE_IMAGEBUTTON 1

#define LV_USE_KEYBOARD   1

#define LV_USE_LABEL      1
#if LV_USE_LABEL
    #define LV_LABEL_TEXT_SELECTION 1
    #define LV_LABEL_LONG_TXT_HINT  1
    #define LV_LABEL_WAIT_CHAR_COUNT 3
#endif

#define LV_USE_LED        0

#define LV_USE_LINE       0

#define LV_USE_LIST       0

#define LV_USE_LOTTIE     0

#define LV_USE_MENU       0

#define LV_USE_MSGBOX     0

#define LV_USE_ROLLER     0

#define LV_USE_SCALE      0

#define LV_USE_SLIDER     1

#define LV_USE_SPAN       0

#define LV_USE_SPINBOX    0

#define LV_USE_SPINNER    1   /* used on No-Connection screen */

#define LV_USE_SWITCH     1

#define LV_USE_TABLE      0

#define LV_USE_TABVIEW    1

#define LV_USE_TEXTAREA   1
#if LV_USE_TEXTAREA != 0
    #define LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500
#endif

#define LV_USE_TILEVIEW   1

#define LV_USE_WIN        0

#define LV_USE_3DTEXTURE  0

/*==================
 * THEMES
 *==================*/
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    #define LV_THEME_DEFAULT_DARK 0
    #define LV_THEME_DEFAULT_GROW 1
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif

#define LV_USE_THEME_SIMPLE 0
#define LV_USE_THEME_MONO   0

/*==================
 * LAYOUTS
 *==================*/
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*====================
 * 3RD PARTY LIBRARIES
 *====================*/
#define LV_FS_DEFAULT_DRIVER_LETTER '\0'
#define LV_USE_FS_STDIO 0
#define LV_USE_FS_POSIX 0
#define LV_USE_FS_WIN32 0
#define LV_USE_FS_FATFS 0
#define LV_USE_FS_MEMFS     0
#define LV_USE_FS_LITTLEFS  0
#define LV_USE_FS_ARDUINO_ESP_LITTLEFS 0
#define LV_USE_FS_ARDUINO_SD 0
#define LV_USE_FS_UEFI      0
#define LV_USE_FS_FROGFS    0

#define LV_USE_LODEPNG      0
#define LV_USE_LIBPNG       0
#define LV_USE_BMP          0
#define LV_USE_TJPGD        0
#define LV_USE_LIBJPEG_TURBO 0
#define LV_USE_LIBWEBP      0
#define LV_USE_GIF          0
#define LV_USE_GSTREAMER    0
#define LV_BIN_DECODER_RAM_LOAD 0
#define LV_USE_RLE          0
#define LV_USE_QRCODE       0
#define LV_USE_BARCODE      0
#define LV_USE_FREETYPE     0
#define LV_USE_TINY_TTF     0
#define LV_USE_RLOTTIE      0
#define LV_USE_GLTF         0
#define LV_USE_SVG          0
#define LV_USE_VECTOR_ANIM  0

/*==================
 * OTHERS
 *==================*/
#define LV_USE_SNAPSHOT     0
#define LV_USE_SYSMON       0
#define LV_USE_PROFILER     0
#define LV_USE_MONKEY       0
#define LV_USE_GRIDNAV      0
#define LV_USE_FRAGMENT     0
#define LV_USE_IMGFONT      0
#define LV_USE_FREERTOS_TASK_NOTIFY 0
#define LV_USE_OBSERVER     1
#define LV_USE_FONT_MANAGER 0
#define LV_USE_XML          0
#define LV_USE_TRANSLATION  0
#define LV_USE_FILE_EXPLORER 0

#define LV_USE_THORVG_INTERNAL 0
#define LV_USE_THORVG_EXTERNAL 0
#define LV_USE_LZ4          0
#define LV_USE_OPENGLES     0
#define LV_USE_NANOVG       0
#define LV_USE_NANOVG_EXTERNAL 0

/*==================
 * DEMO USAGE
 *==================*/
#define LV_USE_DEMO_WIDGETS         0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0
#define LV_USE_DEMO_BENCHMARK       0
#define LV_USE_DEMO_SCROLL          0
#define LV_USE_DEMO_MUSIC           0
#define LV_USE_DEMO_FLEX_LAYOUT     0
#define LV_USE_DEMO_MULTILANG       0
#define LV_USE_DEMO_TRANSFORM       0
#define LV_USE_DEMO_SCROLL2         0
#define LV_USE_DEMO_VECTOR_GRAPHIC  0

#endif /* LV_CONF_H */
#endif /* End of "Content enable" */
