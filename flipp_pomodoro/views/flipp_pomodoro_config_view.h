#pragma once

#include <gui/view.h>

typedef struct FlippPomodoroConfigView FlippPomodoroConfigView;

FlippPomodoroConfigView* flipp_pomodoro_view_config_alloc();
View* flipp_pomodoro_view_config_get_view(FlippPomodoroConfigView* view);
void flipp_pomodoro_view_config_free(FlippPomodoroConfigView* view);
