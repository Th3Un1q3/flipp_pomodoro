#include <furi.h>
#include <furi_hal.h>
#include "../helpers/time.h"
#include "flipp_pomodoro.h"
#include "../services/flipp_pomodoro_settings_storage.h" // For settings_load

PomodoroStage stages_sequence[] = {
    FlippPomodoroStageFocus,
    FlippPomodoroStageRest,

    FlippPomodoroStageFocus,
    FlippPomodoroStageRest,

    FlippPomodoroStageFocus,
    FlippPomodoroStageRest,

    FlippPomodoroStageFocus,
    FlippPomodoroStageLongBreak,
};

char *current_stage_label[] = {
    [FlippPomodoroStageFocus] = "Focusing...",
    [FlippPomodoroStageRest] = "Short Break...",
    [FlippPomodoroStageLongBreak] = "Long Break...",
};

char *next_stage_label[] = {
    [FlippPomodoroStageFocus] = "Focus",
    [FlippPomodoroStageRest] = "Short Break",
    [FlippPomodoroStageLongBreak] = "Long Break",
};

PomodoroStage flipp_pomodoro__stage_by_index(int index) {
    const int one_loop_size = sizeof(stages_sequence);
    return stages_sequence[index % one_loop_size];
}

void flipp_pomodoro__toggle_stage(FlippPomodoroState *state)
{
    furi_assert(state);
    state->current_stage_index = state->current_stage_index + 1;
    state->started_at_timestamp = time_now();
};

PomodoroStage flipp_pomodoro__get_stage(FlippPomodoroState *state)
{
    furi_assert(state);
    return flipp_pomodoro__stage_by_index(state->current_stage_index);
};

char *flipp_pomodoro__current_stage_label(FlippPomodoroState *state)
{
    furi_assert(state);
    return current_stage_label[flipp_pomodoro__get_stage(state)];
};

char *flipp_pomodoro__next_stage_label(FlippPomodoroState *state)
{
    furi_assert(state);
    return next_stage_label[flipp_pomodoro__stage_by_index(state->current_stage_index + 1)];
};

void flipp_pomodoro__destroy(FlippPomodoroState *state)
{
    furi_assert(state);
    free(state);
};

uint32_t flipp_pomodoro__current_stage_total_duration(FlippPomodoroState *state)
{
    PomodoroStage current_stage = flipp_pomodoro__get_stage(state);
    uint8_t duration_minutes;

    switch (current_stage) {
        case FlippPomodoroStageFocus:
            duration_minutes = state->settings.work_minutes;
            break;
        case FlippPomodoroStageRest:
            duration_minutes = state->settings.short_break_minutes;
            break;
        case FlippPomodoroStageLongBreak:
            duration_minutes = state->settings.long_break_minutes;
            break;
        default:
            // Should not happen
            duration_minutes = 25;
            break;
    }
    return duration_minutes * TIME_SECONDS_IN_MINUTE;
};

uint32_t flipp_pomodoro__stage_expires_timestamp(FlippPomodoroState *state)
{
    return state->started_at_timestamp + flipp_pomodoro__current_stage_total_duration(state);
};

TimeDifference flipp_pomodoro__stage_remaining_duration(FlippPomodoroState *state)
{
    const uint32_t stage_ends_at = flipp_pomodoro__stage_expires_timestamp(state);
    return time_difference_seconds(time_now(), stage_ends_at);
};

bool flipp_pomodoro__is_stage_expired(FlippPomodoroState *state)
{
    const uint32_t expired_by = flipp_pomodoro__stage_expires_timestamp(state);
    const uint8_t seamless_change_span_seconds = 1;
    return (time_now() - seamless_change_span_seconds) >= expired_by;
};

FlippPomodoroState *flipp_pomodoro__new()
{
    FlippPomodoroState *state = malloc(sizeof(FlippPomodoroState));
    const uint32_t now = time_now();
    state->started_at_timestamp = now;
    state->current_stage_index = 0;

    // Initialize settings with default values
    state->settings.work_minutes = 25;
    state->settings.short_break_minutes = 5;
    state->settings.long_break_minutes = 30;

    // Load persisted settings, potentially overriding the defaults above
    flipp_pomodoro_settings_load(state);

    return state;
};

FlippPomodoroSettings* flipp_pomodoro__get_settings(FlippPomodoroState* state) {
    furi_assert(state);
    return &state->settings;
}

void flipp_pomodoro__set_settings(FlippPomodoroState* state, FlippPomodoroSettings* settings) {
    furi_assert(state);
    furi_assert(settings);
    state->settings.work_minutes = settings->work_minutes;
    state->settings.short_break_minutes = settings->short_break_minutes;
    state->settings.long_break_minutes = settings->long_break_minutes;
}