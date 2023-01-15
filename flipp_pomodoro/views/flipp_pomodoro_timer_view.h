#pragma once

#include <gui/view.h>
#include "../modules/flipp_pomodoro.h"

typedef struct FlippPomodoroTimerView FlippPomodoroTimerView;

typedef void (*FlippPomodoroTimerViewRightCb)(void* context);

FlippPomodoroTimerView* flipp_pomodoro_view_timer_alloc(void* ctx);

View* flipp_pomodoro_view_timer_get_view(FlippPomodoroTimerView* timer);

void flipp_pomodoro_view_timer_free(FlippPomodoroTimerView* timer);

void flipp_pomodoro_view_timer_set_state(View* view, FlippPomodoroState* state);
