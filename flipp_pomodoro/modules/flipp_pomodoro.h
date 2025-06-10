#pragma once

#include <furi_hal.h>
#include "../helpers/time.h"

/// @brief Options of pomodoro stages
typedef enum
{
    FlippPomodoroStageFocus,
    FlippPomodoroStageRest,
    FlippPomodoroStageLongBreak,
} PomodoroStage;

/// @brief Settings for Pomodoro timer durations
typedef struct {
    uint8_t work_minutes;
    uint8_t short_break_minutes;
    uint8_t long_break_minutes;
} FlippPomodoroSettings;

/// @brief State of the pomodoro timer
typedef struct
{
    uint8_t current_stage_index;
    uint32_t started_at_timestamp;
    FlippPomodoroSettings settings;
} FlippPomodoroState;

/// @brief Generates initial state
/// @returns A new pre-populated state for pomodoro timer
FlippPomodoroState *flipp_pomodoro__new();

/// @brief Extract current stage of pomodoro
/// @param state - pointer to the state of pomorodo
/// @returns Current stage value
PomodoroStage flipp_pomodoro__get_stage(FlippPomodoroState *state);

/// @brief Destroys state of timer and it's dependencies
void flipp_pomodoro__destroy(FlippPomodoroState *state);

/// @brief Get remaining stage time.
/// @param state - pointer to the state of pomorodo
/// @returns Time difference to the end of current stage
TimeDifference flipp_pomodoro__stage_remaining_duration(FlippPomodoroState *state);

/// @brief Label of currently active stage
/// @param state - pointer to the state of pomorodo
/// @returns A string that explains current stage
char *flipp_pomodoro__current_stage_label(FlippPomodoroState *state);

/// @brief Label of transition to the next stage
/// @param state - pointer to the state of pomorodo.
/// @returns string with the label of the "skipp" button
char *flipp_pomodoro__next_stage_label(FlippPomodoroState *state);

/// @brief Check if current stage is expired
/// @param state - pointer to the state of pomorodo.
/// @returns expriations status - true means stage is expired
bool flipp_pomodoro__is_stage_expired(FlippPomodoroState *state);

/// @brief Rotate stage of the timer
/// @param state - pointer to the state of pomorodo.
void flipp_pomodoro__toggle_stage(FlippPomodoroState *state);

/// @brief Get current timer settings
/// @param state - pointer to the state of pomorodo.
/// @returns Pointer to the FlippPomodoroSettings struct
FlippPomodoroSettings* flipp_pomodoro__get_settings(FlippPomodoroState* state);

/// @brief Set new timer settings
/// @param state - pointer to the state of pomorodo.
/// @param settings - pointer to the new FlippPomodoroSettings struct
void flipp_pomodoro__set_settings(FlippPomodoroState* state, FlippPomodoroSettings* settings);
