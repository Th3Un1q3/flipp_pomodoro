#pragma once

#include <gui/view.h>

typedef enum {
    FlippPomodoroInfoViewModeStats,
    FlippPomodoroInfoViewModeAbout,
} FlippPomodoroInfoViewMode;

typedef struct FlippPomodoroInfoView FlippPomodoroInfoView;

typedef void (*FlippPomodoroInfoViewUserActionCb)(void *ctx);

FlippPomodoroInfoView *flipp_pomodoro_info_view_alloc();

View *flipp_pomodoro_info_view_get_view(FlippPomodoroInfoView *info_view);

void flipp_pomodoro_info_view_free(FlippPomodoroInfoView *info_view);

void flipp_pomodoro_info_view_set_pomodoros_completed(View *info_view, uint8_t pomodoros_completed);

void flipp_pomodoro_info_view_set_resume_timer_cb(FlippPomodoroInfoView *info_view, FlippPomodoroInfoViewUserActionCb user_action_cb, void *user_action_cb_ctx);

void flipp_pomodoro_info_view_set_mode(View *view, FlippPomodoroInfoViewMode desired_mode);
