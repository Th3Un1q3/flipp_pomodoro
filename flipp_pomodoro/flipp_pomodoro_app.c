#include "helpers/debug.h"
#include "flipp_pomodoro_app_i.h"


static bool flipp_pomodoro_app_back_event_callback(void* ctx) {
    UNUSED(ctx);
    return true;
}
// static bool flipp_pomodoro_app_back_event_callback(void* context) {
//     furi_assert(context);
//     FlippPomodoroApp* app = context;
//     return scene_manager_handle_back_event(app->scene_manager);
// }

static bool flipp_pomodoro_app_custom_event_callback(void* ctx, uint32_t event) {
    UNUSED(ctx);
    UNUSED(event);
    return true;
}

static void flipp_pomodoro_app_tick_event_callback(void* context) {
    UNUSED(context);
};

// static bool flipp_pomodoro_app_custom_event_callback(void* ctx, uint32_t event) {
//     furi_assert(ctx);
//     FlippPomodoroApp* app = ctx;
//     bool consumed = false;
//     switch (event)
//     {
//     case FlippPomodoroAppCustomEventTogglePomodoroManual:
//         flipp_pomodoro__toggle_stage(app->state);
//         consumed = true;
//         break;
//     case FlippPomodoroAppCustomEventTogglePomodoroNatural:
//         if(app->state->stage == Work) {
//             // REGISTER a deed on work stage complete to get an acheivement
//             DOLPHIN_DEED(DolphinDeedPluginGameWin);
//         };

//         flipp_pomodoro__toggle_stage(app->state);
//         notification_message(app->notification_app, stage_start_notification_sequence_map[app->state->stage]);
//         consumed = true;
//         break;
//     default:
//         break;
//     }
//     return consumed; // Returns true for event consumed;
// }

FlippPomodoroApp* flipp_pomodoro_app_alloc() {
//     // TODO: add free method
    FlippPomodoroApp* app = malloc(sizeof(FlippPomodoroApp));
    app->state = flipp_pomodoro__new();

    app->scene_manager = scene_manager_alloc(&flipp_pomodoro_scene_handlers, app);
    app->gui = furi_record_open(RECORD_GUI);
    app->notification_app = furi_record_open(RECORD_NOTIFICATION);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, flipp_pomodoro_app_custom_event_callback);
    view_dispatcher_set_tick_event_callback(app->view_dispatcher, flipp_pomodoro_app_tick_event_callback, 4000);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, flipp_pomodoro_app_back_event_callback);

    app->timer_view = flipp_pomodoro_view_timer_alloc(app);

    view_dispatcher_add_view(
        app->view_dispatcher,
        FlippPomodoroAppViewTimer,
        flipp_pomodoro_view_timer_get_view(app->timer_view)
    );

    scene_manager_next_scene(app->scene_manager, FlippPomodoroSceneTimer);

    return app;
}


void flipp_pomodoro_app_free(FlippPomodoroApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, FlippPomodoroAppViewTimer);
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    flipp_pomodoro_view_timer_free(app->timer_view);
    flipp_pomodoro__destroy(app->state);
    free(app);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
};


int32_t flipp_pomodoro_app(void* p) {
    UNUSED(p);
    FURI_LOG_E(TAG, "App starting...");
    FlippPomodoroApp* app = flipp_pomodoro_app_alloc(); // TODO: free every alloc eventually and move declarations inside the alloc
    view_dispatcher_run(app->view_dispatcher);
  
    flipp_pomodoro_app_free(app);

    return 0;
}