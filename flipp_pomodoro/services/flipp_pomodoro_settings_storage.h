#pragma once

// Required for APP_DATA_PATH macro
#include <flipper_application/flipper_application.h>

// Definition of FlippPomodoroSettings is in modules/flipp_pomodoro.h
// It's assumed that modules/flipp_pomodoro.h will be included alongside this file
// when settings storage functions are used.

#define FLIPP_POMODORO_SETTINGS_FILE_NAME "flipp_pomodoro.settings"
#define FLIPP_POMODORO_SETTINGS_FILE_PATH APP_DATA_PATH(FLIPP_POMODORO_SETTINGS_FILE_NAME)

// Necessary headers for file operations (for the .c file that will implement storage logic):
// #include <storage/storage.h>
// #include <furi.h> // For FURI_LOG_ macros if used in storage functions

// Forward declaration for FlippPomodoroState, its definition is in modules/flipp_pomodoro.h
typedef struct FlippPomodoroState FlippPomodoroState;

/// @brief Loads FlippPomodoro settings from persistent storage.
/// If loading fails or data is invalid, default settings are applied to the state.
/// @param state Pointer to the FlippPomodoroState to load settings into.
void flipp_pomodoro_settings_load(FlippPomodoroState* state);

/// @brief Saves the current settings from FlippPomodoroState to persistent storage.
/// @param state Pointer to the FlippPomodoroState containing the settings to save.
/// @return True if settings were saved successfully, false otherwise.
bool flipp_pomodoro_settings_save(FlippPomodoroState* state);
