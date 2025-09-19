#include <furi.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include "flipp_pomodoro_scene.h"
#include "../flipp_pomodoro_app.h"
#include "../views/flipp_pomodoro_timer_view.h"
#include "../modules/flipp_pomodoro_settings.h"
#include "../modules/flipp_pomodoro.h"
#include "../helpers/notifications.h"
#include "../helpers/time.h"
#include "../helpers/hints.h"
#include <notification/notification.h>

enum
{
    SceneEventConusmed = true,
    SceneEventNotConusmed = false
};

static char *random_string_of_list(char **hints, size_t num_hints)
{
    int random_index = rand() % num_hints;
    return hints[random_index];
}

// anti duplicates
static bool g_stage_complete_sent = false; // StageComplete sent only once
static bool g_once_notified = false;       // notif Once/Naggy corrects
static uint8_t g_naggy_left = 0;           // counter for Naggy

// Do not duplicate notifications while the previous one is playing.
// Rough estimate of the duration of the standard sequence ~2.5–3s -> we take 3s.
static uint32_t g_naggy_cooldown_until = 0; // timestamp, before which we do not send the next one

// In Slide, notification at the start of the NEXT stage - we simulate it at the stop in Once/Naggy.
// REUSE the standard sequences stage_start_notification_sequence_map[*].
static void notify_like_slide_next_stage(FlippPomodoroApp* app) {
    const uint32_t now = time_now();
    if(now < g_naggy_cooldown_until) {
        return; // don't put a new one in the queue while the previous one is still playing
    }

    PomodoroStage cur = flipp_pomodoro__get_stage(app->state);
    uint8_t pos = app->state->current_stage_index % 8;
    PomodoroStage next;
    if(cur == FlippPomodoroStageFocus) {
        next = (pos == 6) ? FlippPomodoroStageLongBreak : FlippPomodoroStageRest;
    } else {
        next = FlippPomodoroStageFocus;
    }
    const NotificationSequence* seq = stage_start_notification_sequence_map[next];

    NotificationApp* n = furi_record_open(RECORD_NOTIFICATION);
    notification_message(n, seq);
    furi_record_close(RECORD_NOTIFICATION);

    // block repetitions for the duration of the standard sequence playback
    g_naggy_cooldown_until = now + 3;
}

// Instantly mute sound/vibration (without inventing new patterns - only off-messages).
static void stop_all_notifications(void) {
    NotificationApp* n = furi_record_open(RECORD_NOTIFICATION);
    static const NotificationSequence seq_stop = { &message_sound_off, &message_vibro_off, NULL };
    notification_message(n, &seq_stop);
    furi_record_close(RECORD_NOTIFICATION);
}

// Complete stop of Naggy "spam" + jamm
static void naggy_stop(void) {
    g_naggy_left = 0;
    // a small "umbrella" so that the tick that comes immediately after the stop does not have time to put a new one
    g_naggy_cooldown_until = time_now() + 3;
    stop_all_notifications();
}

void flipp_pomodoro_scene_timer_sync_view_state(void *ctx)
{
    furi_assert(ctx);

    FlippPomodoroApp *app = ctx;

    flipp_pomodoro_view_timer_set_state(
        flipp_pomodoro_view_timer_get_view(app->timer_view),
        app->state);
};

void flipp_pomodoro_scene_timer_on_next_stage(void* ctx) {
    furi_assert(ctx);
    FlippPomodoroApp* app = ctx;
    naggy_stop(); // stop "spam" when moving to the next timer
    const bool expired = flipp_pomodoro__is_stage_expired(app->state);
    view_dispatcher_send_custom_event(
        app->view_dispatcher,
        expired ? FlippPomodoroAppCustomEventStageComplete
                : FlippPomodoroAppCustomEventStageSkip);
}

void flipp_pomodoro_scene_timer_on_left(void* ctx) {
    FlippPomodoroApp* app = ctx;
    naggy_stop(); // stop "spam" when going to settings
    scene_manager_next_scene(app->scene_manager, FlippPomodoroSceneConfig);
}

void flipp_pomodoro_scene_timer_on_ask_hint(void *ctx)
{
    FlippPomodoroApp *app = ctx;
    naggy_stop(); // stop spam on center click
    view_dispatcher_send_custom_event(
        app->view_dispatcher,
        FlippPomodoroAppCustomEventTimerAskHint);
}

void flipp_pomodoro_scene_timer_on_enter(void *ctx)
{
    furi_assert(ctx);

    FlippPomodoroApp *app = ctx;
    g_stage_complete_sent = false;
    g_once_notified = false;
    g_naggy_left = 0;
    g_naggy_cooldown_until = 0;

    // If the stage has already expired:
    // - Slide: start over (as before);
    // - Once/Naggy: stay at 00:00 and DO NOT re-notify after returning.
    if (flipp_pomodoro__is_stage_expired(app->state))
    {
        FlippPomodoroSettings s;
        flipp_pomodoro_settings_load(&s);
        if(s.buzz_mode == FlippPomodoroBuzzSlide) {
            flipp_pomodoro__destroy(app->state);
            app->state = flipp_pomodoro__new();
        } else {
            g_once_notified = true; // prohibition of repeated start of notifications on the first tick
            naggy_stop();
        }
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlippPomodoroAppViewTimer);
    flipp_pomodoro_scene_timer_sync_view_state(app);

    flipp_pomodoro_view_timer_set_callback_context(app->timer_view, app);

    // hint here
    flipp_pomodoro_view_timer_set_on_ok_cb(
        app->timer_view,
        flipp_pomodoro_scene_timer_on_ask_hint);

    flipp_pomodoro_view_timer_set_on_right_cb(
         app->timer_view,
         flipp_pomodoro_scene_timer_on_next_stage);

     flipp_pomodoro_view_timer_set_on_left_cb(
          app->timer_view,
          flipp_pomodoro_scene_timer_on_left);
};

char *flipp_pomodoro_scene_timer_get_contextual_hint(FlippPomodoroApp *app)
{
    switch (flipp_pomodoro__get_stage(app->state))
    {
    case FlippPomodoroStageFocus:
        return random_string_of_list(work_hints, sizeof(work_hints) / sizeof(work_hints[0]));
    case FlippPomodoroStageRest:
    case FlippPomodoroStageLongBreak:
        return random_string_of_list(break_hints, BREAK_HINTS_COUNT);
    default:
        return "What's up?";
    }
}

static void handle_buzz_slide(FlippPomodoroApp* app) {
    if(!g_stage_complete_sent) {
        g_stage_complete_sent = true;
        view_dispatcher_send_custom_event(
            app->view_dispatcher,
            FlippPomodoroAppCustomEventStageComplete);
    }
}

static void handle_buzz_once(FlippPomodoroApp* app) {
    if(!g_once_notified) {
        notify_like_slide_next_stage(app);
        g_once_notified = true;
    }
}

static void handle_buzz_annoying(FlippPomodoroApp* app) {
    if(!g_once_notified) {
        g_once_notified = true;
        g_naggy_left = 10;
    }
    if(g_naggy_left > 0) {
        notify_like_slide_next_stage(app);
        g_naggy_left--;
        if(g_naggy_left == 0) {
            stop_all_notifications();
        }
    }
}

static void reset_notification_flags(void) {
    g_stage_complete_sent = false;
    g_once_notified = false;
    g_naggy_left = 0;
}

void flipp_pomodoro_scene_timer_handle_custom_event(FlippPomodoroApp *app, FlippPomodoroAppCustomEvent custom_event)
{
    switch (custom_event)
    {
    case FlippPomodoroAppCustomEventTimerTick: {
        if (!flipp_pomodoro__is_stage_expired(app->state)) {
            reset_notification_flags();
            break;
        }
        FlippPomodoroSettings s;
        flipp_pomodoro_settings_load(&s);
        switch (s.buzz_mode) {
            case FlippPomodoroBuzzSlide:
                handle_buzz_slide(app);
                break;
            case FlippPomodoroBuzzOnce:
                handle_buzz_once(app);
                break;
            case FlippPomodoroBuzzAnnoying:
                handle_buzz_annoying(app);
                break;
        }
        break;
    }
    case FlippPomodoroAppCustomEventStateUpdated:
        // after changing the stage - the center remains a hint, and stop "spam"
        flipp_pomodoro_scene_timer_sync_view_state(app);
        flipp_pomodoro_view_timer_set_on_ok_cb(
            app->timer_view,
            flipp_pomodoro_scene_timer_on_ask_hint);
        g_stage_complete_sent = false;
        g_once_notified = false;
        naggy_stop();
        break;
    case FlippPomodoroAppCustomEventTimerAskHint:
        flipp_pomodoro_view_timer_display_hint(
            flipp_pomodoro_view_timer_get_view(app->timer_view),
            flipp_pomodoro_scene_timer_get_contextual_hint(app));
        break;
    default:
        break;
    }
};

bool flipp_pomodoro_scene_timer_on_event(void *ctx, SceneManagerEvent event)
{
    furi_assert(ctx);
    FlippPomodoroApp *app = ctx;

    switch (event.type)
    {
    case SceneManagerEventTypeCustom:
        flipp_pomodoro_scene_timer_handle_custom_event(
            app,
            event.event);
        return SceneEventConusmed;
    case SceneManagerEventTypeBack:
        naggy_stop(); // stop "spam" when exiting back
        scene_manager_next_scene(app->scene_manager, FlippPomodoroSceneInfo);
        return SceneEventConusmed;
    default:
        break;
    };
    return SceneEventNotConusmed;
};

void flipp_pomodoro_scene_timer_on_exit(void *ctx)
{
    UNUSED(ctx);
    naggy_stop();
};
