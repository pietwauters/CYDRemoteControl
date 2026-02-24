#pragma once
/**
 * rotated_label.h
 *
 * Workaround for LVGL 8.3 not supporting lv_label rotation on ESP32.
 *
 * lv_canvas extends lv_img, so lv_img_set_angle() works on a canvas even
 * though the same call silently fails on lv_label with SW rendering.
 *
 * Usage:
 *   // +90 degrees (text reads bottom-to-top)
 *   lv_obj_t *obj = rotated_label_create(parent, "Score",
 * &lv_font_montserrat_14, lv_color_white(), 900); lv_obj_set_pos(obj, 10, 80);
 *
 *   // -90 degrees (text reads top-to-bottom)
 *   lv_obj_t *obj2 = rotated_label_create(parent, "Score",
 * &lv_font_montserrat_14, lv_color_white(), -900);
 *
 * The angle argument uses LVGL's unit: 1/10 of a degree.
 * So 900 = 90°, -900 = -90°, 1800 = 180°, etc.
 *
 * IMPORTANT: The returned object is a canvas (which is an image).
 *   - Position it with lv_obj_set_pos() / lv_obj_align() as normal.
 *   - The pivot is centred on the canvas automatically.
 *   - Each call allocates a small heap buffer (text_w × text_h × 2 bytes).
 *     Call lv_obj_del() when you're done with it and the buffer is freed via
 *     the event callback registered inside this helper.
 */

#include <lvgl.h>

/**
 * Measure rendered text dimensions using LVGL's own text engine.
 */
static inline void _rl_measure(const char *text, const lv_font_t *font,
                               lv_coord_t *out_w, lv_coord_t *out_h) {
  lv_point_t size;
  lv_txt_get_size(&size, text, font,
                  /*letter_space=*/0, /*line_space=*/0,
                  /*max_width=*/LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  *out_w = size.x;
  *out_h = size.y;
}

/**
 * Event callback: free the canvas pixel buffer when the object is deleted.
 */
static void _rl_delete_cb(lv_event_t *e) {
  void *buf = lv_event_get_user_data(e);
  if (buf)
    lv_mem_free(buf);
}

/**
 * Create a rotated text object.
 *
 * @param parent    Parent LVGL object.
 * @param text      Null-terminated string to display.
 * @param font      Font to use (e.g. &lv_font_montserrat_14).
 * @param color     Text colour.
 * @param angle_10  Rotation in 1/10° units (900 = 90°, -900 = -90°).
 *
 * @return  The canvas object (ready to position). NULL on allocation failure.
 */
static inline lv_obj_t *rotated_label_create(lv_obj_t *parent, const char *text,
                                             const lv_font_t *font,
                                             lv_color_t color,
                                             int16_t angle_10) {
  /* 1. Measure unrotated text */
  lv_coord_t tw, th;
  _rl_measure(text, font, &tw, &th);
  if (tw <= 0)
    tw = 1;
  if (th <= 0)
    th = 1;

  /* 2. Allocate pixel buffer (RGB565 = 2 bytes/px) on the heap */
  uint32_t buf_size = (uint32_t)tw * (uint32_t)th * sizeof(lv_color_t);
  lv_color_t *buf = (lv_color_t *)lv_mem_alloc(buf_size);
  if (!buf)
    return NULL;

  /* 3. Create canvas and attach the buffer */
  lv_obj_t *canvas = lv_canvas_create(parent);
  lv_canvas_set_buffer(canvas, buf, tw, th, LV_IMG_CF_TRUE_COLOR);

  /* Transparent background */
  lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_TRANSP);

  /* 4. Draw the text onto the canvas */
  lv_draw_label_dsc_t dsc;
  lv_draw_label_dsc_init(&dsc);
  dsc.font = font;
  dsc.color = color;
  dsc.opa = LV_OPA_COVER;
  lv_canvas_draw_text(canvas, 0, 0, tw, &dsc, text);

  /* 5. Rotate around the canvas centre */
  lv_img_set_pivot(canvas, tw / 2, th / 2);
  lv_img_set_angle(canvas, angle_10);

  /* 6. Free the pixel buffer when the canvas is deleted */
  lv_obj_add_event_cb(canvas, _rl_delete_cb, LV_EVENT_DELETE, (void *)buf);

  return canvas;
}
