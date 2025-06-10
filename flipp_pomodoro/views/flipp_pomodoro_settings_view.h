#pragma once

#include <gui/view.h>

// Forward declaration of the view structure
typedef struct FlippPomodoroSettingsView FlippPomodoroSettingsView;

// Callback types for input handling (to be defined later)
typedef void (*FlippPomodoroSettingsViewInputCallback)(void* context);

// Allocate and initialize the settings view
FlippPomodoroSettingsView* flipp_pomodoro_settings_view_alloc();

// Free the settings view
void flipp_pomodoro_settings_view_free(FlippPomodoroSettingsView* view);

// Get the underlying view object
View* flipp_pomodoro_settings_view_get_view(FlippPomodoroSettingsView* view);

// Set placeholder values (actual data will be passed later)
void flipp_pomodoro_settings_view_set_work_time(FlippPomodoroSettingsView* view, uint8_t work_time);
void flipp_pomodoro_settings_view_set_short_break_time(FlippPomodoroSettingsView* view, uint8_t short_break_time);
void flipp_pomodoro_settings_view_set_long_break_time(FlippPomodoroSettingsView* view, uint8_t long_break_time);

// Set input callbacks (to be defined later)
void flipp_pomodoro_settings_view_set_input_callback(
    FlippPomodoroSettingsView* view,
    FlippPomodoroSettingsViewInputCallback callback,
    void* context);

// Getter functions for current values from the view's model
uint8_t flipp_pomodoro_settings_view_get_work_time(FlippPomodoroSettingsView* view);
uint8_t flipp_pomodoro_settings_view_get_short_break_time(FlippPomodoroSettingsView* view);
uint8_t flipp_pomodoro_settings_view_get_long_break_time(FlippPomodoroSettingsView* view);
