#include "flipp_pomodoro_config_view.h"
#include "../modules/flipp_pomodoro_settings.h"
#include <furi.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>

typedef struct {
    uint8_t selected;     // 0: focus, 1: short break, 2: long break
    uint8_t durations[3]; // минуты, только на экране (RAM)
} FlippPomodoroConfigViewModel;

struct FlippPomodoroConfigView {
    View* view;
    FlippPomodoroConfigViewActionCb on_save_cb;
    void* cb_ctx;
};

static void config_draw_callback(Canvas* canvas, void* ctx) {
    FlippPomodoroConfigViewModel* model = ctx;
    canvas_clear(canvas);

    const char* labels[3] = {"Focus:", "Short Break:", "Long Break:"};
    for(uint8_t i = 0; i < 3; i++) {
        bool sel = (i == model->selected);

        uint8_t x_label = 10;
        uint8_t y = 18 + 20 * i;
        uint8_t x_left = 73;
        uint8_t x_val = 79;
        uint8_t x_right = 115;

        if(sel) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 6, y - 10, 118, 18);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, x_label, y, labels[i]);
        canvas_draw_str(canvas, x_left, y, "<");

        char val_buf[8];
        snprintf(val_buf, sizeof(val_buf), "%2u min", model->durations[i]);
        canvas_draw_str(canvas, x_val, y, val_buf);

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
                    if(model->selected < 2) model->selected++;
                    handled = true;
                    break;
                case InputKeyRight:
                    if(model->durations[model->selected] < 99) model->durations[model->selected]++;
                    handled = true;
                    break;
                case InputKeyLeft:
                    if(model->durations[model->selected] > 1) model->durations[model->selected]--;
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
    }, false);
}
