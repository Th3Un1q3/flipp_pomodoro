#include <gui/scene_manager.h>
#include "../flipp_pomodoro_app.h"
#include "flipp_pomodoro_scene.h"
#include "../helpers/time.h"
#include "../modules/flipp_pomodoro_settings.h"
#include <string.h>

void flipp_pomodoro_scene_config_on_enter(void* ctx) {
    FlippPomodoroApp* app = ctx;
    app->paused_at_timestamp = time_now();
    FlippPomodoroSettings s;
    if(!flipp_pomodoro_settings_load(&s)) {
        flipp_pomodoro_settings_set_default(&s);
    }
    app->settings_before = s;
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
    FlippPomodoroApp* app = ctx;

    FlippPomodoroSettings now;
    if(!flipp_pomodoro_settings_load(&now)) {
        flipp_pomodoro_settings_set_default(&now);
    }

    bool changed = memcmp(&now, &app->settings_before, sizeof(FlippPomodoroSettings)) != 0;
    if(changed) {
        flipp_pomodoro__destroy(app->state);
        app->state = flipp_pomodoro__new();
    } else {
        uint32_t paused = time_now() - app->paused_at_timestamp;
        app->state->started_at_timestamp += paused;
    }
}
