#pragma once

#include "../modules/flipp_pomodoro.h"
#include "../modules/flipp_pomodoro_settings.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Opaque notification manager state
 */
typedef struct NotificationManager NotificationManager;

/**
 * @brief Create a new notification manager instance
 * @return Pointer to the notification manager
 */
NotificationManager* notification_manager_alloc(void);

/**
 * @brief Free the notification manager
 * @param manager Pointer to the notification manager
 */
void notification_manager_free(NotificationManager* manager);

/**
 * @brief Reset notification state when entering a scene
 * @param manager Pointer to the notification manager
 */
void notification_manager_reset(NotificationManager* manager);

/**
 * @brief Stop all active notifications
 * @param manager Pointer to the notification manager
 */
void notification_manager_stop(NotificationManager* manager);

/**
 * @brief Handle timer tick when stage is expired
 * @param manager Pointer to the notification manager
 * @param current_stage Current pomodoro stage
 * @param stage_index Current stage index for calculating next stage
 * @param buzz_mode Notification mode to use
 * @return true if stage complete event should be sent, false otherwise
 */
bool notification_manager_handle_expired_stage(
    NotificationManager* manager,
    PomodoroStage current_stage,
    uint8_t stage_index,
    FlippPomodoroBuzzMode buzz_mode);

/**
 * @brief Reset notification flags when stage is not expired
 * @param manager Pointer to the notification manager
 */
void notification_manager_reset_flags(NotificationManager* manager);
