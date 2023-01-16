#include <furi.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include "../flipp_pomodoro_app.h"
#include "../views/flipp_pomodoro_timer_view.h"

void flipp_pomodoro_scene_timer_on_next_stage(void* ctx) {
    furi_assert(ctx);

    FlippPomodoroApp* app = ctx;

    view_dispatcher_send_custom_event(
        app->view_dispatcher,
        FlippPomodoroAppCustomEventTogglePomodoroManual
    );
};

void flipp_pomodoro_scene_timer_on_enter(void* context) {
    furi_assert(context);

    FlippPomodoroApp* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, FlippPomodoroAppViewTimer);
    flipp_pomodoro_view_timer_set_state(
        flipp_pomodoro_view_timer_get_view(app->timer_view),
        app->state
    );
    flipp_pomodoro_view_timer_set_on_right_cb(
        app->timer_view,
        flipp_pomodoro_scene_timer_on_next_stage,
        app
    );
};

bool flipp_pomodoro_scene_timer_on_event(void* ctx, SceneManagerEvent event) {
    FlippPomodoroApp* app = ctx;

    const bool is_timer_tick = (event.type == SceneManagerEventTypeCustom) &&
        (event.event==FlippPomodoroAppCustomEventTimerTick);

    if(is_timer_tick && flipp_pomodoro__is_stage_expired(app->state)) {
        view_dispatcher_send_custom_event(
            app->view_dispatcher,
            FlippPomodoroAppCustomEventTogglePomodoroNatural
        );
    };
    return false;
}

void flipp_pomodoro_scene_timer_on_exit(void* context) {
    UNUSED(context);
};

// void flipp_pomodoro_scene_timer_on_exit(void* context) {
//     FlippPomodoroApp* app = context;
//     view_dispatcher_stop(app->view_dispatcher);
// };
