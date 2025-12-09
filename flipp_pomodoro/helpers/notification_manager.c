#include "notification_manager.h"
#include "notifications.h"
#include "../helpers/time.h"
#include <notification/notification.h>
#include <furi.h>

struct NotificationManager {
    bool stage_complete_sent;
    bool notification_started;
    uint8_t notification_repeats_left;
    uint32_t notification_cooldown_until;
    bool flash_backlight_on;
};

NotificationManager* notification_manager_alloc(void) {
    NotificationManager* manager = malloc(sizeof(NotificationManager));
    notification_manager_reset(manager);
    return manager;
}

void notification_manager_free(NotificationManager* manager) {
    furi_assert(manager);
    free(manager);
}

void notification_manager_reset(NotificationManager* manager) {
    furi_assert(manager);
    manager->stage_complete_sent = false;
    manager->notification_started = false;
    manager->notification_repeats_left = 0;
    manager->notification_cooldown_until = 0;
    manager->flash_backlight_on = false;
}

void notification_manager_reset_flags(NotificationManager* manager) {
    furi_assert(manager);
    manager->stage_complete_sent = false;
    manager->notification_started = false;
    manager->notification_repeats_left = 0;
    manager->notification_cooldown_until = 0;
    manager->flash_backlight_on = false;
}

static void send_notification_sequence(const NotificationSequence* seq) {
    if(!seq) {
        return;
    }
    NotificationApp* n = furi_record_open(RECORD_NOTIFICATION);
    notification_message(n, seq);
    furi_record_close(RECORD_NOTIFICATION);
}

static void stop_all_notifications(void) {
    NotificationApp* n = furi_record_open(RECORD_NOTIFICATION);
    static const NotificationSequence seq_stop = {
        &message_sound_off,
        &message_vibro_off,
        &message_green_0,
        &message_red_0,
        &message_display_backlight_on,
        NULL,
    };
    notification_message(n, &seq_stop);
    furi_record_close(RECORD_NOTIFICATION);
}

void notification_manager_stop(NotificationManager* manager) {
    furi_assert(manager);
    manager->notification_repeats_left = 0;
    manager->notification_cooldown_until = time_now() + 1;
    manager->notification_started = false;
    manager->flash_backlight_on = false;
    stop_all_notifications();
}

static const NotificationSequence seq_flash_on = {
    &message_display_backlight_on,
    &message_green_255,
    NULL,
};

static const NotificationSequence seq_flash_off = {
    &message_display_backlight_off,
    &message_green_0,
    NULL,
};

static const NotificationSequence seq_vibrate = {
    &message_vibro_on,
    &message_delay_250,
    &message_vibro_off,
    NULL,
};

static const NotificationSequence seq_beep_soft = {
    &message_display_backlight_on,
    &message_note_d5,
    &message_delay_250,
    &message_sound_off,
    NULL,
};

static const NotificationSequence seq_beep_loud = {
    &message_display_backlight_on,
    &message_note_d5,
    &message_delay_250,
    &message_note_b5,
    &message_delay_250,
    &message_note_d5,
    &message_delay_250,
    &message_sound_off,
    NULL,
};

static void toggle_flash_pattern(NotificationManager* manager) {
    send_notification_sequence(manager->flash_backlight_on ? &seq_flash_off : &seq_flash_on);
    manager->flash_backlight_on = !manager->flash_backlight_on;
}

static void notify_next_stage(NotificationManager* manager, PomodoroStage current_stage, uint8_t stage_index) {
    const uint32_t now = time_now();
    if(now < manager->notification_cooldown_until) {
        return;
    }

    uint8_t pos = stage_index % 8;
    PomodoroStage next;
    if(current_stage == FlippPomodoroStageFocus) {
        next = (pos == 6) ? FlippPomodoroStageLongBreak : FlippPomodoroStageRest;
    } else {
        next = FlippPomodoroStageFocus;
    }
    const NotificationSequence* seq = stage_start_notification_sequence_map[next];

    send_notification_sequence(seq);
    manager->notification_cooldown_until = now + 3;
}

static bool handle_buzz_slide(NotificationManager* manager) {
    if(!manager->stage_complete_sent) {
        manager->stage_complete_sent = true;
        return true; // Signal to send stage complete event
    }
    return false;
}

static bool handle_buzz_once(NotificationManager* manager, PomodoroStage current_stage, uint8_t stage_index) {
    if(!manager->notification_started) {
        manager->notification_started = true;
        notify_next_stage(manager, current_stage, stage_index);
    }
    return false;
}

static bool handle_buzz_annoying(NotificationManager* manager, PomodoroStage current_stage, uint8_t stage_index) {
    if(!manager->notification_started) {
        manager->notification_started = true;
        manager->notification_repeats_left = 10;
    }
    if(manager->notification_repeats_left > 0) {
        notify_next_stage(manager, current_stage, stage_index);
        manager->notification_repeats_left--;
        if(manager->notification_repeats_left == 0) {
            stop_all_notifications();
        }
    }
    return false;
}

static bool handle_buzz_flash(NotificationManager* manager) {
    const uint32_t now = time_now();
    if(now < manager->notification_cooldown_until) {
        return false;
    }
    toggle_flash_pattern(manager);
    manager->notification_started = true;
    manager->notification_cooldown_until = now + 1;
    return false;
}

static bool handle_buzz_vibrate(NotificationManager* manager) {
    const uint32_t now = time_now();
    if(now < manager->notification_cooldown_until) {
        return false;
    }
    send_notification_sequence(&seq_vibrate);
    toggle_flash_pattern(manager);
    manager->notification_started = true;
    manager->notification_cooldown_until = now + 1;
    return false;
}

static bool handle_buzz_soft_beep(NotificationManager* manager) {
    const uint32_t now = time_now();
    if(now < manager->notification_cooldown_until) {
        return false;
    }
    send_notification_sequence(&seq_beep_soft);
    manager->notification_started = true;
    manager->notification_cooldown_until = now + 2;
    return false;
}

static bool handle_buzz_loud_beep(NotificationManager* manager) {
    const uint32_t now = time_now();
    if(now < manager->notification_cooldown_until) {
        return false;
    }
    send_notification_sequence(&seq_beep_loud);
    manager->notification_started = true;
    manager->notification_cooldown_until = now + 3;
    return false;
}

bool notification_manager_handle_expired_stage(
    NotificationManager* manager,
    PomodoroStage current_stage,
    uint8_t stage_index,
    FlippPomodoroBuzzMode buzz_mode) {
    
    furi_assert(manager);
    
    switch(buzz_mode) {
        case FlippPomodoroBuzzSlide:
            return handle_buzz_slide(manager);
        case FlippPomodoroBuzzOnce:
            return handle_buzz_once(manager, current_stage, stage_index);
        case FlippPomodoroBuzzAnnoying:
            return handle_buzz_annoying(manager, current_stage, stage_index);
        case FlippPomodoroBuzzFlash:
            return handle_buzz_flash(manager);
        case FlippPomodoroBuzzVibrate:
            return handle_buzz_vibrate(manager);
        case FlippPomodoroBuzzSoftBeep:
            return handle_buzz_soft_beep(manager);
        case FlippPomodoroBuzzLoudBeep:
            return handle_buzz_loud_beep(manager);
        default:
            return handle_buzz_once(manager, current_stage, stage_index);
    }
}
