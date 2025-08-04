#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t focus_minutes;
    uint8_t short_break_minutes;
    uint8_t long_break_minutes;
} FlippPomodoroSettings;

bool flipp_pomodoro_settings_load(FlippPomodoroSettings* settings);
bool flipp_pomodoro_settings_save(const FlippPomodoroSettings* settings);
void flipp_pomodoro_settings_set_default(FlippPomodoroSettings* settings);
