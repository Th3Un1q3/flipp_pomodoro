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
#include <notification/notification.h>

enum
{
    SceneEventConusmed = true,
    SceneEventNotConusmed = false
};

static char *work_hints[] = {
    "Can you explain the problem as if I'm five?",
    "Expected output vs. reality: what's the difference?",
    "Ever thought of slicing the problem into bite-sized pieces?",
    "What's the story when you walk through the code?",
    "Any error messages gossiping about the issue?",
    "What tricks have you tried to fix this?",
    "Did you test the code, or just hoping for the best?",
    "How's this code mingling with the rest of the app?",
    "Any sneaky side effects causing mischief?",
    "What are you assuming, and is it safe to do so?",
    "Did you remember to invite all the edge cases to the party?",
    "What happens in the isolation chamber (running code separately)?",
    "Can you make the issue appear on command?",
    "What's the scene at the crime spot when the error occurs?",
    "Did you seek wisdom from the grand oracle (Google)?",
    "What if you take a different path to solve this?",
    "Did you take a coffee break to reboot your brain?"};

static char *break_hints[] = {
    "Time to stretch! Remember, your body isn't made of code.",
    "Hydrate or diedrate! Grab a glass of water.",
    "Blink! Your eyes need a break too.",
    "How about a quick dance-off with your shadow?",
    "Ever tried chair yoga? Now's the time!",
    "Time for a quick peek out the window. The outside world still exists!",
    "Quick, think about kittens! Or puppies! Or baby turtles!",
    "Time for a laugh. Look up a joke or two!",
    "Sing a song. Bonus points for making up your own lyrics.",
    "Do a quick tidy-up. A clean space is a happy space!",
    "Time to play 'air' musical instrument for a minute.",
    "How about a quick doodle? Unleash your inner Picasso!",
    "Practice your superhero pose. Feel the power surge!",
    "Quick, tell yourself a joke. Don't worry, I won't judge.",
    "Time to practice your mime skills. Stuck in a box, anyone?",
    "Ever tried juggling? Now's your chance!",
    "Do a quick self high-five, you're doing great!"};

static char *random_string_of_list(char **hints, size_t num_hints)
{
    int random_index = rand() % num_hints;
    return hints[random_index];
}

// анти-дубли
static bool g_stage_complete_sent = false; // чтобы StageComplete ушёл один раз
static bool g_once_prompt_shown = false;   // чтобы центр "Continue" показать один раз
static bool g_once_notified = false;       // чтобы звук/вибро при Once сыграть один раз

static void notify_like_slide_next_stage(FlippPomodoroApp* app) {
    // В Slide уведомление при старте СЛЕДУЮЩЕГО этапа — имитируем его при стопе в Once
    PomodoroStage cur = flipp_pomodoro__get_stage(app->state);
    PomodoroStage next = (cur == FlippPomodoroStageFocus) ? FlippPomodoroStageRest : FlippPomodoroStageFocus;
    const NotificationSequence* seq = stage_start_notification_sequence_map[next];

    NotificationApp* n = furi_record_open(RECORD_NOTIFICATION);
    notification_message(n, seq);
    furi_record_close(RECORD_NOTIFICATION);
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

    view_dispatcher_send_custom_event(
        app->view_dispatcher,
        FlippPomodoroAppCustomEventStageSkip);
};

static void flipp_pomodoro_scene_timer_on_continue(void* ctx) {
    // После Once: перейти БЕЗ доп. уведомления -> используем StageSkip (обычно без звука)
    FlippPomodoroApp* app = ctx;
    g_once_prompt_shown = false;
    view_dispatcher_send_custom_event(
        app->view_dispatcher,
        FlippPomodoroAppCustomEventStageSkip);
}

void flipp_pomodoro_scene_timer_on_left(void* ctx) {
    FlippPomodoroApp* app = ctx;
    scene_manager_next_scene(app->scene_manager, FlippPomodoroSceneConfig);
}

void flipp_pomodoro_scene_timer_on_ask_hint(void *ctx)
{
    FlippPomodoroApp *app = ctx;
    view_dispatcher_send_custom_event(
        app->view_dispatcher,
        FlippPomodoroAppCustomEventTimerAskHint);
}

void flipp_pomodoro_scene_timer_on_enter(void *ctx)
{
    furi_assert(ctx);

    FlippPomodoroApp *app = ctx;
    g_stage_complete_sent = false;
    g_once_prompt_shown = false;
    g_once_notified = false;

    if (flipp_pomodoro__is_stage_expired(app->state))
    {
        flipp_pomodoro__destroy(app->state);
        app->state = flipp_pomodoro__new();
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlippPomodoroAppViewTimer);
    flipp_pomodoro_scene_timer_sync_view_state(app);

    flipp_pomodoro_view_timer_set_callback_context(app->timer_view, app);

    // по умолчанию центр — подсказка
    flipp_pomodoro_view_timer_set_await_continue(
        flipp_pomodoro_view_timer_get_view(app->timer_view), false);
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
        return random_string_of_list(break_hints, sizeof(break_hints) / sizeof(break_hints[0]));
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
            } else {
                // Once: стоп на 00:00; один раз звук/вибро СЕЙЧАС; показать "Continue" на центре
                if(!g_once_notified) {
                    notify_like_slide_next_stage(app);
                    g_once_notified = true;
                }
                if(!g_once_prompt_shown) {
                    g_once_prompt_shown = true;
                    flipp_pomodoro_view_timer_set_await_continue(
                        flipp_pomodoro_view_timer_get_view(app->timer_view), true);
                    flipp_pomodoro_view_timer_set_on_ok_cb(
                        app->timer_view,
                        flipp_pomodoro_scene_timer_on_continue);
                }
            }
        } else {
            // активный этап — вернуть обычный центр и сбросить флаги
            if(g_once_prompt_shown) {
                flipp_pomodoro_view_timer_set_await_continue(
                    flipp_pomodoro_view_timer_get_view(app->timer_view), false);
                flipp_pomodoro_view_timer_set_on_ok_cb(
                    app->timer_view,
                    flipp_pomodoro_scene_timer_on_ask_hint);
            }
            g_once_prompt_shown = false;
            g_stage_complete_sent = false;
            g_once_notified = false;
        }
        break;
    }
    case FlippPomodoroAppCustomEventStateUpdated:
        flipp_pomodoro_scene_timer_sync_view_state(app);
        // после смены этапа — обычный центр и чистые флаги
        flipp_pomodoro_view_timer_set_await_continue(
            flipp_pomodoro_view_timer_get_view(app->timer_view), false);
        flipp_pomodoro_view_timer_set_on_ok_cb(
            app->timer_view,
            flipp_pomodoro_scene_timer_on_ask_hint);
        g_stage_complete_sent = false;
        g_once_prompt_shown = false;
        g_once_notified = false;
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
};
