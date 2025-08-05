#include "flipp_pomodoro_config_view.h"
#include "../modules/flipp_pomodoro_settings.h"
#include <furi.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>

typedef struct {
    uint8_t selected;     // 0: focus, 1: short break, 2: long break, 3: long break
    uint8_t durations[3]; // min at screen only (RAM)
    uint8_t buzz_mode;    // FlippPomodoroBuzzMode (RAM)
} FlippPomodoroConfigViewModel;

struct FlippPomodoroConfigView {
    View* view;
    FlippPomodoroConfigViewActionCb on_save_cb;
    void* cb_ctx;
};

static const char* buzz_mode_to_str(uint8_t m) {
    switch(m) {
        case FlippPomodoroBuzzSlide: return "Slide";
        case FlippPomodoroBuzzOnce: return "Once";
        case FlippPomodoroBuzzAnnoying: return "Naggy";
        default: return "?";
    }
}

static void config_draw_callback(Canvas* canvas, void* ctx) {
    FlippPomodoroConfigViewModel* model = ctx;
    canvas_clear(canvas);

    const char* labels[4] = {"Focus:", "Short Break:", "Long Break:", "Buzz Mode:"};

    const uint8_t y_base = 14;
    const uint8_t row_h = 14;

    for(uint8_t i = 0; i < 4; i++) {
        bool sel = (i == model->selected);

        uint8_t x_label = 6 + 4;
        uint8_t y = y_base + row_h * i;
        uint8_t x_left = 70;
        uint8_t x_val = 78;
        uint8_t x_right = 118;

        if(sel) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 6, y - 9, 118, 16);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, x_label, y, labels[i]);
        canvas_draw_str(canvas, x_left, y, "<");

        if(i < 3) {
            char val_buf[8];
            snprintf(val_buf, sizeof(val_buf), "%2u min", model->durations[i]);
            canvas_draw_str(canvas, x_val, y, val_buf);
        } else {
            canvas_draw_str(canvas, x_val, y, buzz_mode_to_str(model->buzz_mode));
        }

        canvas_draw_str(canvas, x_right, y, ">");
    }

    elements_button_center(canvas, "Save");
    elements_button_right(canvas, "Back");
}

static bool config_input_callback(InputEvent* event, void* ctx) {
    FlippPomodoroConfigView* self = ctx;
    bool handled = false;

    if((event->type == InputTypePress) || (event->type == InputTypeRepeat)) {
        with_view_model(self->view, FlippPomodoroConfigViewModel * model, {
            switch(event->key) {
                case InputKeyUp:
                    if(model->selected > 0) model->selected--;
                    handled = true;
                    break;
                case InputKeyDown:
                    if(model->selected < 3) model->selected++;
                    handled = true;
                    break;
                case InputKeyRight:
                    if(model->selected < 3) {
                        if(model->durations[model->selected] < 99) model->durations[model->selected]++;
                    } else {
                        model->buzz_mode = (model->buzz_mode + 1) % 3;
                    }
                    handled = true;
                    break;
                case InputKeyLeft:
                    if(model->selected < 3) {
                        if(model->durations[model->selected] > 1) model->durations[model->selected]--;
                    } else {
                        model->buzz_mode = (model->buzz_mode + 2) % 3;
                    }
                    handled = true;
                    break;
                case InputKeyOk:
                    if(self->on_save_cb) self->on_save_cb(self->cb_ctx);
                    handled = true;
                    break;
                default:
                    break;
            }
        }, true);
    }

    return handled;
}

FlippPomodoroConfigView* flipp_pomodoro_view_config_alloc() {
    FlippPomodoroConfigView* config = malloc(sizeof(FlippPomodoroConfigView));
    config->view = view_alloc();
    config->on_save_cb = NULL;
    config->cb_ctx = NULL;

    view_allocate_model(config->view, ViewModelTypeLockFree, sizeof(FlippPomodoroConfigViewModel));
    view_set_context(config->view, config);
    view_set_draw_callback(config->view, config_draw_callback);
    view_set_input_callback(config->view, config_input_callback);

    with_view_model(config->view, FlippPomodoroConfigViewModel * model, {
        model->selected = 0;
        model->durations[0] = 0;
        model->durations[1] = 0;
        model->durations[2] = 0;
        model->buzz_mode = FlippPomodoroBuzzOnce;
    }, false);

    return config;
}

View* flipp_pomodoro_view_config_get_view(FlippPomodoroConfigView* config) {
    return config->view;
}

void flipp_pomodoro_view_config_free(FlippPomodoroConfigView* config) {
    view_free(config->view);
    free(config);
}

void flipp_pomodoro_view_config_set_on_save_cb(
    FlippPomodoroConfigView* view,
    FlippPomodoroConfigViewActionCb cb,
    void* ctx
) {
    view->on_save_cb = cb;
    view->cb_ctx = ctx;
}

void flipp_pomodoro_view_config_set_settings(
    FlippPomodoroConfigView* view,
    const FlippPomodoroSettings* in
) {
    furi_assert(in);
    with_view_model(view->view, FlippPomodoroConfigViewModel * model, {
        model->durations[0] = in->focus_minutes;
        model->durations[1] = in->short_break_minutes;
        model->durations[2] = in->long_break_minutes;
        model->buzz_mode = in->buzz_mode;
    }, true);
}

void flipp_pomodoro_view_config_get_settings(
    FlippPomodoroConfigView* view,
    FlippPomodoroSettings* out
) {
    furi_assert(out);
    with_view_model(view->view, FlippPomodoroConfigViewModel * model, {
        out->focus_minutes = model->durations[0];
        out->short_break_minutes = model->durations[1];
        out->long_break_minutes = model->durations[2];
        out->buzz_mode = model->buzz_mode;
    }, false);
}
