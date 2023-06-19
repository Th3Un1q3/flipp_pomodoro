
#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view.h>
#include "flipp_pomodoro_info_view.h"

enum
{
    ViewInputConsumed = true,
    ViewInputNotConusmed = false,
};

struct FlippPomodoroInfoView {
    View* view;
    FlippPomodoroInfoViewUserActionCb resume_timer_cb;
    void* user_action_cb_ctx;
    
};

typedef struct
{
    uint8_t pomodoros_completed;
    FlippPomodoroInfoViewMode mode;
} FlippPomodoroInfoViewModel;


static void flipp_pomodoro_info_view_draw_callback(Canvas *canvas, void *_model) {
    if (!_model)
    {
        return;
    };

    FlippPomodoroInfoViewModel *model = _model;
    // TODO: remove once used;
    UNUSED(model);

    canvas_clear(canvas);

    elements_button_right(canvas, "Resume");
}

bool flipp_pomodoro_info_view_input_callback(InputEvent *event, void *ctx) {
    UNUSED(event);
    UNUSED(ctx);
    return ViewInputNotConusmed;
}

FlippPomodoroInfoView *flipp_pomodoro_info_view_alloc() {
    FlippPomodoroInfoView* info_view = malloc(sizeof(FlippPomodoroInfoView));

    view_allocate_model(flipp_pomodoro_info_view_get_view(info_view), ViewModelTypeLockFree, sizeof(FlippPomodoroInfoViewModel));
    view_set_context(flipp_pomodoro_info_view_get_view(info_view), info_view);
    // TODO: complete by implementing these methods
    view_set_draw_callback(flipp_pomodoro_info_view_get_view(info_view), flipp_pomodoro_info_view_draw_callback);
    view_set_input_callback(flipp_pomodoro_info_view_get_view(info_view), flipp_pomodoro_info_view_input_callback);

    return info_view;
}


View *flipp_pomodoro_info_view_get_view(FlippPomodoroInfoView *info_view) {
    return info_view->view;
}

void flipp_pomodoro_info_view_free(FlippPomodoroInfoView *info_view) {
    furi_assert(info_view);
    view_free(info_view->view);
    free(info_view);
}

void flipp_pomodoro_info_view_set_pomodoros_completed(View *view, uint8_t pomodoros_completed) {
     with_view_model(
        view,
        FlippPomodoroInfoViewModel * model,
        {
            model->pomodoros_completed = pomodoros_completed;
        },
        false);
}

void flipp_pomodoro_info_view_set_resume_timer_cb(FlippPomodoroInfoView *info_view, FlippPomodoroInfoViewUserActionCb user_action_cb, void *user_action_cb_ctx) {
    info_view->resume_timer_cb = user_action_cb;
    info_view->user_action_cb_ctx = user_action_cb_ctx;
}

void flipp_pomodoro_info_view_set_mode(View *view, FlippPomodoroInfoViewMode desired_mode) {
    with_view_model(
        view,
        FlippPomodoroInfoViewModel * model,
        {
            model->mode = desired_mode;
        },
        false);
}
