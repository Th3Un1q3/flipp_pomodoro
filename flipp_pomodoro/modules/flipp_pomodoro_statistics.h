#pragma once
#include <furi_hal.h>

typedef struct
{
    uint8_t focus_stages_completed;
} FlippPomodoroStatistics;

FlippPomodoroStatistics *flipp_pomodoro_statistics__new();

uint8_t flipp_pomodoro_statistics__get_focus_stages_completed(FlippPomodoroStatistics *statistics);

void flipp_pomodoro_statistics__increase_focus_stages_completed(FlippPomodoroStatistics *statistics);


void flipp_pomodoro_statistics__destroy(FlippPomodoroStatistics *state);