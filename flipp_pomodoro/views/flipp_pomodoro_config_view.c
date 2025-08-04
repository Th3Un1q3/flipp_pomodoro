#include "flipp_pomodoro_config_view.h"
#include <furi.h>
#include <gui/view.h>
#include <gui/elements.h>

struct FlippPomodoroConfigView {
    View* view;
};

static void config_draw_callback(Canvas* canvas, void* ctx) {
    (void)ctx;
    canvas_clear(canvas);
}

FlippPomodoroConfigView* flipp_pomodoro_view_config_alloc() {
    FlippPomodoroConfigView* config = malloc(sizeof(FlippPomodoroConfigView));
    config->view = view_alloc();
    view_set_draw_callback(config->view, config_draw_callback);
    return config;
}

View* flipp_pomodoro_view_config_get_view(FlippPomodoroConfigView* config) {
    return config->view;
}

void flipp_pomodoro_view_config_free(FlippPomodoroConfigView* config) {
    view_free(config->view);
    free(config);
}
