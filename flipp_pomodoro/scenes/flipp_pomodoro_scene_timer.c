#include <furi.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include "../flipp_pomodoro_app.h"
#include "../views/flipp_pomodoro_timer_view.h"

void flipp_pomodoro_scene_timer_on_enter(void* context) {
    furi_assert(context);

    FlippPomodoroApp* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, FlippPomodoroAppViewTimer);

    // Start work
    flipp_pomodoro_view_timer_set_state(flipp_pomodoro_view_timer_get_view(app->timer_view), app->state);

    // TODO: ASSIGN CONTROLS
};

bool flipp_pomodoro_scene_timer_on_event(void* context, SceneManagerEvent event) {
    UNUSED(event);
    UNUSED(context);
    return true;
}

// bool flipp_pomodoro_scene_timer_on_event(void* context, SceneManagerEvent event) {
//     FlippPomodoroApp* app = context;
//     bool consumed = false;
    
//     // TODO: add key processing
//     // TODO: add tick processing
//     switch (event.type)
//     {
//     case SceneManagerEventTypeBack:
//         /* code */
//         view_dispatcher_stop(app->view_dispatcher);
//         consumed = true;
//         break;
    
//     default:
//         break;
//     }
//     return consumed;
// };

void flipp_pomodoro_scene_timer_on_exit(void* context) {
    UNUSED(context);
};

// void flipp_pomodoro_scene_timer_on_exit(void* context) {
//     FlippPomodoroApp* app = context;
//     view_dispatcher_stop(app->view_dispatcher);
// };
