#include "flipp_pomodoro_scene_settings.h"
#include "../flipp_pomodoro_app.h" // Provides FlippPomodoroApp and scene definitions
#include "../modules/flipp_pomodoro.h" // Provides flipp_pomodoro__get_settings, etc.
#include "../views/flipp_pomodoro_settings_view.h"
#include "../services/flipp_pomodoro_settings_storage.h" // For saving settings
#include <gui/scene_manager.h> // For scene navigation
#include <gui/view_dispatcher.h> // For view dispatcher
#include <furi.h> // For FURI_LOG_I, UNUSED, FURI_LOG_E

// No need for forward declarations, they are in the header

void flipp_pomodoro_scene_settings_on_enter(void* context) {
    furi_assert(context);
    FlippPomodoroApp* app = context;
    FlippPomodoroSettings* settings = flipp_pomodoro__get_settings(app->state);

    // Assuming flipp_pomodoro_settings_view_alloc has been called and app->settings_view is valid
    // This view should be created and added to view_dispatcher in flipp_pomodoro_app.c
    // For now, we are focusing on scene logic.
    if(app->settings_view) {
        flipp_pomodoro_settings_view_set_work_time(app->settings_view, settings->work_minutes);
        flipp_pomodoro_settings_view_set_short_break_time(app->settings_view, settings->short_break_minutes);
        flipp_pomodoro_settings_view_set_long_break_time(app->settings_view, settings->long_break_minutes);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlippPomodoroAppViewSettings);
    } else {
        FURI_LOG_E("FlippPomodoro", "Settings view is null in scene_settings_on_enter");
    }
    FURI_LOG_I("FlippPomodoro", "SettingsScene::on_enter completed");
}

bool flipp_pomodoro_scene_settings_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    FlippPomodoroApp* app = context;
    bool consumed = false;

    if (event.type == SceneManagerEventTypeKey && event.input.type == InputTypeShort && event.input.key == InputKeyBack) {
        FlippPomodoroSettings new_settings;
        if(app->settings_view) {
            new_settings.work_minutes = flipp_pomodoro_settings_view_get_work_time(app->settings_view);
            new_settings.short_break_minutes = flipp_pomodoro_settings_view_get_short_break_time(app->settings_view);
            new_settings.long_break_minutes = flipp_pomodoro_settings_view_get_long_break_time(app->settings_view);
            flipp_pomodoro__set_settings(app->state, &new_settings);
            FURI_LOG_I("FlippPomodoro", "Settings applied to app state from view.");

            if(flipp_pomodoro_settings_save(app->state)) {
                FURI_LOG_I("FlippPomodoro", "Settings persisted to storage.");
            } else {
                FURI_LOG_E("FlippPomodoro", "Failed to persist settings to storage.");
                // Optionally, provide user feedback about save failure here
            }
        } else {
            FURI_LOG_E("FlippPomodoro", "Settings view is null, cannot update or save settings.");
        }

        // Navigate back even if saving failed, as settings are in memory.
        // The app will use in-memory settings; they just might not persist if saving failed.
        scene_manager_previous_scene(app->scene_manager); // Go back to Timer scene
        consumed = true;
    }
    // Add other event handling for settings modification later (e.g., OK to save, Up/Down to change value)

    return consumed;
}

void flipp_pomodoro_scene_settings_on_exit(void* context) {
    UNUSED(context);
    FURI_LOG_I("FlippPomodoro", "SettingsScene::on_exit");
}
