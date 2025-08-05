#pragma once

#include <gui/view.h>
#include "../modules/flipp_pomodoro_settings.h"

typedef struct FlippPomodoroConfigView FlippPomodoroConfigView;
typedef void (*FlippPomodoroConfigViewActionCb)(void* ctx);

FlippPomodoroConfigView* flipp_pomodoro_view_config_alloc();
View* flipp_pomodoro_view_config_get_view(FlippPomodoroConfigView* view);
void flipp_pomodoro_view_config_free(FlippPomodoroConfigView* view);

// Установить значения в модель (из файла на входе в сцену)
void flipp_pomodoro_view_config_set_settings(
    FlippPomodoroConfigView* view,
    const FlippPomodoroSettings* in);

// Получить текущие значения из модели (для Save)
void flipp_pomodoro_view_config_get_settings(
    FlippPomodoroConfigView* view,
    FlippPomodoroSettings* out);

// Кнопка Save (центральная)
void flipp_pomodoro_view_config_set_on_save_cb(
    FlippPomodoroConfigView* view,
    FlippPomodoroConfigViewActionCb cb,
    void* ctx);
