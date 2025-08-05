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

// анти-дубли
static bool g_stage_complete_sent = false; // чтобы StageComplete ушёл один раз
static bool g_once_notified = false;       // чтобы звук/вибро при Once/Naggy сыграли корректно
static uint8_t g_naggy_left = 0;           // сколько раз ещё повторять штатное уведомление (Naggy)

// Анти-«пролезание»: не дублировать уведомления, пока проигрывается предыдущее.
// Грубая оценка длительности штатной последовательности ~2.5–3с -> берём 3с.
static uint32_t g_naggy_cooldown_until = 0; // timestamp, раньше которого не шлём следующее

// В Slide уведомление при старте СЛЕДУЮЩЕГО этапа — имитируем его при стопе в Once/Naggy.
// ПОВТОРНО ИСПОЛЬЗУЕМ штатные последовательности stage_start_notification_sequence_map[*].
static void notify_like_slide_next_stage(FlippPomodoroApp* app) {
    const uint32_t now = time_now();
    if(now < g_naggy_cooldown_until) {
        return; // не ставим новое в очередь, пока предыдущее ещё играет
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

    // блокируем повторы на время проигрывания штатной последовательности
    g_naggy_cooldown_until = now + 3;
}

// Мгновенно заглушить звук/вибро (без изобретения новых паттернов — только off-сообщения).
static void stop_all_notifications(void) {
    NotificationApp* n = furi_record_open(RECORD_NOTIFICATION);
    static const NotificationSequence seq_stop = { &message_sound_off, &message_vibro_off, NULL };
    notification_message(n, &seq_stop);
    furi_record_close(RECORD_NOTIFICATION);
}

// Полная остановка «спама» Naggy + глушение
static void naggy_stop(void) {
    g_naggy_left = 0;
    // небольшой «зонтик», чтобы тик, пришедший сразу после стопа, не успел поставить новое
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

void flipp_pomodoro_scene_timer_on_next_stage(void *ctx)
{
    furi_assert(ctx);

    FlippPomodoroApp *app = ctx;

    naggy_stop(); // стоп «спам» при переходе на следующий таймер

    view_dispatcher_send_custom_event(
        app->view_dispatcher,
        FlippPomodoroAppCustomEventStageSkip);
};

void flipp_pomodoro_scene_timer_on_left(void* ctx) {
    FlippPomodoroApp* app = ctx;
    naggy_stop(); // стоп «спам» при переходе в настройки
    scene_manager_next_scene(app->scene_manager, FlippPomodoroSceneConfig);
}

void flipp_pomodoro_scene_timer_on_ask_hint(void *ctx)
{
    FlippPomodoroApp *app = ctx;
    naggy_stop(); // стоп «спам» на центральное нажатие
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

    // Если этап уже истёк:
    //  - Slide: начинаем заново (как было);
    //  - Once/Naggy: остаёмся на 00:00 и НЕ переуведомляем после возврата.
    if (flipp_pomodoro__is_stage_expired(app->state))
    {
        FlippPomodoroSettings s;
        if(!flipp_pomodoro_settings_load(&s)) {
            flipp_pomodoro_settings_set_default(&s);
        }
        if(s.buzz_mode == FlippPomodoroBuzzSlide) {
            flipp_pomodoro__destroy(app->state);
            app->state = flipp_pomodoro__new();
        } else {
            g_once_notified = true; // запрет повторного старта уведомлений на первом тике
            naggy_stop();
        }
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlippPomodoroAppViewTimer);
    flipp_pomodoro_scene_timer_sync_view_state(app);

    flipp_pomodoro_view_timer_set_callback_context(app->timer_view, app);

    // центр — подсказка
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

void flipp_pomodoro_scene_timer_handle_custom_event(FlippPomodoroApp *app, FlippPomodoroAppCustomEvent custom_event)
{
    switch (custom_event)
    {
    case FlippPomodoroAppCustomEventTimerTick: {
        const bool expired = flipp_pomodoro__is_stage_expired(app->state);
        if (expired)
        {
            FlippPomodoroSettings s;
            if(!flipp_pomodoro_settings_load(&s)) {
                flipp_pomodoro_settings_set_default(&s);
            }
            if (s.buzz_mode == FlippPomodoroBuzzSlide) {
                // авто-переход (однократно)
                if(!g_stage_complete_sent) {
                    g_stage_complete_sent = true;
                    view_dispatcher_send_custom_event(
                        app->view_dispatcher,
                        FlippPomodoroAppCustomEventStageComplete);
                }
            } else if (s.buzz_mode == FlippPomodoroBuzzOnce) {
                // Once: стоп на 00:00; один раз штатное уведомление СЕЙЧАС; БЕЗ "Continue"
                if(!g_once_notified) {
                    notify_like_slide_next_stage(app); // ПОВТОРНО ИСПОЛЬЗУЕМ штатное
                    g_once_notified = true;
                }
            } else { // FlippPomodoroBuzzAnnoying (Naggy)
                // Naggy: как Once, но повторить штатное уведомление 10 раз (с защитой от очереди)
                if(!g_once_notified) {
                    g_once_notified = true;
                    g_naggy_left = 10; // сразу и ещё 9 попыток
                }
                if(g_naggy_left > 0) {
                    notify_like_slide_next_stage(app); // ПОВТОРНО ИСПОЛЬЗУЕМ штатное (с cooldown)
                    // уменьшаем счётчик только когда попытались отправить (в т.ч. если заблокировано — всё равно считаем попыткой)
                    g_naggy_left--;
                    if(g_naggy_left == 0) {
                        stop_all_notifications();
                    }
                }
            }
        } else {
            // активный этап — чистим флаги/спам
            g_stage_complete_sent = false;
            g_once_notified = false;
            g_naggy_left = 0;
        }
        break;
    }
    case FlippPomodoroAppCustomEventStateUpdated:
        // после смены этапа — центр остаётся подсказкой, и стоп «спам»
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
        naggy_stop(); // стоп «спам» при выходе назад
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
