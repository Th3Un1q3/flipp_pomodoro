#include <furi.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include "../flipp_pomodoro_app.h"

const bool EXIT_SIGNAL = 0;

void flipp_pomodoro_scene_timer_on_enter(void* context) {
    furi_assert(context);

    FlippPomodoroApp* app = context;

    // Start work
    view_dispatcher_switch_to_view(app->view_dispatcher, FlippPomodoroAppViewTimer);
    // TODO: ASSIGN CONTROLS
};

bool flipp_pomodoro_scene_timer_on_event(void* context, SceneManagerEvent event) {
    FlippPomodoroApp* app = context;
    bool consumed = false;
    
    // TODO: add key processing
    // TODO: add tick processing
    switch (event.type)
    {
    case SceneManagerEventTypeBack:
        /* code */
        view_dispatcher_stop(app->view_dispatcher);
        consumed = true;
        break;
    
    default:
        break;
    }
    return consumed;
};

bool flipp_pomodoro_scene_timer_on_exit(void* context) {

};