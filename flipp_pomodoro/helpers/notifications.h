#pragma once

#include "../modules/flipp_pomodoro.h"
#include <notification/notification_messages.h>

extern const NotificationSequence work_start_notification;
extern const NotificationSequence rest_start_notification;

/// @brief Defines a notification sequence that should indicate start of specific pomodoro stage.
const NotificationSequence *stage_start_notification_sequence_map[] = {
    [FlippPomodoroStageFocus] = &work_start_notification,
    [FlippPomodoroStageRest] = &rest_start_notification,
    [FlippPomodoroStageLongBreak] = &rest_start_notification,
};
