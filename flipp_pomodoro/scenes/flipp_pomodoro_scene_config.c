#include <gui/scene_manager.h>
#include "../flipp_pomodoro_app.h"
#include "flipp_pomodoro_scene.h"

void flipp_pomodoro_scene_config_on_enter(void* ctx) {
    FlippPomodoroApp* app = ctx;
    view_dispatcher_switch_to_view(app->view_dispatcher, FlippPomodoroAppViewConfig);
}


bool flipp_pomodoro_scene_config_on_event(void* ctx, SceneManagerEvent event) {
    FlippPomodoroApp* app = ctx;
    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(app->scene_manager, FlippPomodoroSceneTimer);
        return true;
    }
    return false;
}

void flipp_pomodoro_scene_config_on_exit(void* ctx) {
    (void)ctx;
}
