#include "flipp_pomodoro_config_view.h"
#include <furi.h>
#include <gui/view.h>
#include <gui/canvas.h>

typedef struct {
    uint8_t selected; // 0: focus, 1: short break, 2: long break
    uint8_t durations[3]; // минуты, только для отображения
} FlippPomodoroConfigViewModel;

struct FlippPomodoroConfigView {
    View* view;
};

static void config_draw_callback(Canvas* canvas, void* ctx) {
    FlippPomodoroConfigViewModel* model = ctx;
    canvas_clear(canvas);

    const char* labels[3] = {"Focus:", "Short Break:", "Long Break:"};
    for(uint8_t i = 0; i < 3; i++) {
        bool sel = (i == model->selected);

        // координаты
        uint8_t x_label = 10;
        uint8_t y = 18 + 20 * i;
        uint8_t x_left = 73;
        uint8_t x_val = 79;
        uint8_t x_right = 115;

        // инверсия цвета для выбранной строки
        if(sel) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 6, y - 10, 118, 18);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_set_font(canvas, FontPrimary);
        // Название пункта
        canvas_draw_str(canvas, x_label, y, labels[i]);
        // Левая стрелка
        canvas_draw_str(canvas, x_left, y, "<");
        // Значение
        char val_buf[8];
        snprintf(val_buf, sizeof(val_buf), "%2u min", model->durations[i]);
        canvas_draw_str(canvas, x_val, y, val_buf);
        // Правая стрелка
        canvas_draw_str(canvas, x_right, y, ">");
    }
}

static bool config_input_callback(InputEvent* event, void* ctx) {
    View* view = ctx;
    bool handled = false;
    with_view_model(view, FlippPomodoroConfigViewModel * model, {
        if((event->type == InputTypePress) || (event->type == InputTypeRepeat)) {
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
                default: break;
            }
        }
    }, true);
    return handled;
}

FlippPomodoroConfigView* flipp_pomodoro_view_config_alloc() {
    FlippPomodoroConfigView* config = malloc(sizeof(FlippPomodoroConfigView));
    config->view = view_alloc();
    view_allocate_model(config->view, ViewModelTypeLockFree, sizeof(FlippPomodoroConfigViewModel));
    view_set_context(config->view, config->view);
    view_set_draw_callback(config->view, config_draw_callback);
    view_set_input_callback(config->view, config_input_callback);

    with_view_model(config->view, FlippPomodoroConfigViewModel * model, {
        model->selected = 0;
        model->durations[0] = 25; // Focus
        model->durations[1] = 5;  // Short Break
        model->durations[2] = 30; // Long Break
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
